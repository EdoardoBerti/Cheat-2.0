#pragma once
// ============================================================================
//  IOCTL Interface — shared between kernel driver and usermode client
//  Rob Run Cheat — Kernel Driver Communication Protocol
// ============================================================================

#ifdef _KERNEL_MODE
#include <ntddk.h>
#else
#include <Windows.h>
#endif

// Device identifiers — disguised as NVIDIA diagnostics device
#define DRV_DEVICE_NAME     L"\\Device\\NvDiag0"
#define DRV_SYMLINK_NAME    L"\\DosDevices\\NvDiag0"
#define DRV_USERMODE_PATH   L"\\\\.\\NvDiag0"
#define DRV_DEVICE_TYPE     0x00008302

// ---- IOCTL Codes ----
#define IOCTL_DRIVER_READ       CTL_CODE(DRV_DEVICE_TYPE, 0x900, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_DRIVER_WRITE      CTL_CODE(DRV_DEVICE_TYPE, 0x901, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_DRIVER_GET_PID    CTL_CODE(DRV_DEVICE_TYPE, 0x902, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_DRIVER_GET_MODULE CTL_CODE(DRV_DEVICE_TYPE, 0x903, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// ---- Request Structures ----

#pragma pack(push, 1)

// Read request:
//   Input:  DRIVER_READ_REQ (pid, address, size)
//   Output: raw bytes read from target process (size bytes)
typedef struct _DRIVER_READ_REQ {
    ULONG   pid;
    ULONG64 address;
    ULONG   size;
} DRIVER_READ_REQ;

// Write request:
//   Input:  DRIVER_WRITE_REQ header followed by data bytes
//   Output: none
typedef struct _DRIVER_WRITE_REQ {
    ULONG   pid;
    ULONG64 address;
    ULONG   size;
    // BYTE data[size] follows immediately after this struct
} DRIVER_WRITE_REQ;

// Process ID lookup:
//   Input/Output: DRIVER_PID_REQ (name in, pid out)
typedef struct _DRIVER_PID_REQ {
    WCHAR   name[260];
    ULONG   pid;
} DRIVER_PID_REQ;

// Module base lookup:
//   Input/Output: DRIVER_MODULE_REQ (pid+name in, base+size out)
typedef struct _DRIVER_MODULE_REQ {
    ULONG   pid;
    WCHAR   name[260];
    ULONG64 base;
    ULONG   size;
} DRIVER_MODULE_REQ;

#pragma pack(pop)
