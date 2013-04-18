/**************************************************************************//**
 * @file libusb_helper.c
 * @brief Supplement older versions of libusb with missing entry points
 * @details Derived from public libusb header file
 *
 * Public libusb header file
 * Copyright (C) 2007-2008 Daniel Drake <dsd@gentoo.org>
 * Copyright (c) 2001 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
