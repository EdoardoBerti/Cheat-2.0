#pragma once
// ============================================================================
//  Unified Memory Interface — Dual-Engine (Kernel Driver + Direct Usermode)
//  Allows RONRUN to run as a 100% standalone single executable or via kernel stealth.
// ============================================================================

#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>

#pragma comment(lib, "psapi.lib")

#include "../driver/ioctl.h"
#include "driver_payload.h"

class DriverComm {
public:
    enum MemoryMode {
        MODE_NONE = 0,
        MODE_KERNEL = 1,
        MODE_USERMODE = 2
    };

    MemoryMode mode     = MODE_NONE;
    HANDLE  device      = INVALID_HANDLE_VALUE; // For kernel mode
    HANDLE  hProcess    = NULL;                 // For usermode direct
    DWORD   pid         = 0;
    uint64_t game_base  = 0;  // ROB RUN.exe
    uint64_t ga_base    = 0;  // GameAssembly.dll
    uint32_t ga_size    = 0;
    uint64_t unity_base = 0;  // UnityPlayer.dll
    uint32_t unity_size = 0;

    const char* GetModeName() const {
        if (mode == MODE_KERNEL)   return "KERNEL DRIVER";
        if (mode == MODE_USERMODE) return "USERMODE DIRECT";
        return "DISCONNECTED";
    }

    // ---- Connection / Init ----

    bool ConnectKernel() {
        device = CreateFileW(
            DRV_USERMODE_PATH,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        if (device != INVALID_HANDLE_VALUE) {
            mode = MODE_KERNEL;
            return true;
        }
        return false;
    }

    void Disconnect() {
        if (device != INVALID_HANDLE_VALUE) {
            CloseHandle(device);
            device = INVALID_HANDLE_VALUE;
        }
        if (hProcess && hProcess != INVALID_HANDLE_VALUE) {
            CloseHandle(hProcess);
            hProcess = NULL;
        }
        mode = MODE_NONE;
    }

    bool IsConnected() const {
        if (mode == MODE_KERNEL)   return (device != INVALID_HANDLE_VALUE);
        if (mode == MODE_USERMODE) return (hProcess != NULL && hProcess != INVALID_HANDLE_VALUE);
        return false;
    }

    // Hybrid auto-loader: tries Kernel first, falls back to direct Usermode (standalone single .exe)
    bool EnsureDriverLoaded() {
        // 1. Check if kernel driver is already running
        if (ConnectKernel()) {
            printf("[+] Kernel Driver rilevato ed attivo! (Stealth IOCTL Mode)\n");
            return true;
        }

        // 2. If embedded driver payload is present, extract to %TEMP% and attempt load
        if (g_driver_payload_size > 0) {
            WCHAR temp_path[MAX_PATH];
            GetTempPathW(MAX_PATH, temp_path);
            wcscat_s(temp_path, L"ronrun_driver.sys");

            HANDLE hFile = CreateFileW(temp_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(hFile, g_driver_payload, (DWORD)g_driver_payload_size, &written, nullptr);
                CloseHandle(hFile);
                printf("[*] Driver integrato estratto in: %ls\n", temp_path);
            }
        }

        // 3. Check if kdmapper.exe and ronrun_driver.sys exist in current directory
        if (GetFileAttributesW(L"kdmapper.exe") != INVALID_FILE_ATTRIBUTES &&
            GetFileAttributesW(L"ronrun_driver.sys") != INVALID_FILE_ATTRIBUTES)
        {
            printf("[*] kdmapper.exe trovato. Avvio mapping del driver...\n");
            system("kdmapper.exe ronrun_driver.sys");
            Sleep(1000);
            if (ConnectKernel()) {
                printf("[+] Driver mappato con successo tramite kdmapper!\n");
                return true;
            }
        }

        // 4. Try Service Manager (Test Signing)
        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (scm) {
            WCHAR path[MAX_PATH];
            GetFullPathNameW(L"ronrun_driver.sys", MAX_PATH, path, nullptr);

            SC_HANDLE svc = OpenServiceW(scm, L"RonRunDriver", SERVICE_ALL_ACCESS);
            if (!svc && GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
                svc = CreateServiceW(
                    scm, L"RonRunDriver", L"RonRun Driver Service",
                    SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                    path, nullptr, nullptr, nullptr, nullptr, nullptr
                );
            }
            if (svc) {
                StartServiceW(svc, 0, nullptr);
                CloseServiceHandle(svc);
            }
            CloseServiceHandle(scm);
            Sleep(500);
            if (ConnectKernel()) {
                printf("[+] Driver avviato come servizio di sistema!\n");
                return true;
            }
        }

        // 5. Fallback: Direct Usermode Standalone Mode (Zero external driver required!)
        mode = MODE_USERMODE;
        printf("[*] Nessun driver kernel attivo.\n");
        printf("[+] Modalita USERMODE DIRETTA attivata con successo!\n");
        printf("[+] Il cheat funzionera in modalita standalone (eseguibile unico autonomo).\n");
        return true;
    }

    // Wait for game process and all required modules
    bool WaitForGame() {
        printf("[*] In attesa che ROB RUN.exe venga avviato... (Apri ora il gioco da Steam)\n");

        while (!FindProcess(L"ROB RUN.exe")) {
            Sleep(500);
        }
        printf("[+] Rilevato ROB RUN.exe (PID: %u)!\n", pid);

        printf("[*] In attesa del caricamento dei moduli (GameAssembly.dll, UnityPlayer.dll)...\n");
        uint32_t game_size = 0;
        int attempts = 0;
        while (!FindModule(L"GameAssembly.dll", ga_base, ga_size) ||
               !FindModule(L"UnityPlayer.dll", unity_base, unity_size))
        {
            Sleep(1000);
            attempts++;
            if (attempts > 90) {
                printf("[!] Timeout: moduli di gioco non caricati in tempo.\n");
                return false;
            }
        }

        FindModule(L"ROB RUN.exe", game_base, game_size);
        printf("[+] Tutti i moduli sono stati agganciati con successo!\n");
        printf("    Modalita:         [%s]\n", GetModeName());
        printf("    ROB RUN.exe:      0x%llX\n", game_base);
        printf("    GameAssembly.dll: 0x%llX (size: 0x%X)\n", ga_base, ga_size);
        printf("    UnityPlayer.dll:  0x%llX (size: 0x%X)\n\n", unity_base, unity_size);

        return true;
    }

    // ---- Process / Module Lookup ----

    bool FindProcess(const wchar_t* name) {
        if (mode == MODE_KERNEL) {
            DRIVER_PID_REQ req = {};
            wcsncpy_s(req.name, name, 259);

            DWORD returned = 0;
            BOOL ok = DeviceIoControl(
                device, IOCTL_DRIVER_GET_PID,
                &req, sizeof(req),
                &req, sizeof(req),
                &returned, nullptr
            );
            if (ok && returned > 0) {
                pid = req.pid;
                return true;
            }
            return false;
        }

        // Usermode direct lookup via Toolhelp32
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(entry);

        bool found = false;
        if (Process32FirstW(snap, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, name) == 0) {
                    pid = entry.th32ProcessID;
                    found = true;
                    break;
                }
            } while (Process32NextW(snap, &entry));
        }
        CloseHandle(snap);

        if (found) {
            if (hProcess && hProcess != INVALID_HANDLE_VALUE) {
                CloseHandle(hProcess);
                hProcess = NULL;
            }
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (!hProcess) {
                hProcess = OpenProcess(
                    PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                    PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION,
                    FALSE, pid
                );
            }
            return (hProcess != NULL && hProcess != INVALID_HANDLE_VALUE);
        }

        return false;
    }

    bool FindModule(const wchar_t* name, uint64_t& base, uint32_t& size) {
        if (mode == MODE_KERNEL) {
            DRIVER_MODULE_REQ req = {};
            req.pid = pid;
            wcsncpy_s(req.name, name, 259);

            DWORD returned = 0;
            BOOL ok = DeviceIoControl(
                device, IOCTL_DRIVER_GET_MODULE,
                &req, sizeof(req),
                &req, sizeof(req),
                &returned, nullptr
            );
            if (ok && returned > 0) {
                base = req.base;
                size = req.size;
                return true;
            }
            return false;
        }

        // Usermode direct lookup via Toolhelp32 Snapshot
        if (!pid) return false;

        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return false;

        MODULEENTRY32W me = {};
        me.dwSize = sizeof(me);

        bool found = false;
        if (Module32FirstW(snap, &me)) {
            do {
                if (_wcsicmp(me.szModule, name) == 0) {
                    base = reinterpret_cast<uint64_t>(me.modBaseAddr);
                    size = me.modBaseSize;
                    found = true;
                    break;
                }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
        return found;
    }

    // ---- Memory Read ----

    bool ReadRaw(uint64_t address, void* buffer, size_t size) {
        if (!address || !buffer || size == 0) return false;

        if (mode == MODE_KERNEL) {
            DRIVER_READ_REQ req;
            req.pid     = pid;
            req.address = address;
            req.size    = static_cast<ULONG>(size);

            DWORD returned = 0;
            return DeviceIoControl(
                device, IOCTL_DRIVER_READ,
                &req, sizeof(req),
                buffer, static_cast<DWORD>(size),
                &returned, nullptr
            ) && (returned > 0);
        }

        // Usermode direct ReadProcessMemory
        if (!hProcess || hProcess == INVALID_HANDLE_VALUE) return false;
        SIZE_T bytes_read = 0;
        return (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read) && bytes_read > 0);
    }

    template <typename T>
    T Read(uint64_t address) {
        T value{};
        ReadRaw(address, &value, sizeof(T));
        return value;
    }

    uint64_t ReadChain(uint64_t base, const std::vector<uint64_t>& offsets) {
        uint64_t addr = base;
        for (size_t i = 0; i < offsets.size(); i++) {
            addr = Read<uint64_t>(addr + offsets[i]);
            if (addr == 0) return 0;
        }
        return addr;
    }

    std::wstring ReadWString(uint64_t address, size_t max_len = 128) {
        std::vector<wchar_t> buf(max_len + 1, 0);
        ReadRaw(address, buf.data(), max_len * sizeof(wchar_t));
        return std::wstring(buf.data());
    }

    // ---- Memory Write ----

    bool WriteRaw(uint64_t address, const void* buffer, size_t size) {
        if (!address || !buffer || size == 0) return false;

        if (mode == MODE_KERNEL) {
            std::vector<uint8_t> buf(sizeof(DRIVER_WRITE_REQ) + size);
            auto* req = reinterpret_cast<DRIVER_WRITE_REQ*>(buf.data());
            req->pid     = pid;
            req->address = address;
            req->size    = static_cast<ULONG>(size);
            memcpy(buf.data() + sizeof(DRIVER_WRITE_REQ), buffer, size);

            DWORD returned = 0;
            return DeviceIoControl(
                device, IOCTL_DRIVER_WRITE,
                buf.data(), static_cast<DWORD>(buf.size()),
                nullptr, 0,
                &returned, nullptr
            );
        }

        // Usermode direct WriteProcessMemory
        if (!hProcess || hProcess == INVALID_HANDLE_VALUE) return false;
        SIZE_T bytes_written = 0;
        if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), buffer, size, &bytes_written) && bytes_written == size) {
            return true;
        }

        // If direct write fails, try VirtualProtectEx
        DWORD old_protect = 0;
        if (VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &old_protect)) {
            BOOL ok = WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), buffer, size, &bytes_written);
            VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(address), size, old_protect, &old_protect);
            return (ok && bytes_written == size);
        }

        return false;
    }

    template <typename T>
    bool Write(uint64_t address, const T& value) {
        return WriteRaw(address, &value, sizeof(T));
    }

    // ---- Pattern Scanner ----

    uint64_t PatternScan(uint64_t module_base, uint32_t module_size, const char* pattern) {
        std::vector<int> bytes;
        const char* p = pattern;
        while (*p) {
            if (*p == ' ') { p++; continue; }
            if (*p == '?') {
                bytes.push_back(-1);
                p++;
                if (*p == '?') p++;
                continue;
            }
            char hex[3] = { p[0], p[1], 0 };
            bytes.push_back(static_cast<int>(strtol(hex, nullptr, 16)));
            p += 2;
        }

        if (bytes.empty() || !module_base || !module_size) return 0;

        const size_t CHUNK_SIZE = 0x100000; // 1 MB
        std::vector<uint8_t> buffer(CHUNK_SIZE);

        for (uint32_t offset = 0; offset < module_size; offset += static_cast<uint32_t>(CHUNK_SIZE - bytes.size())) {
            uint32_t read_size = min(static_cast<uint32_t>(CHUNK_SIZE), module_size - offset);
            if (!ReadRaw(module_base + offset, buffer.data(), read_size))
                continue;

            for (uint32_t i = 0; i < read_size - static_cast<uint32_t>(bytes.size()); i++) {
                bool found = true;
                for (size_t j = 0; j < bytes.size(); j++) {
                    if (bytes[j] != -1 && buffer[i + j] != static_cast<uint8_t>(bytes[j])) {
                        found = false;
                        break;
                    }
                }
                if (found)
                    return module_base + offset + i;
            }
        }

        return 0;
    }

    uint64_t PatternScanRIP(uint64_t module_base, uint32_t module_size,
                            const char* pattern, int instruction_offset, int instruction_size) {
        uint64_t addr = PatternScan(module_base, module_size, pattern);
        if (!addr) return 0;

        int32_t rip_offset = Read<int32_t>(addr + instruction_offset);
        return addr + instruction_size + rip_offset;
    }
};
