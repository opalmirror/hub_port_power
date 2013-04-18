/**************************************************************************//**
 * @file libusb_helper.h
 * @brief Supplement older versions of libusb with missing declarations
 *
 * @details Extracted from header file
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

/*
 * Structure representing the libusb version.
 */
struct libusb_version {
	/** Library major version. */
	const uint16_t major;

	/** Library minor version. */
	const uint16_t minor;

	/** Library micro version. */
	const uint16_t micro;

	/** Library nano version. This field is only nonzero on Windows. */
	const uint16_t nano;

	/** Library release candidate suffix string, e.g. "-rc4". */
	const char *rc;

	/** Output of `git describe --tags` at library build time. */
	const char *describe;
};

const char *libusb_error_name(int errcode);
const struct libusb_version *libusb_get_version(void);

/*
 * vim:ts=4:sw=4:et
 */
