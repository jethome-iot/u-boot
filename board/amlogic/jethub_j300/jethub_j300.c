#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <errno.h>
#include <env.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <amlogic/cpu_id.h>
#include <asm/amlogic/arch/secure_apb.h>
#include <asm/amlogic/arch/pinctrl_init.h>
#include <linux/sizes.h>
#include <asm-generic/gpio.h>
#include <dm.h>
#include <amlogic/aml_v3_burning.h>
#include <amlogic/aml_v2_burning.h>
#include <linux/mtd/partitions.h>
#include <asm/amlogic/arch/bl31_apis.h>
#include <amlogic/aml_mtd.h>
#include <amlogic/board.h>
#include <asm-generic/u-boot.h>
#include <command.h>
#include <asm/amlogic/arch/usb.h>
#include <asm/amlogic/arch/stick_mem.h>

#ifdef CONFIG_AML_VPU
#include <amlogic/media/vpu/vpu.h>
#endif
#ifdef CONFIG_AML_VPP
#include <amlogic/media/vpp/vpp.h>
#endif
#ifdef CONFIG_AML_HDMITX21
#include <amlogic/media/vout/hdmitx21/hdmitx_module.h>
#endif
#ifdef CONFIG_AML_CVBS
#include <amlogic/media/vout/aml_cvbs.h>
#endif
#ifdef CONFIG_AMLOGIC_AMFC
#include <amlogic/amfc.h>
#endif
#include <amlogic/aml_profile.h>

DECLARE_GLOBAL_DATA_PTR;
extern int cc_statue, bc_status;

void sys_led_init(void)
{
}

int serial_set_pin_port(unsigned long port_base)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = (readl(SYSCTRL_SEC_STATUS_REG4) & ~0xffffUL) << 4;
	return 0;
}

void secondary_boot_func(void)
{
}

int board_eth_init(bd_t *bis)
{
	return 0;
}

int active_clk(void)
{
	struct udevice *clk = NULL;
	int err;

	err = uclass_get_device_by_name(UCLASS_CLK, "xtal-clk", &clk);
	if (err) {
		pr_err("Can't find xtal-clk clock (%d)\n", err);
		return err;
	}
	err = uclass_get_device_by_name(UCLASS_CLK, "clock-controller@0", &clk);
	if (err) {
		pr_err("Can't find clock-controller@0 clock (%d)\n", err);
		return err;
	}

	return 0;
}

void board_init_mem(void)
{
	phys_size_t ram_size;
	char *env_tmp;

	env_tmp = env_get("bootm_size");
	if (!env_tmp) {
		ram_size =
		    ((readl(SYSCTRL_SEC_STATUS_REG4) & 0xFFF00000) << 4) >
		    0xe0000000 ? 0xe0000000 : ((readl(SYSCTRL_SEC_STATUS_REG4) & 0xFFF00000) << 4);
		env_set_hex("bootm_low", 0);
		env_set_hex("bootm_size", ram_size);
	}
}

void set_usb_status(void)
{
	print_aml_cc_ufp_current_type();
	switch (cc_statue) {
	case 0:
		printf("usb power supply is detach\n");
		break;
	case 1:
		printf("usb power supply is 0.5a@5v\n");
		env_set("usb_status", "0.5a@5v");
		break;
	case 2:
		printf("usb power supply is 1.5a@5v\n");
		env_set("usb_status", "1.5a@5v");
		break;
	case 3:
		printf("usb power supply is 3a@5v\n");
		env_set("usb_status", "3a@5v");
		break;
	case 4:
		printf("CC_STATUS_TYPEA_SDP\n");
		env_set("usb_status", "0.5a@5v");
		break;
	case 5:
		printf("CC_STATUS_TYPEA_DCP\n");
		env_set("usb_status", "1.5a@5v");
		break;
	case 6:
		printf("CC_STATUS_TYPEA_CDP\n");
		env_set("usb_status", "3a@5v");
		break;
	default:
		printf("error status %s:%d\n", __func__, __LINE__);
		break;
	}
}

extern void set_usb_power_off(void);
int board_init(void)
{
	printf("board init\n");

#if !defined(CONFIG_PXP_DDR)
	aml_set_bootsequence(0);
#if defined(CONFIG_AML_V3_FACTORY_BURN) && defined(CONFIG_AML_V3_USB_TOOl)
	if ((readl(SYSCTRL_SEC_STICKY_REG2) != 0x1b8ec003) &&
	    (readl(SYSCTRL_SEC_STICKY_REG2) != 0x1b8ec004)) {
		aml_v3_factory_usb_burning(0, gd->bd);
	}
#endif
#endif
	pinctrl_devices_active(PIN_CONTROLLER_NUM);
#ifdef CONFIG_AMLOGIC_AMFC
	amfc_init();
#endif
	set_usb_power_off();
	return 0;
}

int board_late_init(void)
{
	printf("board late init\n");
	env_set("defenv_para", "-c");
	if (env_get_ulong("default_env", 10, 0))
		env_set("defenv_para", "-c0");
	aml_board_late_init_front(NULL);
#ifdef CONFIG_PXP_EMULATOR
	return 0;
#endif
	get_stick_reboot_flag_mbx();

	PUSH_TIME_TE("vpu vpp init", BL33_VPUVPP_INIT_s);
#ifdef CONFIG_AML_VPU
	vpu_probe();
#endif
#ifdef CONFIG_AML_VPP
	vpp_init();
#endif
#ifdef CONFIG_AML_CVBS
	cvbs_init();
#endif
	PUSH_TIME_TE("vpu vpp init", BL33_VPUVPP_INIT_e);
	set_usb_status();
	aml_board_late_init_tail(NULL);
	return 0;
}

phys_size_t get_effective_memsize(void)
{
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	return ((readl(SYSCTRL_SEC_STATUS_REG4) & 0xFFF00000) << 4) > 0xe0000000 ? 0xe0000000 :
	    (((readl(SYSCTRL_SEC_STATUS_REG4) & 0xFFF00000) << 4) - CONFIG_SYS_MEM_TOP_HIDE);
#else
	return ((readl(SYSCTRL_SEC_STATUS_REG4) & 0xFFF00000) << 4) > 0xe0000000 ? 0xe0000000 :
	    ((readl(SYSCTRL_SEC_STATUS_REG4) & 0xFFF00000) << 4);
#endif
}

int mach_cpu_init(void)
{
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}

#ifdef CONFIG_SPI_FLASH
static const struct mtd_partition spiflash_partitions[] = {
	{
	 .name = "env",
	 .offset = 0,
	 .size = 1 * SZ_256K,
	},
	{
	 .name = "dtb",
	 .offset = 0,
	 .size = 1 * SZ_256K,
	},
	{
	 .name = "boot",
	 .offset = 0,
	 .size = 2 * SZ_1M,
	},
	{
	 .name = "user",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTDPART_SIZ_FULL,
	}
};

const struct mtd_partition *get_spiflash_partition_table(int *partitions)
{
	*partitions = ARRAY_SIZE(spiflash_partitions);
	return spiflash_partitions;
}
#endif

struct rsv_part rsv_partitions[] = {
#ifndef CONFIG_ENV_IS_IN_NAND
	{ ENV_NAND_MAGIC, 0, MTD_RSV_ENV_BLOCK_CNT, CONFIG_ENV_SIZE },
#endif
	{ KEY_NAND_MAGIC, 0, MTD_RSV_KEY_BLOCK_CNT, MTD_RSV_KEY_SIZE },
#ifndef CONFIG_DTB_BIND_KERNEL
	{ DTB_NAND_MAGIC, 0, MTD_RSV_DTB_BLOCK_CNT, MTD_RSV_DTB_SIZE },
#endif
};

struct rsv_part *get_mtd_rsv_partition(void)
{
	return rsv_partitions;
}

int get_mtd_rsv_partition_count(void)
{
	return ARRAY_SIZE(rsv_partitions) + 1;
}

#ifdef CONFIG_MULTI_DTB
int checkhw(char *name)
{
	char dtb_name[64] = { 0 };
	cpu_id_t cpu_id = get_cpu_id();

	if (cpu_id.family_id == 0x47)
		strcpy(dtb_name, "s7d_jethub_j300");

	strcpy(name, dtb_name);
	env_set("aml_dt", dtb_name);
	return 0;
}
#endif

int __attribute__((weak)) mmc_initialize(bd_t *bis)
{
	return 0;
}

void __attribute__((weak)) set_working_fdt_addr(ulong addr)
{
}

void __attribute__((weak)) md5_wd(unsigned char *input, int len,
				  unsigned char output[16],
				  unsigned int chunk_sz)
{
}
