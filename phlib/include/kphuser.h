#ifndef _PH_KPHUSER_H
#define _PH_KPHUSER_H

#include <kphapi.h>
#include <kphcomms.h>

#ifdef __cplusplus
extern "C" {
#endif

PHLIBAPI
NTSTATUS
NTAPI
KphConnect(
    _In_ PPH_STRINGREF ServiceName,
    _In_ PPH_STRINGREF FileName,
    _In_opt_ PKPH_COMMS_CALLBACK Callback
    );

PHLIBAPI
NTSTATUS
NTAPI
KphSetParameters(
    _In_ PPH_STRINGREF ServiceName
    );

PHLIBAPI
BOOLEAN
NTAPI
KphParametersExists(
    _In_z_ PWSTR ServiceName
    );

PHLIBAPI
NTSTATUS
NTAPI
KphResetParameters(
    _In_z_ PWSTR ServiceName
    );

PHLIBAPI
VOID
NTAPI
KphSetServiceSecurity(
    _In_ SC_HANDLE ServiceHandle
    );

PHLIBAPI
NTSTATUS
NTAPI
KphInstall(
    _In_ PPH_STRINGREF ServiceName,
    _In_ PPH_STRINGREF FileName
    );

PHLIBAPI
NTSTATUS
NTAPI
KphUninstall(
    _In_z_ PWSTR ServiceName
    );

PHLIBAPI
NTSTATUS
NTAPI
KphGetInformerSettings(
    _Out_ PKPH_INFORMER_SETTINGS Settings
    );

PHLIBAPI
NTSTATUS
NTAPI
KphSetInformerSettings(
    _In_ PKPH_INFORMER_SETTINGS Settings
    );

PHLIBAPI
NTSTATUS
NTAPI
KphOpenProcess(
    _Out_ PHANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ PCLIENT_ID ClientId
    );

PHLIBAPI
NTSTATUS
NTAPI
KphOpenProcessToken(
    _In_ HANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _Out_ PHANDLE TokenHandle
    );

PHLIBAPI
NTSTATUS
NTAPI
KphOpenProcessJob(
    _In_ HANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _Out_ PHANDLE JobHandle
    );

PHLIBAPI
NTSTATUS
NTAPI
KphTerminateProcess(
    _In_ HANDLE ProcessHandle,
    _In_ NTSTATUS ExitStatus
    );

PHLIBAPI
NTSTATUS
NTAPI
KphReadVirtualMemoryUnsafe(
    _In_opt_ HANDLE ProcessHandle,
    _In_ PVOID BaseAddress,
    _Out_writes_bytes_(BufferSize) PVOID Buffer,
    _In_ SIZE_T BufferSize,
    _Inout_opt_ PSIZE_T NumberOfBytesRead
    );

PHLIBAPI
NTSTATUS
NTAPI
KphOpenThread(
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ PCLIENT_ID ClientId
    );

PHLIBAPI
NTSTATUS
NTAPI
KphOpenThreadProcess(
    _In_ HANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _Out_ PHANDLE ProcessHandle
    );

PHLIBAPI
NTSTATUS
NTAPI
KphCaptureStackBackTraceThread(
    _In_ HANDLE ThreadHandle,
    _In_ ULONG FramesToSkip,
    _In_ ULONG FramesToCapture,
    _Out_writes_(FramesToCapture) PVOID *BackTrace,
    _Inout_opt_ PULONG CapturedFrames,
    _Inout_opt_ PULONG BackTraceHash
    );

PHLIBAPI
NTSTATUS
NTAPI
KphEnumerateProcessHandles(
    _In_ HANDLE ProcessHandle,
    _Out_writes_bytes_(BufferLength) PVOID Buffer,
    _In_opt_ ULONG BufferLength,
    _Inout_opt_ PULONG ReturnLength
    );

PHLIBAPI
NTSTATUS
NTAPI
KphEnumerateProcessHandles2(
    _In_ HANDLE ProcessHandle,
    _Out_ PKPH_PROCESS_HANDLE_INFORMATION *Handles
    );

PHLIBAPI
NTSTATUS
NTAPI
KphQueryInformationObject(
    _In_ HANDLE ProcessHandle,
    _In_ HANDLE Handle,
    _In_ KPH_OBJECT_INFORMATION_CLASS ObjectInformationClass,
    _Out_writes_bytes_opt_(ObjectInformationLength) PVOID ObjectInformation,
    _In_ ULONG ObjectInformationLength,
    _Inout_opt_ PULONG ReturnLength
    );

PHLIBAPI
NTSTATUS
NTAPI
KphSetInformationObject(
    _In_ HANDLE ProcessHandle,
    _In_ HANDLE Handle,
    _In_ KPH_OBJECT_INFORMATION_CLASS ObjectInformationClass,
    _In_reads_bytes_(ObjectInformationLength) PVOID ObjectInformation,
    _In_ ULONG ObjectInformationLength
    );

PHLIBAPI
NTSTATUS
NTAPI
KphOpenDriver(
    _Out_ PHANDLE DriverHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
    );

PHLIBAPI
NTSTATUS
NTAPI
KphQueryInformationDriver(
    _In_ HANDLE DriverHandle,
    _In_ DRIVER_INFORMATION_CLASS DriverInformationClass,
    _Out_writes_bytes_opt_(DriverInformationLength) PVOID DriverInformation,
    _In_ ULONG DriverInformationLength,
    _Inout_opt_ PULONG ReturnLength
    );

PHLIBAPI
NTSTATUS
NTAPI
KphQueryInformationProcess(
    _In_ HANDLE ProcessHandle,
    _In_ KPH_PROCESS_INFORMATION_CLASS ProcessInformationClass,
    _Out_writes_bytes_opt_(ProcessInformationLength) PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength,
    _Inout_opt_ PULONG ReturnLength
    );

PHLIBAPI
KPH_PROCESS_STATE
NTAPI
KphGetProcessState(
    _In_ HANDLE ProcessHandle
    );

PHLIBAPI
KPH_PROCESS_STATE
NTAPI
KphGetCurrentProcessState(
    VOID
    );

typedef enum _KPH_LEVEL
{
    KphLevelNone,
    KphLevelMin,
    KphLevelLow,
    KphLevelMed,
    KphLevelHigh,
    KphLevelMax

} KPH_LEVEL;

PHLIBAPI
KPH_LEVEL
NTAPI
KphProcessLevel(
    _In_ HANDLE ProcessHandle
    );

PHLIBAPI
KPH_LEVEL
NTAPI
KphLevel(
    VOID
    );

// kphdata

PHLIBAPI
NTSTATUS
NTAPI
KphInitializeDynamicConfiguration(
    _Out_ PKPH_DYN_CONFIGURATION Configuration 
    );

#ifdef __cplusplus
}
#endif

#endif
