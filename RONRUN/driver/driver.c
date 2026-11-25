// ============================================================================
//  Rob Run — Kernel Driver
//  Memory R/W via MmCopyVirtualMemory, process/module lookup
//  Build with WDK. Map with KDMapper or manual mapper (no service registration).
// ============================================================================

#include <ntddk.h>
#include <ntstrsafe.h>
#include "ioctl.h"

// ---- Undocumented but stable kernel exports ----

NTKERNELAPI NTSTATUS MmCopyVirtualMemory(
    PEPROCESS       SourceProcess,
    PVOID           SourceAddress,
    PEPROCESS       TargetProcess,
    PVOID           TargetAddress,
    SIZE_T          BufferSize,
    KPROCESSOR_MODE PreviousMode,
    PSIZE_T         ReturnSize
);

NTKERNELAPI PVOID  PsGetProcessPeb(PEPROCESS Process);
NTKERNELAPI PUCHAR PsGetProcessImageFileName(PEPROCESS Process);

// ---- Forward declarations ----

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD     DriverUnload;

_Dispatch_type_(IRP_MJ_CREATE)         DRIVER_DISPATCH DispatchCreate;
_Dispatch_type_(IRP_MJ_CLOSE)          DRIVER_DISPATCH DispatchClose;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DispatchIoctl;

// ============================================================================
//  Memory Operations
// ============================================================================

static NTSTATUS KReadProcessMemory(
    ULONG pid, ULONG64 address, PVOID buffer, SIZE_T size
) {
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &process);
    if (!NT_SUCCESS(status))
        return status;

    SIZE_T bytes = 0;
    status = MmCopyVirtualMemory(
        process,            (PVOID)address,
        PsGetCurrentProcess(), buffer,
        size, KernelMode, &bytes
    );

    ObDereferenceObject(process);
    return status;
}

static NTSTATUS KWriteProcessMemory(
    ULONG pid, ULONG64 address, PVOID buffer, SIZE_T size
) {
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &process);
    if (!NT_SUCCESS(status))
        return status;

    SIZE_T bytes = 0;
    status = MmCopyVirtualMemory(
        PsGetCurrentProcess(), buffer,
        process,            (PVOID)address,
        size, KernelMode, &bytes
    );

    ObDereferenceObject(process);
    return status;
}

// ============================================================================
//  Process Lookup by Name
// ============================================================================

static NTSTATUS KFindProcessByName(PCWCHAR name, PULONG out_pid) {
    UNICODE_STRING uni_name;
    RtlInitUnicodeString(&uni_name, name);

    ANSI_STRING ansi_target;
    NTSTATUS status = RtlUnicodeStringToAnsiString(&ansi_target, &uni_name, TRUE);
    if (!NT_SUCCESS(status))
        return status;

    for (ULONG pid = 4; pid < 0x10000; pid += 4) {
        PEPROCESS process = NULL;
        if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &process)))
            continue;

        PUCHAR img = PsGetProcessImageFileName(process);
        if (img) {
            ANSI_STRING ansi_img;
            RtlInitAnsiString(&ansi_img, (PCSZ)img);
            if (RtlCompareString(&ansi_img, &ansi_target, TRUE) == 0) {
                *out_pid = pid;
                ObDereferenceObject(process);
                RtlFreeAnsiString(&ansi_target);
                return STATUS_SUCCESS;
            }
        }
        ObDereferenceObject(process);
    }

    RtlFreeAnsiString(&ansi_target);
    return STATUS_NOT_FOUND;
}

// ============================================================================
//  Module Base Lookup (walks PEB->Ldr->InMemoryOrderModuleList)
// ============================================================================

// Partial PEB_LDR_DATA for reading InMemoryOrderModuleList head
typedef struct _K_LDR_DATA {
    ULONG       Length;
    BOOLEAN     Initialized;
    PVOID       SsHandle;
    LIST_ENTRY  InLoadOrderModuleList;
    LIST_ENTRY  InMemoryOrderModuleList;
} K_LDR_DATA;

// Partial LDR_DATA_TABLE_ENTRY
typedef struct _K_LDR_ENTRY {
    LIST_ENTRY      InLoadOrderLinks;
    LIST_ENTRY      InMemoryOrderLinks;
    LIST_ENTRY      InInitializationOrderLinks;
    PVOID           DllBase;
    PVOID           EntryPoint;
    ULONG           SizeOfImage;
    UNICODE_STRING  FullDllName;
    UNICODE_STRING  BaseDllName;
} K_LDR_ENTRY;

#define PEB_LDR_OFFSET 0x18   // PEB->Ldr (x64)

static NTSTATUS KFindModuleBase(
    ULONG pid, PCWCHAR module_name, PULONG64 out_base, PULONG out_size
) {
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &process);
    if (!NT_SUCCESS(status))
        return status;

    PVOID peb = PsGetProcessPeb(process);
    if (!peb) {
        ObDereferenceObject(process);
        return STATUS_NOT_FOUND;
    }

    // Read PEB->Ldr pointer
    PVOID ldr_ptr = NULL;
    SIZE_T bytes = 0;
    status = MmCopyVirtualMemory(
        process, (PVOID)((ULONG_PTR)peb + PEB_LDR_OFFSET),
        PsGetCurrentProcess(), &ldr_ptr,
        sizeof(PVOID), KernelMode, &bytes
    );
    if (!NT_SUCCESS(status) || !ldr_ptr) {
        ObDereferenceObject(process);
        return STATUS_NOT_FOUND;
    }

    // Read InMemoryOrderModuleList head
    LIST_ENTRY list_head = { 0 };
    ULONG_PTR head_addr = (ULONG_PTR)ldr_ptr +
        FIELD_OFFSET(K_LDR_DATA, InMemoryOrderModuleList);

    status = MmCopyVirtualMemory(
        process, (PVOID)head_addr,
        PsGetCurrentProcess(), &list_head,
        sizeof(LIST_ENTRY), KernelMode, &bytes
    );
    if (!NT_SUCCESS(status)) {
        ObDereferenceObject(process);
        return STATUS_NOT_FOUND;
    }

    // Walk the doubly-linked list
    PLIST_ENTRY current = list_head.Flink;

    for (int i = 0; i < 512 && (ULONG_PTR)current != head_addr; i++) {
        ULONG_PTR entry_addr = (ULONG_PTR)current -
            FIELD_OFFSET(K_LDR_ENTRY, InMemoryOrderLinks);

        K_LDR_ENTRY entry = { 0 };
        status = MmCopyVirtualMemory(
            process, (PVOID)entry_addr,
            PsGetCurrentProcess(), &entry,
            sizeof(K_LDR_ENTRY), KernelMode, &bytes
        );
        if (!NT_SUCCESS(status))
            break;

        // Read BaseDllName
        if (entry.BaseDllName.Length > 0 && entry.BaseDllName.Length < 520) {
            WCHAR name_buf[260] = { 0 };
            SIZE_T name_len = min(entry.BaseDllName.Length, sizeof(name_buf) - 2);

            status = MmCopyVirtualMemory(
                process, entry.BaseDllName.Buffer,
                PsGetCurrentProcess(), name_buf,
                name_len, KernelMode, &bytes
            );
            if (NT_SUCCESS(status)) {
                UNICODE_STRING uni_found, uni_target;
                RtlInitUnicodeString(&uni_found, name_buf);
                RtlInitUnicodeString(&uni_target, module_name);

                if (RtlCompareUnicodeString(&uni_found, &uni_target, TRUE) == 0) {
                    *out_base = (ULONG64)(ULONG_PTR)entry.DllBase;
                    *out_size = entry.SizeOfImage;
                    ObDereferenceObject(process);
                    return STATUS_SUCCESS;
                }
            }
        }

        current = entry.InMemoryOrderLinks.Flink;
    }

    ObDereferenceObject(process);
    return STATUS_NOT_FOUND;
}

// ============================================================================
//  IOCTL Dispatch
// ============================================================================

NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer    = Irp->AssociatedIrp.SystemBuffer;
    ULONG in_size   = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG out_size  = stack->Parameters.DeviceIoControl.OutputBufferLength;

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG bytes_returned = 0;

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {

    // ---- Read Memory ----
    case IOCTL_DRIVER_READ: {
        if (in_size < sizeof(DRIVER_READ_REQ)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        // Copy request before overwriting buffer with output
        DRIVER_READ_REQ req;
        RtlCopyMemory(&req, buffer, sizeof(DRIVER_READ_REQ));

        if (out_size < req.size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = KReadProcessMemory(req.pid, req.address, buffer, req.size);
        if (NT_SUCCESS(status))
            bytes_returned = req.size;
        break;
    }

    // ---- Write Memory ----
    case IOCTL_DRIVER_WRITE: {
        if (in_size < sizeof(DRIVER_WRITE_REQ)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        DRIVER_WRITE_REQ* req = (DRIVER_WRITE_REQ*)buffer;
        ULONG total_needed = sizeof(DRIVER_WRITE_REQ) + req->size;
        if (in_size < total_needed) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        PVOID data = (PVOID)((ULONG_PTR)buffer + sizeof(DRIVER_WRITE_REQ));
        status = KWriteProcessMemory(req->pid, req->address, data, req->size);
        break;
    }

    // ---- Get Process ID ----
    case IOCTL_DRIVER_GET_PID: {
        if (in_size < sizeof(DRIVER_PID_REQ) || out_size < sizeof(DRIVER_PID_REQ)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        DRIVER_PID_REQ* req = (DRIVER_PID_REQ*)buffer;
        status = KFindProcessByName(req->name, &req->pid);
        if (NT_SUCCESS(status))
            bytes_returned = sizeof(DRIVER_PID_REQ);
        break;
    }

    // ---- Get Module Base ----
    case IOCTL_DRIVER_GET_MODULE: {
        if (in_size < sizeof(DRIVER_MODULE_REQ) || out_size < sizeof(DRIVER_MODULE_REQ)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        DRIVER_MODULE_REQ* req = (DRIVER_MODULE_REQ*)buffer;
        status = KFindModuleBase(req->pid, req->name, &req->base, &req->size);
        if (NT_SUCCESS(status))
            bytes_returned = sizeof(DRIVER_MODULE_REQ);
        break;
    }

    } // switch

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = bytes_returned;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

// ============================================================================
//  Create / Close / Unload
// ============================================================================

NTSTATUS DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symlink;
    RtlInitUnicodeString(&symlink, DRV_SYMLINK_NAME);
    IoDeleteSymbolicLink(&symlink);

    if (DriverObject->DeviceObject)
        IoDeleteDevice(DriverObject->DeviceObject);
}

// ============================================================================
//  Entry Point
// ============================================================================

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;

    UNICODE_STRING dev_name, sym_name;
    RtlInitUnicodeString(&dev_name, DRV_DEVICE_NAME);
    RtlInitUnicodeString(&sym_name, DRV_SYMLINK_NAME);

    PDEVICE_OBJECT device = NULL;
    NTSTATUS status = IoCreateDevice(
        DriverObject, 0, &dev_name,
        DRV_DEVICE_TYPE, 0, FALSE, &device
    );
    if (!NT_SUCCESS(status))
        return status;

    status = IoCreateSymbolicLink(&sym_name, &dev_name);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(device);
        return status;
    }

    return STATUS_SUCCESS;
}
