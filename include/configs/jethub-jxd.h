/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for JetHome devices
 * Copyright (c) 2023 JetHome, Viacheslav Bocharov
 * Author: Viacheslav Bocharov <adeep@lexina.in>
 */

#ifndef __JETHUB_CONFIG_H
#define __JETHUB_CONFIG_H


#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"gpio read userbutton PE5; " \
		"if test ${userbutton} -eq 1;  then " \
			"setenv bootargs \"console=ttyS3,115200 ubi.mtd=ubifs root=ubi0:rootfs rw rootfstype=ubifs rootwait\";" \
			"mtd read dtb ${fdt_addr_r}; " \
			"fdt addr ${fdt_addr_r}; " \
			"fdt resize 65536; " \
			"mtd read kernela ${loadaddr}; " \
			"bootz ${loadaddr} - ${fdt_addr_r};" \
		"fi;\0"

#define BOOTENV_DEV_NAME_RESCUE(devtypeu, devtypel, instance) \
	"rescue "

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(RESCUE, rescue, na) \
	func(FEL, fel, na) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)
#endif

#include <configs/sunxi-common.h>

#endif /* __JETHUB_CONFIG_H */
