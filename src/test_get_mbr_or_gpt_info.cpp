/**
 * @file    
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    05/03/2025 12:13 Created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "_MyLib/src/log.h"

#pragma pack(push, 1)

struct MBR_PARTITION_ENTRY {
    uint8_t status;
    uint8_t chsFirst[3];
    uint8_t type;
    uint8_t chsLast[3];
    uint32_t lbaFirst;
    uint32_t sectorCount;
};

struct MBR {
    uint8_t bootCode[446];
    MBR_PARTITION_ENTRY partitions[4];
    uint16_t signature;
};

struct GPT_HEADER {
    uint64_t signature;             // "EFI PART"
    uint32_t revision;
    uint32_t headerSize;
    uint32_t headerCRC32;
    uint32_t reserved;
    uint64_t currentLBA;
    uint64_t backupLBA;
    uint64_t firstUsableLBA;
    uint64_t lastUsableLBA;
    uint8_t diskGUID[16];
    uint64_t partitionEntryLBA;
    uint32_t numberOfPartitionEntries;
    uint32_t sizeOfPartitionEntry;
    uint32_t partitionEntryArrayCRC32;
};

// GPT 파티션 엔트리 구조체 추가
struct GPT_PARTITION_ENTRY {
    uint8_t partitionTypeGUID[16];
    uint8_t uniquePartitionGUID[16];
    uint64_t startingLBA;
    uint64_t endingLBA;
    uint64_t attributes;
    WCHAR partitionName[36]; // UTF-16 인코딩, 36 글자(72 바이트)
};

#pragma pack(pop)

// 커스텀 HANDLE deleter
struct HandleDeleter {
    void operator()(_In_opt_ HANDLE handle) const {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
};

using unique_handle = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;

// GUID 출력 함수
void PrintGUID(_In_reads_(16) const uint8_t* guid)
{
    printf("%08x-%04x-%04x-",
           *(const uint32_t*)&guid[0],
           *(const uint16_t*)&guid[4],
           *(const uint16_t*)&guid[6]);
    for (int i = 8; i < 10; ++i) printf("%02x", guid[i]);
    printf("-");
    for (int i = 10; i < 16; ++i) printf("%02x", guid[i]);
}

// GPT 파티션 타입 이름 반환 함수
const char* GetPartitionTypeName(const uint8_t* guid) {
    // 자주 사용되는 파티션 타입 GUID
    static const struct {
        uint8_t guid[16];
        const char* name;
    } KNOWN_PARTITION_TYPES[] = {
        { {0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}, "EFI System" },
        { {0xA2, 0xA0, 0xD0, 0xEB, 0xE5, 0xB9, 0x33, 0x44, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}, "Basic Data" },
        { {0x16, 0xE3, 0xC9, 0xE3, 0x5C, 0x0B, 0xB8, 0x4D, 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE}, "MS Reserved" },
        { {0xAA, 0xC8, 0x08, 0xEF, 0x10, 0x63, 0xC6, 0x48, 0x82, 0x57, 0xAC, 0x66, 0xF0, 0xA9, 0xB8, 0x14}, "Linux Swap" },
        { {0x0F, 0x88, 0x9D, 0xA1, 0xFC, 0x05, 0x3B, 0x4D, 0xA0, 0x06, 0x74, 0x3F, 0x0F, 0x84, 0x91, 0x1E}, "Linux LVM" },
        { {0xAF, 0x3D, 0xC6, 0x0F, 0x83, 0x84, 0x72, 0x47, 0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4}, "Linux Home" },
        { {0x4F, 0x68, 0xBC, 0xE3, 0xE8, 0xCD, 0x4D, 0xB1, 0x96, 0x10, 0x65, 0xA3, 0x55, 0x1E, 0x28, 0xEB}, "Apple HFS+" },
        // 추가 타입은 필요에 따라 확장 가능
    };

    for (const auto& type : KNOWN_PARTITION_TYPES) {
        if (memcmp(guid, type.guid, 16) == 0) {
            return type.name;
        }
    }

    return "Unknown";
}

// 섹터 읽기 함수
_Success_(return != false)
bool ReadSector(
    _In_ HANDLE hDisk,
    _In_ DWORD64 sectorIndex,
    _Out_writes_bytes_(sectorSize) BYTE * buffer,
    _In_ DWORD sectorSize = 512
)
{
    LARGE_INTEGER pos;
    pos.QuadPart = sectorIndex * sectorSize;

    if (!SetFilePointerEx(hDisk, pos, NULL, FILE_BEGIN)) {
        _tprintf(_T("[-] SetFilePointerEx failed: %lu\n"), GetLastError());
        return false;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hDisk, buffer, sectorSize, &bytesRead, NULL) || bytesRead != sectorSize) {
        _tprintf(_T("[-] ReadFile failed: %lu\n"), GetLastError());
        return false;
    }

    return true;
}

bool get_mbr_gpt_info()
{
    // Open PhysicalDrive0 with RAII wrapper
    unique_handle hDisk(CreateFile(_T("\\\\.\\PhysicalDrive0"),
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    nullptr,
                                    OPEN_EXISTING,
                                    0,
                                    nullptr));
    if (hDisk.get() == INVALID_HANDLE_VALUE) {
        log_err "Failed to open PhysicalDrive0" log_end;
        return false;
    }

    auto sector = std::make_unique<BYTE[]>(512);
    if (!ReadSector(hDisk.get(), 0, sector.get())) {
        log_err "Failed to read MBR sector" log_end;
        return false; 
    }

    const MBR* mbr = reinterpret_cast<const MBR*>(sector.get());
    if (mbr->signature != 0xAA55) {
        log_err "Invalid MBR signature: 0x%04X", mbr->signature log_end;
        return false;
    }

    bool gptDetected = false;
    for (int i = 0; i < 4; ++i) {
        if (mbr->partitions[i].type == 0xEE) {
            log_info "[+] Protective MBR detected (GPT disk)" log_end;
            gptDetected = true;
            break;
        }
    }

    if (gptDetected) {
        auto gptSector = std::make_unique<BYTE[]>(512);
        if (!ReadSector(hDisk.get(), 1, gptSector.get())) {
            log_err "Failed to read GPT header" log_end;
            return false;
        }

        const GPT_HEADER* gpt = reinterpret_cast<const GPT_HEADER*>(gptSector.get());
        if (gpt->signature == 0x5452415020494645ULL) { // "EFI PART"
            log_info "[+] GPT header is valid" log_end;
            log_info "    Current LBA: %llu", gpt->currentLBA log_end;
            log_info "    Backup LBA : %llu", gpt->backupLBA log_end;
            log_info "    Disk GUID  : " log_end;
            PrintGUID(gpt->diskGUID);
            log_info "" log_end;
            log_info "    Partition Entries LBA: %llu", gpt->partitionEntryLBA log_end;
            log_info "    Number of Partition Entries: %u", gpt->numberOfPartitionEntries log_end;
            log_info "    Size of Partition Entry: %u", gpt->sizeOfPartitionEntry log_end;

            // 파티션 엔트리 읽기
            log_info "[+] GPT Partition Entries : " log_end;

            // 파티션 엔트리는 일반적으로 크기가 128바이트이며, 섹터당 최대 4개의 엔트리가 들어갈 수 있음
            // 실제 엔트리의 크기는 gpt->sizeOfPartitionEntry에 저장되어 있음
            const DWORD entriesPerSector = 512 / gpt->sizeOfPartitionEntry;
            auto partitionEntriesSector = std::make_unique<BYTE[]>(512);

            for (UINT i = 0; i < gpt->numberOfPartitionEntries; i++) {
                // 현재 파티션 엔트리의 섹터 및 섹터 내 오프셋 계산
                DWORD64 sectorIndex = gpt->partitionEntryLBA + (i / entriesPerSector);
                DWORD offsetInSector = (i % entriesPerSector) * gpt->sizeOfPartitionEntry;

                // 필요한 경우에만 새 섹터 읽기
                if (i % entriesPerSector == 0) {
                    if (!ReadSector(hDisk.get(), sectorIndex, partitionEntriesSector.get())) {
                        log_err "Failed to read GPT partition entry sector" log_end;
                    }
                }

                // 파티션 엔트리 참조
                const GPT_PARTITION_ENTRY* entry = reinterpret_cast<const GPT_PARTITION_ENTRY*>(
                    partitionEntriesSector.get() + offsetInSector);

                // 비어있지 않은 파티션만 출력 (Type GUID가 모두 0이면 비어있는 파티션)
                bool isEmpty = true;
                for (int j = 0; j < 16; j++) {
                    if (entry->partitionTypeGUID[j] != 0) {
                        isEmpty = false;
                        break;
                    }
                }

                if (!isEmpty) {
                    log_info "    Partition % u:", i + 1 log_end;
                    log_info "      Type GUID: " log_end;
                    PrintGUID(entry->partitionTypeGUID);
                    log_info " (%S)", GetPartitionTypeName(entry->partitionTypeGUID) log_end;

                    log_info "      Unique GUID: " log_end;
                    PrintGUID(entry->uniquePartitionGUID);
                    log_info "" log_end;

                    log_info "      Starting LBA: %llu", entry->startingLBA log_end;
                    log_info "      Ending LBA: %llu", entry->endingLBA log_end;
                    log_info "      Size: %.2f GB (%llu sectors)",
                        (entry->endingLBA - entry->startingLBA + 1) * 512.0 / (1024 * 1024 * 1024),
                        (entry->endingLBA - entry->startingLBA + 1) log_end;

                    log_info "      Attributes: 0x%016llX", entry->attributes log_end;
                    log_info "      Name: %ws", entry->partitionName log_end;
                }
            }
        }
        else {
            log_err "[-] Invalid GPT header signature" log_end;
        }
    }
    else {
        log_info "[*] MBR Partition Table" log_end;
        for (int i = 0; i < 4; ++i) {
            if (mbr->partitions[i].type != 0) {
                log_info 
                    "    Partition %d: Type = 0x%02X, Start LBA = %u, Size = %u sectors",
                    i + 1,
                    mbr->partitions[i].type,
                    mbr->partitions[i].lbaFirst,
                    mbr->partitions[i].sectorCount
                    log_end;
            }
        }
    }

    return true;
}



