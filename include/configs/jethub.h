/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for JetHome devices
 * Copyright (C) 2021 Vyacheslav Bocharov
 * Author: Vyacheslav Bocharov <adeep@lexina.in>
 */

#include <config_distro_bootcmd.h>

#ifndef __JETHUB_CONFIG_H
#define __JETHUB_CONFIG_H

#if defined(CONFIG_MESON_AXG)
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"if gpio input periphs-banks10; then " \
		"run bootcmd_mmc0; " \
		"run bootcmd_usb0;" \
		"fi;\0"
#else
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"if test \"${userbutton}\" = \"true\"; then " \
		"echo \"Initializing USB...\"; " \
		"usb reset; " \
		"if test -e usb 0:1 rescue.img; then " \
		"echo \"Rescue image found, starting recovery...\"; " \
		"load usb 0:1 ${loadaddr} rescue.img; " \
		"mmc write ${loadaddr} 0x30000 0x4000; " \
		"reset; " \
		"else " \
		"echo \"No rescue image found, booting normally...\"; " \
		"run boot_script; " \
		"fi; " \
		"fi;\0"
#endif

#define BOOTENV_DEV_NAME_RESCUE(devtypeu, devtypel, instance) \
	"rescue "

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(RESCUE, rescue, na) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif

#include <configs/meson64.h>

#endif /* __JETHUB_CONFIG_H */
