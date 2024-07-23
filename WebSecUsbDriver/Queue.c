/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, WebSecUsbDriverQueueInitialize)
#endif

NTSTATUS
WebSecUsbDriverQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;

    PAGED_CODE();
    
    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = WebSecUsbDriverEvtIoDeviceControl;
    queueConfig.EvtIoStop = WebSecUsbDriverEvtIoStop;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if( !NT_SUCCESS(status) ) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }

    return status;
}

VOID
WebSecUsbDriverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
    NTSTATUS status = STATUS_SUCCESS;

    switch (IoControlCode) {
    case IOCTL_USBOTP_VALIDATE_OTP:
        status = HandleValidateOtp(Request, OutputBufferLength, InputBufferLength);
        break;
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "Invalid IOCTL %u", IoControlCode);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);

    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, sizeof(ULONG));

    return;
}

NTSTATUS HandleValidateOtp(
    WDFREQUEST Request,
    size_t OutputBufferLength,
    size_t InputBufferLength
)
{
    NTSTATUS status = STATUS_SUCCESS;
    // Define your OTP input and output structures
    PULONG inputOtp = NULL;
    PULONG outputResult = NULL;

    // Ensure the input buffer length is as expected
    if (InputBufferLength < sizeof(ULONG)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Ensure the output buffer length is as expected
    if (OutputBufferLength < sizeof(ULONG)) {
        return STATUS_BUFFER_TOO_SMALL;
    }


    // Retrieve the input buffer containing the OTP
    status = WdfRequestRetrieveInputBuffer(Request, sizeof(ULONG), (PVOID*)&inputOtp, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Assume the input is a ULONG for OTP and output is a ULONG for result
    status = WdfRequestRetrieveInputBuffer(Request, sizeof(ULONG), &inputOtp, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Retrieve the output buffer where the validation result will be stored
    status = WdfRequestRetrieveOutputBuffer(Request, sizeof(ULONG), (PVOID*)&outputResult, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_QUEUE,
        "%!FUNC! InputOTP %lu",
        (ULONG)*inputOtp);

    // Validate the OTP - Placeholder for actual validation logic
    *outputResult = 15;
        //ValidateOtp(*inputOtp); // Assume ValidateOtp returns a ULONG

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_QUEUE,
        "%!FUNC! OutputResult %d",
        (ULONG)*outputResult);


    return status;
}

BOOLEAN ValidateOtp(ULONG inputOtp) {
    // Implement the validation logic here
    // Return TRUE if valid, FALSE otherwise
    return (inputOtp == 123456); // Example logic
}

VOID
WebSecUsbDriverEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d", 
                Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it either postpones further processing
    //   of the request and calls WdfRequestStopAcknowledge, or it calls WdfRequestComplete
    //   with a completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //  
    //   The driver must call WdfRequestComplete only once, to either complete or cancel
    //   the request. To ensure that another thread does not call WdfRequestComplete
    //   for the same request, the EvtIoStop callback must synchronize with the driver's
    //   other event callback functions, for instance by using interlocked operations.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time. For example, the driver might
    // take no action for requests that are completed in one of the driver’s request handlers.
    //

    return;
}
