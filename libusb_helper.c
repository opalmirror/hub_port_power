/**************************************************************************//**
 * @file libusb_helper.c
 * @brief Supplement older versions of libusb with missing entry points
 *
 * @copyright 2012-2013 Datalogic ADC Inc. All rights reserved.
 *****************************************************************************/

#include <libusb.h>
#include "libusb_helper.h"

const struct libusb_version libusb_version_dummy = {
    .major = 1,
    .minor = 0,
    .micro = 0,
    .nano = 0,
    .rc = "-dummy",
    .describe = "libusb_helper"
};

const char *libusb_error_name(int errcode)
{
    if (errcode < 0) {
        switch (errcode) {
            case LIBUSB_ERROR_IO:
                return "Input/output error";
            case LIBUSB_ERROR_INVALID_PARAM:
                return "Invalid parameter";
            case LIBUSB_ERROR_ACCESS:
                return "Access denied (insufficient permissions)";
            case LIBUSB_ERROR_NO_DEVICE:
                return "No such device (it may have been disconnected)";
            case LIBUSB_ERROR_NOT_FOUND:
                return "Entity not found";
            case LIBUSB_ERROR_BUSY:
                return "Resource busy";
            case LIBUSB_ERROR_TIMEOUT:
                return "Operation timed out";
            case LIBUSB_ERROR_OVERFLOW:
                return "Overflow";
            case LIBUSB_ERROR_PIPE:
                return "Pipe error";
            case LIBUSB_ERROR_INTERRUPTED:
                return "System call interrupted (perhaps due to signal)";
            case LIBUSB_ERROR_NO_MEM:
                return "Insufficient memory";
            case LIBUSB_ERROR_NOT_SUPPORTED:
                return "Operation not supported or unimplemented on this platform";
            default:
                break;
        }
       return "Other error";
    }
    return "Success";
}

const struct libusb_version *libusb_get_version(void)
{
    return &libusb_version_dummy;
}

/*
 * vim:ts=4:sw=4:et
 */
