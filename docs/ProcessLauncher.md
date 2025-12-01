# ProcessLauncher 모듈 기술 문서

## 개요

`ProcessLauncher`는 Windows 서비스(LocalSystem)에서 사용자 세션으로 프로세스를 생성하기 위한 모듈임. 다양한 권한 레벨(User/Admin/System)과 환경 설정을 지원하며, 일반 UAC 환경뿐만 아니라 PAM(Privilege Access Management) 환경에서도 안정적으로 동작함.

### 주요 기능
- Cross-session 프로세스 생성 (Session 0 → Session N)
- 세 가지 권한 레벨 지원: User, Administrator, System
- PAM 환경에서 SYSTEM 권한 + 사용자 환경 변수 조합
- 세션 환경 분석 및 권장 권한 레벨 제안
- 상세한 오류 보고

---

## 기술적 배경

### 1. Windows 세션 아키텍처

Windows Vista 이후, Session 0 Isolation이 도입되어 서비스와 사용자 프로세스가 분리됨:

```
┌─────────────────────────────────────────────────────────────┐
│  Session 0 (서비스 세션)                                     │
│  ├── services.exe                                           │
│  ├── lsass.exe                                              │
│  └── Monster 서비스 (LocalSystem)                           │
├─────────────────────────────────────────────────────────────┤
│  Session 1+ (사용자 대화형 세션)                             │
│  ├── explorer.exe (사용자 셸)                               │
│  ├── 사용자 애플리케이션들                                   │
│  └── [대상] 우리가 생성하려는 프로세스                       │
└─────────────────────────────────────────────────────────────┘
```

서비스에서 사용자 세션으로 프로세스를 생성하려면 다음이 필요함:
1. 대상 세션에 바인딩된 토큰
2. `CreateProcessAsUser()` API 사용
3. 적절한 환경 블록과 데스크톱 지정

### 2. UAC (User Account Control) 토큰 구조

UAC가 활성화된 환경에서 관리자 그룹 사용자는 **두 개의 연결된 토큰**을 가짐:

```
┌─────────────────────────────────────────────────────────────┐
│  관리자 그룹 사용자 로그인                                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────┐    ┌─────────────────────┐        │
│  │   Standard Token    │◄──►│   Elevated Token    │        │
│  │   (제한된 토큰)      │    │   (Linked Token)    │        │
│  ├─────────────────────┤    ├─────────────────────┤        │
│  │ Medium IL           │    │ High IL             │        │
│  │ 관리자 권한 비활성화  │    │ 관리자 권한 활성화   │        │
│  │ explorer.exe 사용   │    │ UAC 승인 후 사용    │        │
│  └─────────────────────┘    └─────────────────────┘        │
│                                                             │
│  TokenElevationType:                                        │
│  - TokenElevationTypeLimited(3): Standard Token 사용 중     │
│  - TokenElevationTypeFull(2): Elevated Token 사용 중        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Token Elevation Type 값의 의미

| 값 | 상수 | 의미 |
|---|------|------|
| 1 | `TokenElevationTypeDefault` | UAC 비활성화 또는 표준 사용자 |
| 2 | `TokenElevationTypeFull` | 이미 Elevated 상태 (관리자 권한 실행 중) |
| 3 | `TokenElevationTypeLimited` | UAC 활성화, 제한된 토큰 사용 중 |

### 3. PAM (Privilege Access Management) 환경의 특수성

PAM 솔루션이 적용된 엔터프라이즈 환경에서는 다음과 같은 상황이 발생함:

```
┌─────────────────────────────────────────────────────────────┐
│  PAM 환경의 사용자                                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────┐                                   │
│  │   Standard Token    │    Linked Token 없음!             │
│  │   (유일한 토큰)      │    (관리자 그룹 아님)              │
│  ├─────────────────────┤                                   │
│  │ Medium IL           │                                   │
│  │ Administrators ✗    │                                   │
│  └─────────────────────┘                                   │
│                                                             │
│  TokenElevationType: TokenElevationTypeDefault(1)          │
│                                                             │
│  PAM이 Just-In-Time으로 권한 부여                           │
│  → 일반적인 Linked Token 메커니즘 사용 불가                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**핵심 문제**: 사용자가 Administrators 그룹에 속하지 않으므로 Linked Token이 존재하지 않음. 따라서 기존의 `administrator_privilege` 방식으로는 관리자 권한 프로세스를 생성할 수 없음.

---

## 설계 결정 (Design Decisions)

### 결정 1: 세 가지 권한 레벨 도입

**문제**: 기존 `create_process_with_privilege()`는 `user_privilege`와 `administrator_privilege` 두 가지만 지원함. PAM 환경에서 `administrator_privilege`가 실패하면 대안이 없었음.

**해결책**: `system_privilege` 레벨을 추가함.

```cpp
enum class launcher_privilege_level
{
    user_privilege,           // explorer.exe Standard Token
    administrator_privilege,  // explorer.exe Linked Token (UAC)
    system_privilege          // LocalSystem Token (세션 바인딩)
};
```

| 레벨 | 토큰 소스 | Integrity Level | 사용 시나리오 |
|-----|---------|-----------------|--------------|
| `user_privilege` | explorer.exe | Medium | 일반 사용자 권한 실행 |
| `administrator_privilege` | Linked Token | High | UAC 환경 관리자 권한 |
| `system_privilege` | LocalSystem | System | PAM 환경 관리자 권한 |

**트레이드오프**: `system_privilege`는 필요 이상으로 높은 권한(SYSTEM)을 부여하지만, PAM 환경에서는 이것이 유일한 선택지임.

### 결정 2: 환경 블록 전략 (Environment Strategy)

**문제**: LocalSystem 토큰으로 프로세스를 생성하면 환경 변수가 SYSTEM 계정의 것으로 설정됨:

```
SYSTEM 환경                          사용자 환경 (원하는 값)
─────────────────────────────────    ─────────────────────────────────
%USERPROFILE% = C:\Windows\System32  %USERPROFILE% = C:\Users\somma
              \config\systemprofile
%APPDATA% = ...systemprofile\...     %APPDATA% = C:\Users\somma\AppData
%TEMP% = C:\Windows\Temp             %TEMP% = C:\Users\somma\...\Temp
```

이로 인해 사용자 설정 파일 접근, 임시 파일 생성 등에서 문제가 발생함.

**해결책**: 환경 블록 전략 옵션을 도입함.

```cpp
enum class environment_strategy
{
    system_environment,   // SYSTEM 환경 (기본, 호환성 문제 가능)
    user_environment,     // 사용자 환경 (권장)
    merged_environment    // 병합 (향후 확장)
};
```

**구현 방식**:
```
┌─────────────────────────────────────────────────────────────┐
│  system_privilege + user_environment                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  토큰: LocalSystem (세션 바인딩)                             │
│    → SYSTEM 권한으로 실행함                                 │
│    → 파일 시스템/레지스트리 접근에 최고 권한을 가짐           │
│                                                             │
│  환경 블록: explorer.exe 토큰에서 생성함                     │
│    → CreateEnvironmentBlock(explorer_token)                 │
│    → %USERPROFILE%, %APPDATA% 등이 사용자 값이 됨           │
│                                                             │
│  결과: SYSTEM 권한 + 사용자 환경 변수                        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 결정 3: 세션 환경 분석 API

**문제**: 호출자가 현재 환경이 UAC인지 PAM인지 알 수 없어서 적절한 권한 레벨을 선택하기 어려움.

**해결책**: `analyze_session_capabilities()` API를 제공함.

```cpp
session_capability_info info;
analyze_session_capabilities(session_id, info);

// 분석 결과:
// - is_admin_group_member: Administrators 그룹 멤버십
// - has_linked_token: Linked Token 존재 여부
// - is_pam_environment: PAM 환경 추정
// - recommended_admin_level: 권장 권한 레벨
```

**환경 감지 로직**:
```
if (!is_admin_group_member && !has_linked_token)
    → PAM 환경으로 추정함
    → recommended_admin_level = system_privilege

else if (has_linked_token)
    → 일반 UAC 환경임
    → recommended_admin_level = administrator_privilege

else if (is_admin_group_member && !has_linked_token)
    → UAC 비활성화 환경임
    → recommended_admin_level = administrator_privilege
```

### 결정 4: 구조체 기반 API와 간편 API 이중 제공

**문제**: 복잡한 옵션을 모두 함수 파라미터로 받으면 함수 시그니처가 길어지고, 향후 확장 시 호환성 문제 발생.

**해결책**: 두 가지 API 스타일을 제공.

```cpp
// 1. 구조체 기반 API (유연하고 확장 가능)
process_launch_request request;
request.privilege_level = ...;
request.session_id = ...;
request.cmdline = ...;
request.sys_options.env_strategy = ...;

process_launch_response response;
launch_process(request, response);

// 2. 간편 API (일반적인 사용 케이스)
launch_process(privilege_level, session_id, cmdline, &pi);

// 3. 확장 API (system_privilege 옵션 필요시)
launch_process_ex(privilege_level, session_id, cmdline, sys_options, &pi);
```

**장점**:
- 간단한 케이스는 간편하게 사용함
- 복잡한 케이스는 구조체로 명확하게 전달함
- 새로운 옵션 추가 시 구조체만 확장함 (기존 API 호환)

### 결정 5: 상세한 오류 보고

**문제**: 기존 `create_process_with_privilege()`는 `bool`만 반환하여 실패 원인을 파악하기 어려웠음.

**해결책**: `launcher_result` 열거형과 `process_launch_response` 구조체를 도입함.

```cpp
enum class launcher_result
{
    success,
    invalid_parameter,
    session_not_found,
    token_source_not_found,     // explorer.exe 없음
    token_acquisition_failed,
    token_duplication_failed,
    elevation_not_available,    // Linked Token 없음 (PAM 환경)
    session_binding_failed,
    environment_creation_failed,
    process_creation_failed,
    wait_failed,
    internal_error
};

struct process_launch_response
{
    launcher_result result;
    PROCESS_INFORMATION pi;
    DWORD exit_code;
    DWORD last_error;           // GetLastError() 값
    std::wstring token_source_process;  // 디버깅용
};
```

---

## 구현 세부 사항

### 토큰 획득 흐름

```
┌─────────────────────────────────────────────────────────────┐
│  acquire_execution_token()                                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [user_privilege]                                           │
│    │                                                        │
│    ├─ 1. 대상 세션의 explorer.exe를 찾음                    │
│    ├─ 2. OpenProcess() + OpenProcessToken()을 호출함       │
│    ├─ 3. DuplicateTokenEx() (Standard Token)을 수행함      │
│    └─ 4. SetTokenInformation(TokenSessionId)을 설정함      │
│                                                             │
│  [administrator_privilege]                                  │
│    │                                                        │
│    ├─ 1. 대상 세션의 explorer.exe를 찾음                    │
│    ├─ 2. OpenProcess() + OpenProcessToken()을 호출함       │
│    ├─ 3. GetTokenInformation(TokenLinkedToken)을 조회함    │
│    │     ├─ 성공: Linked Token을 획득함                    │
│    │     └─ 실패: elevation_not_available을 반환함         │
│    └─ 4. DuplicateTokenEx() (Linked Token)을 수행함        │
│                                                             │
│  [system_privilege]                                         │
│    │                                                        │
│    ├─ 1. OpenProcessToken(GetCurrentProcess())을 호출함    │
│    │     (LocalSystem 토큰)                                 │
│    ├─ 2. DuplicateTokenEx()를 수행함                       │
│    └─ 3. SetTokenInformation(TokenSessionId)으로           │
│          대상 세션에 바인딩함                                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 환경 블록 생성 흐름

```
┌─────────────────────────────────────────────────────────────┐
│  환경 블록 전략별 처리                                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [system_environment]                                       │
│    └─ 환경 블록 없음 (nullptr)                              │
│       → CreateProcessAsUser가 SYSTEM 기본값을 사용함        │
│                                                             │
│  [user_environment]                                         │
│    │                                                        │
│    ├─ 1. 대상 세션의 explorer.exe 토큰을 획득함             │
│    ├─ 2. CreateEnvironmentBlock(explorer_token, TRUE)을    │
│    │     호출함                                             │
│    └─ 3. 사용자 환경 블록을 프로세스 생성에 사용함           │
│                                                             │
│  [merged_environment] (향후 확장)                           │
│    └─ SYSTEM 환경 + 사용자 경로 변수를 병합함               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 프로세스 생성 재시도 로직

`CreateProcessAsUser()`는 다양한 이유로 실패할 수 있어 재시도 로직을 구현함:

```
┌─────────────────────────────────────────────────────────────┐
│  create_process_internal() 재시도 로직                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [시도 1] 기본 설정                                         │
│    - 환경 블록을 사용함                                     │
│    - lpDesktop = "winsta0\\default"                        │
│    - CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT       │
│    │                                                        │
│    └─ 실패 시 ↓                                            │
│                                                             │
│  [시도 2] 환경 블록 없이 시도함                             │
│    - 환경 블록 = nullptr                                    │
│    - 일부 환경에서 환경 블록 문제를 회피함                   │
│    │                                                        │
│    └─ 실패 시 ↓                                            │
│                                                             │
│  [시도 3] 데스크톱 지정 없이 시도함                         │
│    - lpDesktop = nullptr                                   │
│    - 환경 블록 = nullptr                                    │
│    - 최소 설정으로 시도함                                   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 사용 시나리오

### 시나리오 1: 일반 UAC 환경

```cpp
// 환경을 분석함
session_capability_info cap;
analyze_session_capabilities(LAUNCHER_SESSION_ACTIVE_CONSOLE, cap);
// 결과: has_linked_token = true, recommended_admin_level = administrator_privilege

// 관리자 권한으로 실행함
PROCESS_INFORMATION pi;
launch_process(
    launcher_privilege_level::administrator_privilege,
    cap.session_id,
    L"C:\\MyApp\\admin_tool.exe",
    &pi
);
```

### 시나리오 2: PAM 환경

```cpp
// 환경을 분석함
session_capability_info cap;
analyze_session_capabilities(LAUNCHER_SESSION_ACTIVE_CONSOLE, cap);
// 결과: is_pam_environment = true, recommended_admin_level = system_privilege

// SYSTEM 권한 + 사용자 환경으로 실행함
system_privilege_options opts;
opts.env_strategy = environment_strategy::user_environment;

launch_process_ex(
    launcher_privilege_level::system_privilege,
    cap.session_id,
    L"C:\\MyApp\\admin_tool.exe",
    opts,
    &pi
);
```

### 시나리오 3: 자동 권한 레벨 선택

```cpp
session_capability_info cap;
if (analyze_session_capabilities(session_id, cap))
{
    // 환경에 따라 자동으로 선택함
    launcher_privilege_level level = need_admin 
        ? cap.recommended_admin_level 
        : launcher_privilege_level::user_privilege;
    
    if (level == launcher_privilege_level::system_privilege)
    {
        // PAM 환경: 사용자 환경 블록을 사용함
        system_privilege_options opts(environment_strategy::user_environment);
        launch_process_ex(level, session_id, cmdline, opts, &pi);
    }
    else
    {
        launch_process(level, session_id, cmdline, &pi);
    }
}
```

---

## 보안 고려 사항

### SE_TCB_PRIVILEGE 요구

세션 간 토큰 조작(`SetTokenInformation(TokenSessionId)`)에는 `SE_TCB_PRIVILEGE`가 필요함. 이 권한은 일반적으로 LocalSystem 계정에만 부여됨.

```
호출자 요구 사항:
- LocalSystem 계정으로 실행되는 서비스
- 또는 SE_TCB_PRIVILEGE가 부여된 계정
```

### system_privilege 사용 시 주의

`system_privilege`는 SYSTEM 권한을 부여하므로 다음 사항에 주의 필요:
- 필요 이상의 권한임 (최소 권한 원칙 위배 가능)
- 신뢰할 수 있는 애플리케이션에만 사용함
- 가능하면 `administrator_privilege`를 먼저 시도함

### 환경 블록 보안

`user_environment` 전략 사용 시 다음 사항을 고려함:
- 사용자 환경 변수에 민감한 정보가 포함될 수 있음
- PATH 조작을 통한 DLL Hijacking 가능성을 고려함
- 신뢰할 수 있는 경로에서 실행 파일을 로드하도록 권장함

---

## 마이그레이션 가이드

### 기존 코드

```cpp
#include "Win32Utils.h"

create_process_with_privilege(
    process_privilege_level::administrator_privilege,
    session_id,
    cmdline,
    &pi
);
```

### 새 코드

```cpp
#include "ProcessLauncher.h"

// 방법 1: 직접 변환 (UAC 환경만 지원)
launch_process(
    launcher_privilege_level::administrator_privilege,
    session_id,
    cmdline,
    &pi
);

// 방법 2: 환경 분석 후 적절한 레벨을 선택함 (권장)
session_capability_info cap;
if (analyze_session_capabilities(session_id, cap))
{
    if (cap.recommended_admin_level == launcher_privilege_level::system_privilege)
    {
        system_privilege_options opts(environment_strategy::user_environment);
        launch_process_ex(cap.recommended_admin_level, session_id, cmdline, opts, &pi);
    }
    else
    {
        launch_process(cap.recommended_admin_level, session_id, cmdline, &pi);
    }
}
```

## 참고 자료

- [Session 0 Isolation](https://docs.microsoft.com/en-us/windows/win32/services/services-and-the-session-0-isolation)
- [User Account Control](https://docs.microsoft.com/en-us/windows/security/identity-protection/user-account-control/user-account-control-overview)
- [CreateProcessAsUser](https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessasuser)
- [Token Linked Token](https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-token_information_class)
- [CreateEnvironmentBlock](https://docs.microsoft.com/en-us/windows/win32/api/userenv/nf-userenv-createenvironmentblock)
