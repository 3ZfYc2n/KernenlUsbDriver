#pragma once

#pragma warning(disable:4201)  // nameless struct/union
#include <winioctl.h>
#pragma warning(default:4201)


//Define device type in the "user-defined" range
#define FILE_DEVICE_USBOTP 0x00008010

//Define the function code and ensure it's unique within your device type.
#define IOCTL_USBOTP_VALIDATE_OTP CTL_CODE(FILE_DEVICE_USBOTP, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)