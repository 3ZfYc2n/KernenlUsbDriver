/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

EXTERN_C_START

//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

    ULONG PrivateDeviceData;  // just a placeholder

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
WebSecUsbDriverQueueInitialize(
    _In_ WDFDEVICE Device
    );

NTSTATUS
HandleValidateOtp(
    WDFREQUEST Request,
    size_t OutputBufferLength,
    size_t InputBufferLength
);

BOOLEAN ValidateOtp(ULONG inputOtp);

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL WebSecUsbDriverEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP WebSecUsbDriverEvtIoStop;

EXTERN_C_END
