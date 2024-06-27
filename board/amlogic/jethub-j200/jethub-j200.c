// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 JetHome
 * Author: Viacheslav Bocharov <adeep@lexina.in>
 *
 */

#include <common.h>
#include <dm.h>
#include <adc.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <power/regulator.h>
#include <asm/io.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/boot.h>
#include <asm/gpio.h>

#define EFUSE_SN_OFFSET		50
#define EFUSE_SN_SIZE		32
#define EFUSE_MAC_OFFSET	0
#define EFUSE_MAC_SIZE		6
#define EFUSE_USID_OFFSET	18
#define EFUSE_USID_SIZE		32

int mmc_get_env_dev(void)
{
	if (meson_get_boot_device() == BOOT_DEVICE_EMMC)
		return 1;
	return 0;
}

static void init_board_power(void)
{
	struct gpio_desc *usb_hog;
	int ret = 0;

	ret = gpio_hog_lookup_name("usb-hub-hog", &usb_hog);
	if (ret)
		goto err;

	ret = dm_gpio_set_value(usb_hog, false);
	if (ret && ret != -EALREADY)
		goto err;

	return;
err:
	printf("%s: Failed to init USB power %i\n", __func__, ret);
}

int misc_init_r(void)
{
	u8 mac_addr[EFUSE_MAC_SIZE];
	char serial[EFUSE_SN_SIZE];
	char usid[EFUSE_USID_SIZE];
	ssize_t len;

	init_board_power();

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  mac_addr, EFUSE_MAC_SIZE);
		if (len == EFUSE_MAC_SIZE && is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
		else {
			printf("eFuse MAC address is not valid\n");
			meson_generate_serial_ethaddr();
		}
	}

	if (!env_get("serial")) {
		len = meson_sm_read_efuse(EFUSE_SN_OFFSET, serial,
					  EFUSE_SN_SIZE);
		if (len == EFUSE_SN_SIZE)
			env_set("serial", serial);
	}

	if (!env_get("usid")) {
		len = meson_sm_read_efuse(EFUSE_USID_OFFSET, usid,
					  EFUSE_USID_SIZE);
		if (len == EFUSE_USID_SIZE)
			env_set("usid", usid);
	}

	return 0;
}

