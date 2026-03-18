# _MyLib/src — Legacy Windows Utility Library

> 레거시 코드. 전역 의존성 허브. 변경 시 전체 빌드 영향.
> 코딩 규칙 → 루트 `CLAUDE.md`. 아키텍처 개요 → 루트 `AGENTS.md`.

## ⚠️ Complexity Warning

- `Win32Utils.cpp` (8990 LOC) — 프로젝트 최대 파일. 거의 모든 모듈이 의존.
- `strtk.hpp` (16775 LOC) — 3rd party 토크나이저. **수정 금지**.

## Category: Process & Service

| 파일 | 역할 |
|------|------|
| ProcessLauncher | 프로세스 생성/제어 |
| process_tree | 프로세스 트리 구축 |
| ServiceBase | Windows 서비스 기본 클래스 (Monster 서비스가 상속) |
| ThreadManager | 스레드 생성/관리 |

## Category: File & Crypto

| 파일 | 역할 |
|------|------|
| FileIoHelper / FileIoHelperClass | 파일 읽기/쓰기 유틸리티 |
| AirCrypto | 암호화 유틸리티 |
| md5, sha2 | 해시 함수 구현 |
| base64 | Base64 인코딩/디코딩 |
| rc4 | RC4 스트림 암호 |
| crc64 | CRC-64 체크섬 |

## Category: Network

| 파일 | 역할 |
|------|------|
| net_util | 네트워크 유틸리티 (IP/MAC/어댑터) |
| curl_client | libcurl HTTP 클라이언트 래퍼 |
| ntp_client | NTP 시간 동기화 |
| send_ping.h | ICMP ping 유틸리티 |

## Category: System

| 파일 | 역할 |
|------|------|
| Win32Utils | 종합 Win32 API 래퍼 (⚠️ 8990 LOC) |
| RegistryUtil | 레지스트리 읽기/쓰기 |
| wmi_client | WMI 쿼리 클라이언트 |
| nt_name_conv | NT ↔ DOS 경로 변환 |
| machine_id | 머신 고유 ID 생성 |
| account_info | 사용자 계정 정보 |
| Wow64Util | WOW64 리다이렉션 유틸 |
| scm_context | 서비스 제어 관리자 래퍼 |
| sched_client | Windows 작업 스케줄러 클라이언트 |

## Category: Helpers

| 파일 | 역할 |
|------|------|
| match | 문자열 패턴 매칭 |
| CStream | 메모리 스트림 |
| StopWatch.h | 고해상도 타이머 |
| steady_timer.h | 안정 타이머 |
| thread_pool.h | 스레드 풀 |
| Queue.h | 스레드 안전 큐 |
| StatusCode.h | 상태 코드 정의 |
| Singleton.h | ⚠️ DEPRECATED — DI 사용 |

## 3rd Party (수정 금지)

- `strtk.hpp` (16775 LOC) — String Toolkit, 널리 include됨
- `GeneralHashFunctions` — 범용 해시 함수

## Dependencies

- ← MonsterCore: 강한 의존 (Win32Utils, RegistryUtil, net_util 등)
- ← MonsterCoreSupport: 일부 유틸 사용
- ← Monster: ServiceBase 상속
- vcpkg: boost, curl, openssl
