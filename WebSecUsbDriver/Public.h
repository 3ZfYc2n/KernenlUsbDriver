/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_WebSecUsbDriver,
    0x2086f6e3,0x4080,0x4c5d,0xb4,0xd6,0x00,0xae,0xb7,0x7e,0xc4,0x06);
// {2086f6e3-4080-4c5d-b4d6-00aeb77ec406}


//
// Device type           -- in the "User Defined" range."
//
#define FILE_DEVICE_USBOTP 40001

//
// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
//
#define IOCTL_USBOTP_VALIDATE_OTP \
    CTL_CODE( FILE_DEVICE_USBOTP, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS  )
