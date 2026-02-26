/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Board configuration for JetHub J300 (S7D)
 */

#ifndef __JETHUB_J300_CFG_H__
#define __JETHUB_J300_CFG_H__

#include <asm/amlogic/arch/cpu.h>

/*
 * platform power init config
 */
#define AML_VCCK_INIT_VOLTAGE	  959	    //VCCK power up voltage
#define AML_VDDEE_INIT_VOLTAGE    800       // VDDEE power up voltage

/*Distinguish whether to use efuse to adjust vddee*/
#define CONFIG_PDVFS_ENABLE

/* SMP definitions */
#define CPU_RELEASE_ADDR		secondary_boot_func

/* Serial config */
#define CONFIG_CONS_INDEX 2

/*low console baudrate*/
#define CONFIG_LOW_CONSOLE_BAUD			0

/* Enable ir remote wake up for bl30 */
#define AML_IR_REMOTE_POWER_UP_KEY_VAL1 0xef10fe01
#define AML_IR_REMOTE_POWER_UP_KEY_VAL2 0XBB44FB04
#define AML_IR_REMOTE_POWER_UP_KEY_VAL3 0xF20DFE01
#define AML_IR_REMOTE_POWER_UP_KEY_VAL4 0XBA45BD02
#define AML_IR_REMOTE_POWER_UP_KEY_VAL5 0xe51afb04
#define AML_IR_REMOTE_POWER_UP_KEY_VAL6 0xFFFFFFFF
#define AML_IR_REMOTE_POWER_UP_KEY_VAL7 0xFFFFFFFF
#define AML_IR_REMOTE_POWER_UP_KEY_VAL8 0xFFFFFFFF
#define AML_IR_REMOTE_POWER_UP_KEY_VAL9 0xFFFFFFFF

/*config the default parameters for adc power key*/
#define AML_ADC_POWER_KEY_CHAN   2  /*channel range: 0-7*/
#define AML_ADC_POWER_KEY_VAL    0  /*sample value range: 0-1023*/

/* Bootloader Control Block function */
#ifndef CONFIG_PXP_DDR
#define CONFIG_BOOTLOADER_CONTROL_BLOCK
#endif

/* args/envs */
#define CONFIG_SYS_MAXARGS  64

#define CONFIG_EXTRA_ENV_SETTINGS \
	"scramble_reg=0xfe02e030\0"\
	"uart_base=0xfe07a000\0"\
	"usb_burning=" CONFIG_USB_TOOL_ENTRY "\0"\
	"dtb_mem_addr=0x01000000\0"\
	"boot_part=boot\0"\
	"Irq_check_en=0\0"\
	"cc_enable=1\0"\

#define CONFIG_PREBOOT  "echo JetHub J300 boot"

#define CONFIG_FIT 1
#define CONFIG_OF_LIBFDT 1

/* running in sram */
#define CONFIG_SYS_INIT_SP_ADDR				(0x00200000)

/* ddr functions */
#define DDR_FULL_TEST            0
#define DDR_LOW_POWER            0
#define DDR_ZQ_PD                0
#define DDR_USE_EXT_VREF         0
#define DDR4_TIMING_TEST         0
#define DDR_PLL_BYPASS           0

/* storage: emmc/nand/sd */
#define CONFIG_ENV_OVERWRITE

#if (defined(CONFIG_ENV_IS_IN_AMLNAND) || defined(CONFIG_ENV_IS_IN_MMC)) && \
defined(CONFIG_STORE_COMPATIBLE)
#error env in amlnand/mmc already be compatible;
#endif

#if defined(CONFIG_AML_NAND) && defined(CONFIG_MESON_NFC)
#error CONFIG_AML_NAND/CONFIG_MESON_NFC can not support at the sametime;
#endif

#if (defined(CONFIG_AML_NAND) || defined(CONFIG_MESON_NFC)) && defined(CONFIG_MESON_FBOOT)
#error CONFIG_AML_NAND/CONFIG_MESON_NFC CONFIG _MESON_FBOOT can not support at the sametime;
#endif

#if defined(CONFIG_SPI_NAND) && defined(CONFIG_MTD_SPI_NAND) && defined(CONFIG_MESON_NFC)
#error CONFIG_SPI_NAND/CONFIG_MTD_SPI_NAND/CONFIG_MESON_NFC can not support at the sametime;
#endif

/* mtd device board config */
#define BOARD_BOOT_LAYOUT_DISCRETE_BL2         1
#define CONFIG_BL2_COPY_NUM                    8
#define CONFIG_NAND_TPL_COPY_NUM           2
#define CONFIG_NOR_TPL_COPY_NUM                1
#define CONFIG_TPL_SIZE_PER_COPY           0x380000
#define CONFIG_BOOTLOADER_SIZE			0x380000

#define BOOTLOADER_MODE_NAND            ADVANCE_BOOTLOADER
#define BOOTLOADER_MODE_SNAND           ADVANCE_BOOTLOADER
#define BOOTLOADER_MODE_SNOR            COMPACT_BOOTLOADER
#define BOOTLOADER_MODE_ADVANCE_INIT        1
#define BOOTLOADER_DDR_FIP_SIZE             0x40000

/* mtd device rsv board config */
#ifndef CONFIG_ENV_IS_IN_NAND
#define MTD_RSV_ENV_BLOCK_CNT			4
#else
#define MTD_RSV_ENV_BLOCK_CNT			0
#endif

#define MTD_RSV_KEY_BLOCK_CNT			8

#ifndef CONFIG_DTB_BIND_KERNEL
#define MTD_RSV_DTB_BLOCK_CNT			4
#else
#define MTD_RSV_DTB_BLOCK_CNT			0
#endif

#define MTD_RSV_DDR_BLOCK_CNT			0

#define MTD_RSV_KEY_SIZE			0x8000
#define MTD_RSV_DTB_SIZE			0x10000
#define MTD_RSV_DDR_SIZE			0x10000
#define MTD_RSV_BLOCK_CNT	\
	(MTD_RSV_ENV_BLOCK_CNT + \
	MTD_RSV_KEY_BLOCK_CNT +	\
	MTD_RSV_DTB_BLOCK_CNT +	\
	MTD_RSV_DDR_BLOCK_CNT + 4)

#define BOARD_CONFIG_BL2_LAYOUT_TYPE		1

#ifdef CONFIG_AML_SD_EMMC
	#define		CONFIG_GENERIC_MMC 1
	#define		CONFIG_CMD_MMC 1
	#define CONFIG_CMD_GPT 1
	#define	CONFIG_SYS_MMC_ENV_DEV 1
	#define CONFIG_EMMC_DDR52_EN 0
	#define CONFIG_EMMC_DDR52_CLK 35000000
#endif
#define		CONFIG_PARTITIONS 1

#if defined CONFIG_MESON_NFC || defined CONFIG_SPI_NAND || defined CONFIG_MTD_SPI_NAND
	#define CONFIG_SYS_MAX_NAND_DEVICE  2
#endif

/* USB */
#define CONFIG_CMD_CC 1
#define CONFIG_CMD_BC 1

#define USB_PHY2_PLL_PARAMETER_1	0x09400414
#define USB_PHY2_PLL_PARAMETER_2	0x927e0000
#define USB_PHY2_PLL_PARAMETER_3	0xAC5F49E5

#define USB_G12x_PHY_PLL_SETTING_1	(0xfe18)
#define USB_G12x_PHY_PLL_SETTING_2	(0xfff)
#define USB_G12x_PHY_PLL_SETTING_3	(0x78000)
#define USB_G12x_PHY_PLL_SETTING_4	(0xe0004)
#define USB_G12x_PHY_PLL_SETTING_5	(0xe000c)

#define AML_TXLX_USB        1
#define AML_USB_V2             1
#define USB_GENERAL_BIT         3
#define USB_PHY21_BIT           4

/* net */
#define CONFIG_ETH_DESIGNWARE
#if defined(CONFIG_CMD_NET)
	#define CONFIG_PHYLIB	1
	#define CONFIG_CMD_PING 1
	#define CONFIG_CMD_DHCP 1
	#define CONFIG_CMD_RARP 1
	#define CONFIG_HOSTNAME        "jethub_j300"
	#define CONFIG_IPADDR          10.18.9.97
	#define CONFIG_GATEWAYIP       10.18.9.1
	#define CONFIG_SERVERIP        10.18.9.113
	#define CONFIG_NETMASK         255.255.255.0
#endif

#define MAC_ADDR_NEW  1

/* other devices */
#define CONFIG_SHA1 1
#define CONFIG_MD5 1

/*file system*/
#define CONFIG_DOS_PARTITION 1
#define CONFIG_EFI_PARTITION 1
#define CONFIG_FS_FAT 1
#define CONFIG_FS_EXT4 1
#define CONFIG_LZO 1

#define CONFIG_FAT_WRITE 1

/* define CONFIG_SYS_MEM_TOP_HIDE 8M space for free buffer */
#define CONFIG_SYS_MEM_TOP_HIDE		0x00800000

#define CONFIG_MULTI_DTB    1

/* support secure boot */
#define CONFIG_AML_SECURE_UBOOT   1

#if defined(CONFIG_AML_SECURE_UBOOT)
/* unify build for generate encrypted bootloader "u-boot.bin.encrypt" */
#define CONFIG_AML_CRYPTO_UBOOT   1
#endif

#define CONFIG_FIP_IMG_SUPPORT  1

#define BL32_SHARE_MEM_SIZE  0x800000

#define CONFIG_AVB2_UBOOT_SHA256

#endif /* __JETHUB_J300_CFG_H__ */
