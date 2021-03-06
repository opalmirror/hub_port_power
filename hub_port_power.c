/**************************************************************************/
/**
 * @file hub_port_power.c
 * @brief USB hub port power set/clear program
 *
 * @copyright 2012-2016 Datalogic ADC Inc. <jadetechnicalhelp@datalogic.com>
 *
 * License: LGPLv2.1+ - see accompanying file, COPYING.LGPL-2.1
 *
 * This application is free software; you can redistribute it and/or
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
 * License along with this application; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libusb.h>

#if LIBUSB_HELPER==1            // Accommodate old versions of libusb
#include "libusb_helper.h"
#endif

enum
{
    LIBUSB_DEBUG_LEVEL = 3,     // Level 3 advised for software debug
    USB_RT_PORT = (LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_OTHER),
    USB_PORT_FEAT_POWER = 8,    // USB port power feature code
    MAX_HUB_INSTANCE = 15,      // max matching USB HUB instance
    MAX_HUB_PORT = 7,           // max number of device ports on USB HUB
    MAX_HUB_FIND_RETRIES = 2,   // # of attempts to read device list
    HUB_FIND_RETRY_SLEEP = 4,   // # seconds to wait between hub retries
    MAX_HUB_PORT_POWER_SET_RETRIES = 3, // # of attempts to set port power
    HUB_DEVICE_CONFIGURATION = 1,   // usb dev configuration to set for hubs
    USB_TIMEOUT = 500,          // USB transaction timeout (ms)
};

const char *progname;

/**************************************************************************/
/**
 * @brief emit a command line usage message
 *
 * @param msg
 *   if non-NULL, a message to emit before the usage message
 *****************************************************************************/
void usage(const char *msg)
{
    if (msg)
    {
        fprintf(stderr, "%s: %s\n", progname, msg);
    }
    fprintf(stderr,
        "usage: %s [-q] -v VendorID -p ProductID [-i Instance ]\n"
        "               -n PortNum -s PowerSetting\n", progname);
    fprintf(stderr,
        "  -v VendorID      USB Vendor ID (base 16), ex. for SMSC, use -v 0424\n");
    fprintf(stderr,
        "  -p ProductID     USB Product ID (base 16), ex. for 2514 hub, use -p 2514\n");
    fprintf(stderr,
        "  -i Instance      Use the Instance'th hub matching -v, -p (range 1 to %u)\n",
        MAX_HUB_INSTANCE);
    fprintf(stderr,
        "  -n PortNum       USB Hub Port Number to affect (range 1 to %u)\n",
        MAX_HUB_PORT);
    fprintf(stderr,
        "  -s PowerSetting  Port Power setting (0 = turn off, 1 = turn on)\n");
    fprintf(stderr, "  -q               Quiet; suppress debug output\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "EXAMPLE: if you run run 'lsusb' and see a hub listed like this:\n");
    fprintf(stderr, "  Bus 002 Device 002: ID 110a:0407 Moxa Technologies Co., Ltd.\n");
    fprintf(stderr, "\n");
    fprintf(stderr,
        "Then, to turn off power to port 2 and on for port 3, issue commands:\n");
    fprintf(stderr, "  hub_port_power -v 110a -p 0407 -n 2 -s 0\n");
    fprintf(stderr, "  hub_port_power -v 110a -p 0407 -n 3 -s 1\n");
    exit(1);
}

/**************************************************************************/
/**
 * @brief parse command-line arguments into memory pointed to by params
 *
 * @param ac
 *   count of command line arguments
 *
 * @param av
 *   pointer to base of array of command-line argument string pointers
 *
 * @param pVid
 *   pointer to storage location for USB VendorID extracted from command line
 *
 * @param pPid
 *   pointer to storage location for USB ProductID extracted from command line
 *
 * @param pHub_instance
 *   pointer to storage location for hub Instance extracted from command line
 *
 * @param pPort_num
 *   pointer to storage location for hub Port Number extracted from command line
 *
 * @param pQuiet
 *   pointer to storage location for quiet operation flag
 *
 * @param pPower_setting
 *   pointer to storage location for port Power Setting extracted from command line
 *****************************************************************************/
void parse_args(int ac, char **av, uint16_t * pVid, uint16_t * pPid,
    unsigned int *pHub_instance, unsigned int *pPort_num,
    unsigned int *pPower_setting, unsigned int *pQuiet)
{
    progname = *av++;           // save for debug output
    ac--;

    *pVid = 0;
    *pPid = 0;
    *pHub_instance = 1;
    *pPort_num = 0;
    *pPower_setting = 2;
    *pQuiet = 0;

    for (; ac > 0; ac--, av++)
    {
        if (*av && strcmp(*av, "-v") == 0)
        {
            if (--ac <= 0 || sscanf(*++av, "%hx", pVid) != 1 || *pVid == 0)
            {
                usage("-v takes a hexadecimal argument between 1 and ffff");
            }
        }
        else if (*av && strcmp(*av, "-p") == 0)
        {
            if (--ac <= 0 || sscanf(*++av, "%hx", pPid) != 1 || *pPid == 0)
            {
                usage("-p takes a hexadecimal argument between 1 and ffff");
            }
        }
        else if (*av && strcmp(*av, "-i") == 0)
        {
            if (--ac <= 0 || sscanf(*++av, "%u", pHub_instance) != 1 ||
                *pHub_instance == 0 || *pHub_instance > MAX_HUB_INSTANCE)
            {
                usage("-i takes a numeric argument");
            }
        }
        else if (*av && strcmp(*av, "-n") == 0)
        {
            if (--ac <= 0 || sscanf(*++av, "%u", pPort_num) != 1 ||
                *pPort_num == 0 || *pPort_num > MAX_HUB_PORT)
            {
                usage("-n takes a numeric argument");
            }
        }
        else if (*av && strcmp(*av, "-s") == 0)
        {
            if (--ac <= 0 || sscanf(*++av, "%u", pPower_setting) != 1 ||
                *pPower_setting > 1)
            {
                usage("-s takes a numeric argument of 0 or 1");
            }
        }
        else if (*av && strcmp(*av, "-q") == 0)
        {
            *pQuiet = 1;
        }
        else
        {
            usage("unrecognized command-line argument");
        }
    }
    if (*pVid == 0 && *pPid == 0 && *pPort_num == 0 && *pPower_setting == 2)
    {
        usage(NULL);
    }
    if (*pVid == 0)
    {
        usage("-v VendorID required");
    }
    if (*pPid == 0)
    {
        usage("-p ProductID required");
    }
    if (*pPort_num == 0)
    {
        usage("-n PortNum required");
    }
    if (*pPower_setting == 2)
    {
        usage("-s PowerSetting required");
    }
}

/**************************************************************************/
/**
 * @brief initialize libusb and fill in a LIBUSB context structure
 *
 * @param pUsbctx
 *   pointer to storage location for usb context pointer
 *****************************************************************************/
void init_libusb(libusb_context ** pUsbctx)
{
    int result;

    result = libusb_init(pUsbctx);
    if (result != 0)
    {
        fprintf(stderr, "%s: Unable to initialize libusb: %s\n",
            progname, libusb_error_name(result));
        exit(1);
    }
}

/**************************************************************************/
/**
 * @brief print the version of libusb being used
 *
 * @param usbctx
 *   pointer to usb context
 *
 * @param quiet
 *   suppress debug output
 *****************************************************************************/
void print_libusb_version(libusb_context * usbctx, int quiet)
{
    const struct libusb_version *pVer = libusb_get_version();
    if (pVer == NULL)
    {
        fprintf(stderr, "%s: libusb_get_version returned NULL\n", progname);
        libusb_exit(usbctx);    // close USB library
        exit(1);
    }
    if (!quiet)
    {
        printf("Opened libusb: version %d.%d.%d.%d  %s %s\n",
            pVer->major, pVer->minor, pVer->micro, pVer->nano, pVer->rc, pVer->describe);
    }
}

/**************************************************************************/
/**
 * @brief find the requested USB hub device and fill in its device handle
 *
 * @param usbctx
 *   pointer to usb context
 *
 * @param vid
 *   USB VendorID of hub device to find
 *
 * @param pid
 *   USB ProductID of hub device to find
 *
 * @param hub_instance
 *   instance of matching hub device to find
 *
 * @param pHub_device
 *   pointer to location to store the hub device handle pointer
 *
 * @param quiet
 *   suppress debug output
 *****************************************************************************/
void find_hub_device(libusb_context * usbctx, uint16_t vid, uint16_t pid,
    unsigned int hub_instance, libusb_device_handle ** pHub_device, unsigned int quiet)
{
    int result;
    unsigned int numPasses;
    int instanceFound;
    int deviceFound;
    int numDevices;
    int deviceNum;
    libusb_device **deviceList;
    struct libusb_device_descriptor devDesc;

    *pHub_device = NULL;

    for (numPasses = 0, deviceFound = 0;
        numPasses < MAX_HUB_FIND_RETRIES && !deviceFound; numPasses++)
    {
        if (numPasses > 0)
        {
            sleep(HUB_FIND_RETRY_SLEEP);    // Linux may need a while to enumerate
        }

        numDevices = libusb_get_device_list(usbctx, &deviceList);
        if (numDevices < 0)
        {
            fprintf(stderr, "%s: Could not get USB device list: %s\n", progname,
                libusb_error_name(numDevices));
            continue;
        }

        // search list for specified VID and PID, starting from end of list
        instanceFound = 0;
        for (deviceNum = numDevices - 1; deviceNum >= 0 && !deviceFound; deviceNum--)
        {
            result = libusb_get_device_descriptor(deviceList[deviceNum], &devDesc);
            if (result != 0)
            {
                fprintf(stderr,
                    "%s: Could not get USB device descriptor (%d of %d): %s\n",
                    progname, deviceNum, numDevices, libusb_error_name(result));
                continue;
            }
            if (devDesc.idVendor == vid && devDesc.idProduct == pid &&
                ++instanceFound == hub_instance)
            {
                // Found a matching device, open it
                result = libusb_open(deviceList[deviceNum], pHub_device);
                if (result != 0)
                {
                    fprintf(stderr,
                        "%s: Could not open USB device (%d of %d): %s\n",
                        progname, deviceNum, numDevices, libusb_error_name(result));
                    break;
                }

                deviceFound = 1;
                if (!quiet)
                {
                    printf
                        ("%s: Found matching device instance %u at list entry %d of %d\n",
                        progname, instanceFound, deviceNum + 1, numDevices);
                }
                break;
            }
        }

        if (deviceFound)
        {
            break;
        }
        else
        {
            fprintf(stderr,
                "%s: No device matching vid 0x%04X, pid 0x%04X, instance %u found\n"
                "  in list of %d devices\n",
                progname, vid, pid, hub_instance, numDevices);
            libusb_free_device_list(deviceList, 1);
        }
    }
    if (!deviceFound)
    {
        fprintf(stderr, "%s: hub not found after %d attempts\n", progname,
            MAX_HUB_FIND_RETRIES);
        libusb_exit(usbctx);    // close USB library
        exit(1);
    }
}

/**************************************************************************/
/**
 * @brief get and set (if needed) the hub device's USB Configuration
 *
 * @param usbctx
 *   pointer to usb context
 *
 * @param hub_device
 *   pointer to hub device handle
 *
 * @param hub_configuration
 *   USB device configuration to check and configure (typ. 1)
 *
 * @param quiet
 *   suppress debug output
 *****************************************************************************/
void set_hub_configuration(libusb_context * usbctx,
    libusb_device_handle * hub_device, int hub_configuration, unsigned int quiet)
{
    int result;
    int currConfiguration;

    result = libusb_get_configuration(hub_device, &currConfiguration);
    if (currConfiguration != hub_configuration)
    {
        if (!quiet)
        {
            printf("%s: Setting USB device configuration to %d\n", progname,
                hub_configuration);
        }
        result = libusb_set_configuration(hub_device, hub_configuration);
        if (result != 0)
        {
            fprintf(stderr,
                "%s: Could not set configuration on USB device: %s\n",
                progname, libusb_error_name(result));
            // ignore failure, for now
        }
    }
}

/**************************************************************************/
/**
 * @brief set or clear the power port feature for the given hub and port
 *
 * @param usbctx
 *   pointer to usb context
 *
 * @param hub_device
 *   pointer to hub device handle
 *
 * @param port_num
 *   Number of port to affect (1 - MAX_HUB_PORT)
 *
 * @param port_power_on
 *   If zero, clear port power feature. If non-zero, set port power feature.
 *
 * @param quiet
 *   suppress debug output
 *****************************************************************************/
void set_hub_port_power(libusb_context * usbctx,
    libusb_device_handle * hub_device, int port_num, int port_power_on,
    unsigned int quiet)
{
    int result = 1;
    int numAttempts = 0;

    result = 1;
    while (result != 0 && numAttempts < MAX_HUB_PORT_POWER_SET_RETRIES)
    {
        result = libusb_control_transfer(hub_device, USB_RT_PORT,
            (port_power_on ? LIBUSB_REQUEST_SET_FEATURE :
                LIBUSB_REQUEST_CLEAR_FEATURE),
            USB_PORT_FEAT_POWER, port_num, NULL, 0, USB_TIMEOUT);
        switch (result)
        {
            case 0:
                break;
            case LIBUSB_ERROR_INTERRUPTED:
                fprintf(stderr, "%s: interrupt\n", progname);
                numAttempts++;
                break;
            case LIBUSB_ERROR_TIMEOUT:
                fprintf(stderr, "%s: control transfer timeout\n", progname);
                numAttempts++;
                break;
            case LIBUSB_ERROR_NO_DEVICE:
                // don't retry the no device error
                // it doesn't seem likely to work on a second try
                fprintf(stderr, "%s: device not present\n", progname);
                break;
            case LIBUSB_ERROR_IO:
                fprintf(stderr, "%s: IO error in libusb\n", progname);
                numAttempts++;
                break;
            default:
                break;
        }
    }
    if (result != 0)
    {
        fprintf(stderr, "%s: failed: %s\n", progname, libusb_error_name(result));
        libusb_exit(usbctx);    // close USB library
        exit(1);
    }
    if (!quiet)
    {
        printf("%s: Hub port %d power Port-%s-Feature\n", progname, port_num,
            (port_power_on ? "Set" : "Clear"));
    }
}

/**************************************************************************/
/**
 * @brief main routine
 *
 * @param ac
 *   count of command-line arguments
 *
 * @param av
 *   pointer to base of array of command-line argument string pointers
 *
 * @return N/A; Does not return
 *****************************************************************************/
int main(int ac, char **av)
{
    libusb_context *usbctx;
    libusb_device_handle *hub_device;
    uint16_t vid;
    uint16_t pid;
    unsigned int hub_instance;
    unsigned int port_num;
    unsigned int power_setting;
    unsigned int quiet;

    parse_args(ac, av, &vid, &pid, &hub_instance, &port_num, &power_setting, &quiet);
    init_libusb(&usbctx);
    libusb_set_debug(usbctx, LIBUSB_DEBUG_LEVEL);
    print_libusb_version(usbctx, quiet);
    find_hub_device(usbctx, vid, pid, hub_instance, &hub_device, quiet);
    set_hub_configuration(usbctx, hub_device, HUB_DEVICE_CONFIGURATION, quiet);
    // note: for hub control transfers, interface need not be set
    set_hub_port_power(usbctx, hub_device, port_num, power_setting, quiet);

    exit(0);
}

/*
 * vim:ts=4:sw=4:et
 */
