/*
 * drivers/amlogic/media/vout/lcd/lcd_tablet/lcd_drv.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <amlogic/media/vpp/vpp.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "../lcd_reg.h"
#include "../lcd_common.h"
#include "lcd_tablet.h"
#include "mipi_dsi_util.h"

static int lcd_type_supported(struct lcd_config_s *pconf)
{
	int lcd_type = pconf->basic.lcd_type;
	int ret = -1;

	switch (lcd_type) {
	case LCD_TTL:
	case LCD_LVDS:
	case LCD_VBYONE:
	case LCD_MIPI:
	case LCD_EDP:
		ret = 0;
		break;
	default:
		LCDERR("invalid lcd type: %s(%d)\n",
			lcd_type_type_to_str(lcd_type), lcd_type);
		break;
	}
	return ret;
}

static void lcd_ttl_control_set(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	unsigned int clk_pol, rb_swap, bit_swap;

	clk_pol = pconf->control.ttl_cfg.clk_pol;
	rb_swap = (pconf->control.ttl_cfg.swap_ctrl >> 1) & 1;
	bit_swap = (pconf->control.ttl_cfg.swap_ctrl >> 0) & 1;

	lcd_vcbus_setb(L_POL_CNTL_ADDR, clk_pol, 6, 1);
	lcd_vcbus_setb(L_DUAL_PORT_CNTL_ADDR, rb_swap, 1, 1);
	lcd_vcbus_setb(L_DUAL_PORT_CNTL_ADDR, bit_swap, 0, 1);
}

static void lcd_mipi_control_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int bit_lane_sel;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	if (pdrv->data->chip_type == LCD_CHIP_T7) {
		switch (pdrv->index) {
		case 0:
			bit_lane_sel = 0;
			break;
		case 1:
			bit_lane_sel = 16;
			break;
		default:
			LCDERR("[%d]: %s: invalid drv_index\n",
			       pdrv->index, __func__);
			return;
		}
		// sel dphy lane
		lcd_combo_dphy_setb(COMBO_DPHY_CNTL1, 0x0, bit_lane_sel, 10);
	}

	mipi_dsi_tx_ctrl(pdrv, 1);
}

static void lcd_mipi_disable(struct aml_lcd_drv_s *pdrv)
{
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	mipi_dsi_tx_ctrl(pdrv, 0);
}

static void lcd_edp_control_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int reg_dphy_tx_ctrl0, reg_dphy_tx_ctrl1;
	unsigned int bit_data_in_lvds, bit_data_in_edp, bit_lane_sel;

	switch (pdrv->index) {
	case 0:
		reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL0;
		reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL1;
		bit_data_in_lvds = 0;
		bit_data_in_edp = 1;
		bit_lane_sel = 0;
		break;
	case 1:
		reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL0;
		reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL1;
		bit_data_in_lvds = 2;
		bit_data_in_edp = 3;
		bit_lane_sel = 16;
		break;
	default:
		LCDERR("[%d]: %s: invalid drv_index\n", pdrv->index, __func__);
		return;
	}

	// sel dphy data_in
	lcd_combo_dphy_setb(COMBO_DPHY_CNTL0, 0, bit_data_in_lvds, 1);
	lcd_combo_dphy_setb(COMBO_DPHY_CNTL0, 1, bit_data_in_edp, 1);
	// sel dphy lane
	lcd_combo_dphy_setb(COMBO_DPHY_CNTL1, 0x155, bit_lane_sel, 10);

	// sel edp fifo clkdiv 20, enable lane
	lcd_combo_dphy_write(reg_dphy_tx_ctrl0, ((0x4 << 5) | (0x1f << 16)));

	// fifo enable
	lcd_combo_dphy_setb(reg_dphy_tx_ctrl1, 1, 6, 10);
	// fifo wr enable
	lcd_combo_dphy_setb(reg_dphy_tx_ctrl1, 1, 7, 10);

	edp_tx_ctrl(pdrv, 1);
}

static void lcd_edp_disable(struct aml_lcd_drv_s *pdrv)
{
	unsigned int reg_dphy_tx_ctrl0, reg_dphy_tx_ctrl1;

	switch (pdrv->index) {
	case 0:
		reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL0;
		reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL1;
		break;
	case 1:
		reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL0;
		reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL1;
		break;
	default:
		LCDERR("[%d]: %s: invalid drv_index\n", pdrv->index, __func__);
		return;
	}

	edp_tx_ctrl(pdrv, 0);

	// fifo wr disable
	lcd_combo_dphy_setb(reg_dphy_tx_ctrl1, 0, 7, 10);
	// fifo disable
	lcd_combo_dphy_setb(reg_dphy_tx_ctrl1, 0, 6, 10);
	// lane disable
	lcd_combo_dphy_setb(reg_dphy_tx_ctrl0, 0, 16, 8);
}

static void lcd_lvds_clk_util_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int reg_phy_tx_ctrl0, reg_phy_tx_ctrl1;
	unsigned int bit_data_in_lvds, bit_data_in_edp, bit_lane_sel;
	unsigned int phy_div, val_lane_sel, len_lane_sel;

	if (pdrv->config.control.lvds_cfg.dual_port)
		phy_div = 2;
	else
		phy_div = 1;

	if (pdrv->data->chip_type == LCD_CHIP_T7) {
		switch (pdrv->index) {
		case 0:
			reg_phy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL0;
			reg_phy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL1;
			bit_data_in_lvds = 0;
			bit_data_in_edp = 1;
			bit_lane_sel = 0;
			val_lane_sel = 0x155;
			len_lane_sel = 10;
			break;
		case 1:
			reg_phy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL0;
			reg_phy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL1;
			bit_data_in_lvds = 2;
			bit_data_in_edp = 3;
			bit_lane_sel = 10;
			val_lane_sel = 0x155;
			len_lane_sel = 10;
			break;
		case 2:
			reg_phy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY2_CNTL0;
			reg_phy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY2_CNTL1;
			bit_data_in_lvds = 4;
			bit_data_in_edp = 0xff;
			if (pdrv->config.control.lvds_cfg.dual_port) {
				bit_lane_sel = 10;
				val_lane_sel = 0xaaaaa;
				len_lane_sel = 20;
			} else {
				bit_lane_sel = 20;
				val_lane_sel = 0x2aa;
				len_lane_sel = 10;
			}
			break;
		default:
			LCDERR("[%d]: %s: invalid drv_index\n",
			       pdrv->index, __func__);
			return;
		}

		// sel dphy data_in
		if (bit_data_in_edp < 0xff)
			lcd_combo_dphy_setb(COMBO_DPHY_CNTL0, 0, bit_data_in_edp, 1);
		lcd_combo_dphy_setb(COMBO_DPHY_CNTL0, 1, bit_data_in_lvds, 1);
		// sel dphy lane
		lcd_combo_dphy_setb(COMBO_DPHY_CNTL1, val_lane_sel,
				    bit_lane_sel, len_lane_sel);

		/* set fifo_clk_sel: div 7 */
		lcd_combo_dphy_write(reg_phy_tx_ctrl0, (1 << 5));
		/* set cntl_ser_en:  8-channel */
		lcd_combo_dphy_setb(reg_phy_tx_ctrl0, 0x3ff, 16, 10);

		/* decoupling fifo enable, gated clock enable */
		lcd_combo_dphy_write(reg_phy_tx_ctrl1, (1 << 6) | (1 << 0));
		/* decoupling fifo write enable after fifo enable */
		lcd_combo_dphy_setb(reg_phy_tx_ctrl1, 1, 7, 1);
	} else {
		/* set fifo_clk_sel: div 7 */
		lcd_ana_write(HHI_LVDS_TX_PHY_CNTL0, (1 << 6));
		/* set cntl_ser_en:  8-channel to 1 */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL0, 0xfff, 16, 12);
		/* pn swap */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL0, 1, 2, 1);

		/* decoupling fifo enable, gated clock enable */
		lcd_ana_write(HHI_LVDS_TX_PHY_CNTL1,
			      (1 << 30) | ((phy_div - 1) << 25) | (1 << 24));
		/* decoupling fifo write enable after fifo enable */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL1, 1, 31, 1);
	}
}

static void lcd_lvds_control_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int bit_num, pn_swap, port_swap, lane_reverse;
	unsigned int dual_port, fifo_mode, lvds_repack;
	unsigned int offset;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	lcd_lvds_clk_util_set(pdrv);

	offset = pdrv->data->offset_venc_if[pdrv->index];
	lvds_repack = (pdrv->config.control.lvds_cfg.lvds_repack) & 0x3;
	pn_swap   = (pdrv->config.control.lvds_cfg.pn_swap) & 0x1;
	dual_port = (pdrv->config.control.lvds_cfg.dual_port) & 0x1;
	port_swap = (pdrv->config.control.lvds_cfg.port_swap) & 0x1;
	lane_reverse = (pdrv->config.control.lvds_cfg.lane_reverse) & 0x1;

	switch (pdrv->config.basic.lcd_bits) {
	case 10:
		bit_num = 0;
		break;
	case 6:
		bit_num = 2;
		break;
	case 8:
	default:
		bit_num = 1;
		break;
	}
	if (dual_port)
		fifo_mode = 0x3;
	else
		fifo_mode = 0x1;

	if (pdrv->data->chip_type == LCD_CHIP_T7)
		lcd_vcbus_write(LVDS_SER_EN + offset, 0xfff);

	lcd_vcbus_write(LVDS_PACK_CNTL_ADDR + offset,
			(lvds_repack << 0) | // repack //[1:0]
			(0 << 3) |		// reserve
			(0 << 4) |		// lsb first
			(pn_swap << 5) |	// pn swap
			(dual_port << 6) |	// dual port
			(0 << 7) |		// use tcon control
			(bit_num << 8) |	// 0:10bits, 1:8bits, 2:6bits, 3:4bits.
			(0 << 10) |		//r_select  //0:R, 1:G, 2:B, 3:0
			(1 << 12) |		//g_select  //0:R, 1:G, 2:B, 3:0
			(2 << 14));		//b_select  //0:R, 1:G, 2:B, 3:0;

	/* lvsd swap */
	switch (pdrv->data->chip_type) {
	case LCD_CHIP_TL1:
	case LCD_CHIP_TM2:
		/* lvds channel:    //tx 12 channels
		 *    0: d0_a
		 *    1: d1_a
		 *    2: d2_a
		 *    3: clk_a
		 *    4: d3_a
		 *    5: d4_a
		 *    6: d0_b
		 *    7: d1_b
		 *    8: d2_b
		 *    9: clk_b
		 *    a: d3_b
		 *    b: d4_b
		 */
		if (port_swap) {
			if (lane_reverse) {
				lcd_vcbus_write(P2P_CH_SWAP0, 0x456789ab);
				lcd_vcbus_write(P2P_CH_SWAP1, 0x0123);
			} else {
				lcd_vcbus_write(P2P_CH_SWAP0, 0x10ba9876);
				lcd_vcbus_write(P2P_CH_SWAP1, 0x5432);
			}
		} else {
			if (lane_reverse) {
				lcd_vcbus_write(P2P_CH_SWAP0, 0xab012345);
				lcd_vcbus_write(P2P_CH_SWAP1, 0x6789);
			} else {
				lcd_vcbus_write(P2P_CH_SWAP0, 0x76543210);
				lcd_vcbus_write(P2P_CH_SWAP1, 0xba98);
			}
		}
		break;
	case LCD_CHIP_T5:
	case LCD_CHIP_T5D:
		/* lvds channel:    //tx 12 channels
		 *    0: d0_a
		 *    1: d1_a
		 *    2: d2_a
		 *    3: clk_a
		 *    4: d3_a
		 *    5: d4_a
		 *    6: d0_b
		 *    7: d1_b
		 *    8: d2_b
		 *    9: clk_b
		 *    a: d3_b
		 *    b: d4_b
		 */
		if (port_swap) {
			if (lane_reverse) {
				lcd_vcbus_write(P2P_CH_SWAP0, 0x345789ab);
				lcd_vcbus_write(P2P_CH_SWAP1, 0x0612);
			} else {
				lcd_vcbus_write(P2P_CH_SWAP0, 0x210a9876);
				lcd_vcbus_write(P2P_CH_SWAP1, 0x5b43);
			}
		} else {
			if (lane_reverse) {
				lcd_vcbus_write(P2P_CH_SWAP0, 0xab12345);
				lcd_vcbus_write(P2P_CH_SWAP1, 0x60789);
			} else {
				lcd_vcbus_write(P2P_CH_SWAP0, 0x87643210);
				lcd_vcbus_write(P2P_CH_SWAP1, 0xb5a9);
			}
		}
		break;
	case LCD_CHIP_T7:
		/* lvds channel:    //tx 12 channels
		 *    0: d0_a
		 *    1: d1_a
		 *    2: d2_a
		 *    3: clk_a
		 *    4: d3_a
		 *    5: d4_a
		 *    6: d0_b
		 *    7: d1_b
		 *    8: d2_b
		 *    9: clk_b
		 *    a: d3_b
		 *    b: d4_b
		 */
		if (port_swap) {
			if (lane_reverse) {
				lcd_vcbus_write(P2P_CH_SWAP0 + offset, 0x345789ab);
				lcd_vcbus_write(P2P_CH_SWAP1 + offset, 0x0612);
			} else {
				lcd_vcbus_write(P2P_CH_SWAP0 + offset, 0x210a9876);
				lcd_vcbus_write(P2P_CH_SWAP1 + offset, 0x5b43);
			}
		} else {
			if (lane_reverse) {
				lcd_vcbus_write(P2P_CH_SWAP0 + offset, 0xab12345);
				lcd_vcbus_write(P2P_CH_SWAP1 + offset, 0x60789);
			} else {
				lcd_vcbus_write(P2P_CH_SWAP0 + offset, 0x87643210);
				lcd_vcbus_write(P2P_CH_SWAP1 + offset, 0xb5a9);
			}
		}
		lcd_vcbus_write(P2P_BIT_REV + offset, 2);
		break;
	default:
		break;
	}

	lcd_vcbus_write(LVDS_GEN_CNTL + offset,
			(lcd_vcbus_read(LVDS_GEN_CNTL + offset) | (1 << 4) | (fifo_mode << 0)));
	lcd_vcbus_setb(LVDS_GEN_CNTL + offset, 1, 3, 1);
}

static void lcd_lvds_disable(struct aml_lcd_drv_s *pdrv)
{
	unsigned int reg_dphy_tx_ctrl0, reg_dphy_tx_ctrl1, offset = 0;

	if (pdrv->data->chip_type == LCD_CHIP_T7) {
		switch (pdrv->index) {
		case 0:
			reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL0;
			reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL1;
			break;
		case 1:
			reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL0;
			reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL1;
			break;
		case 2:
			reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY2_CNTL0;
			reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY2_CNTL1;
			break;
		default:
			LCDERR("[%d]: %s: invalid drv_index\n", pdrv->index, __func__);
			return;
		}
		offset = pdrv->data->offset_venc_if[pdrv->index];

		/* disable lvds fifo */
		lcd_vcbus_setb(LVDS_GEN_CNTL + offset, 0, 3, 1);
		/* disable fifo */
		lcd_combo_dphy_setb(reg_dphy_tx_ctrl1, 0, 6, 2);
		/* disable lane */
		lcd_combo_dphy_setb(reg_dphy_tx_ctrl0, 0, 16, 10);
	} else {
		/* disable lvds fifo */
		lcd_vcbus_setb(LVDS_GEN_CNTL, 0, 3, 1);
		/* disable fifo */
		lcd_clk_setb(HHI_LVDS_TX_PHY_CNTL1, 0, 30, 2);
		/* disable lane */
		lcd_clk_setb(HHI_LVDS_TX_PHY_CNTL0, 0, 16, 12);
	}
}

static void lcd_vbyone_sync_pol(unsigned int offset, int hsync_pol, int vsync_pol)
{
	lcd_vcbus_setb(VBO_VIN_CTRL + offset, hsync_pol, 4, 1);
	lcd_vcbus_setb(VBO_VIN_CTRL + offset, vsync_pol, 5, 1);

	lcd_vcbus_setb(VBO_VIN_CTRL + offset, hsync_pol, 6, 1);
	lcd_vcbus_setb(VBO_VIN_CTRL + offset, vsync_pol, 7, 1);
}

static void lcd_vbyone_clk_util_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int lcd_bits, div_sel, phy_div;
	unsigned int reg_phy_tx_ctrl0, reg_phy_tx_ctrl1;
	unsigned int bit_data_in_lvds, bit_data_in_edp, bit_lane_sel;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	phy_div = pdrv->config.control.vbyone_cfg.phy_div;
	lcd_bits = pdrv->config.basic.lcd_bits;
	switch (lcd_bits) {
	case 6:
		div_sel = 0;
		break;
	case 8:
		div_sel = 2;
		break;
	case 10:
	default:
		div_sel = 3;
		break;
	}

	if (pdrv->data->chip_type == LCD_CHIP_T7) {
		switch (pdrv->index) {
		case 0:
			reg_phy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL0;
			reg_phy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL1;
			bit_data_in_lvds = 0;
			bit_data_in_edp = 1;
			bit_lane_sel = 0;
			break;
		case 1:
			reg_phy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL0;
			reg_phy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL1;
			bit_data_in_lvds = 2;
			bit_data_in_edp = 3;
			bit_lane_sel = 16;
			break;
		default:
			LCDERR("[%d]: %s: invalid drv_index\n",
			       pdrv->index, __func__);
			return;
		}

		// sel dphy data_in
		lcd_combo_dphy_setb(COMBO_DPHY_CNTL0, 0, bit_data_in_edp, 1);
		lcd_combo_dphy_setb(COMBO_DPHY_CNTL0, 1, bit_data_in_lvds, 1);
		// sel dphy lane
		lcd_combo_dphy_setb(COMBO_DPHY_CNTL1, 0x5555, bit_lane_sel, 16);

		/* set fifo_clk_sel: div 7 */
		lcd_combo_dphy_write(reg_phy_tx_ctrl0, (div_sel << 5));
		/* set cntl_ser_en:  8-channel to 1 */
		lcd_combo_dphy_setb(reg_phy_tx_ctrl0, 0xff, 16, 8);

		/* decoupling fifo enable, gated clock enable */
		lcd_combo_dphy_write(reg_phy_tx_ctrl1, (1 << 6) | (1 << 0));
		/* decoupling fifo write enable after fifo enable */
		lcd_combo_dphy_setb(reg_phy_tx_ctrl1, 1, 7, 1);
	} else {
		/* set fifo_clk_sel: div 10 */
		lcd_ana_write(HHI_LVDS_TX_PHY_CNTL0, (div_sel << 6));
		/* set cntl_ser_en:  8-channel to 1 */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL0, 0xfff, 16, 12);
		/* pn swap */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL0, 1, 2, 1);

		/* decoupling fifo enable, gated clock enable */
		lcd_ana_write(HHI_LVDS_TX_PHY_CNTL1,
			      (1 << 30) | ((phy_div - 1) << 25) | (1 << 24));
		/* decoupling fifo write enable after fifo enable */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL1, 1, 31, 1);
	}
}

static int lcd_vbyone_lanes_set(unsigned int offset, int lane_num, int byte_mode,
				int region_num, int hsize, int vsize)
{
	int sublane_num;
	int region_size[4];
	int tmp;

	switch (lane_num) {
	case 1:
	case 2:
	case 4:
	case 8:
		break;
	default:
		return -1;
	}
	switch (region_num) {
	case 1:
	case 2:
	case 4:
		break;
	default:
		return -1;
	}
	if (lane_num % region_num)
		return -1;
	switch (byte_mode) {
	case 3:
	case 4:
		break;
	default:
		return -1;
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("byte_mode=%d, lane_num=%d, region_num=%d\n",
			byte_mode, lane_num, region_num);
	}

	sublane_num = lane_num / region_num; /* lane num in each region */
	lcd_vcbus_setb(VBO_LANES + offset, (lane_num - 1), 0, 3);
	lcd_vcbus_setb(VBO_LANES + offset, (region_num - 1), 4, 2);
	lcd_vcbus_setb(VBO_LANES + offset, (sublane_num - 1), 8, 3);
	lcd_vcbus_setb(VBO_LANES + offset, (byte_mode - 1), 11, 2);

	if (region_num > 1) {
		region_size[3] = (hsize / lane_num) * sublane_num;
		tmp = (hsize % lane_num);
		region_size[0] = region_size[3] + (((tmp / sublane_num) > 0) ?
			sublane_num : (tmp % sublane_num));
		region_size[1] = region_size[3] + (((tmp / sublane_num) > 1) ?
			sublane_num : (tmp % sublane_num));
		region_size[2] = region_size[3] + (((tmp / sublane_num) > 2) ?
			sublane_num : (tmp % sublane_num));
		lcd_vcbus_write(VBO_REGION_00 + offset, region_size[0]);
		lcd_vcbus_write(VBO_REGION_01 + offset, region_size[1]);
		lcd_vcbus_write(VBO_REGION_02 + offset, region_size[2]);
		lcd_vcbus_write(VBO_REGION_03 + offset, region_size[3]);
	}
	lcd_vcbus_write(VBO_ACT_VSIZE + offset, vsize);
	/* different from FBC code!!! */
	/* lcd_vcbus_setb(VBO_CTRL_H + offset,0x80,11,5); */
	/* different from simulation code!!! */
	lcd_vcbus_setb(VBO_CTRL_H + offset, 0x0, 0, 4);
	lcd_vcbus_setb(VBO_CTRL_H + offset, 0x1, 9, 1);
	/* lcd_vcbus_setb(VBO_CTRL_L + offset,enable,0,1); */

	return 0;
}

static void lcd_vbyone_control_set(struct aml_lcd_drv_s *pdrv)
{
	int lane_count, byte_mode, region_num, hsize, vsize;
	/* int color_fmt; */
	int vin_color, vin_bpp;
	unsigned int offset;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	offset = pdrv->data->offset_venc_if[pdrv->index];

	hsize = pdrv->config.basic.h_active;
	vsize = pdrv->config.basic.v_active;
	lane_count = pdrv->config.control.vbyone_cfg.lane_count; /* 8 */
	region_num = pdrv->config.control.vbyone_cfg.region_num; /* 2 */
	byte_mode = pdrv->config.control.vbyone_cfg.byte_mode; /* 4 */
	/* color_fmt = pdrv->config.control.vbyone_cfg.color_fmt; // 4 */

	vin_color = 4; /* fixed RGB */
	switch (pdrv->config.basic.lcd_bits) {
	case 6:
		vin_bpp = 2; /* 18bbp 4:4:4 */
		break;
	case 8:
		vin_bpp = 1; /* 24bbp 4:4:4 */
		break;
	case 10:
	default:
		vin_bpp = 0; /* 30bbp 4:4:4 */
		break;
	}

	lcd_vbyone_clk_util_set(pdrv);

	/* set Vbyone vin color format */
	lcd_vcbus_setb(VBO_VIN_CTRL + offset, vin_color, 8, 3);
	lcd_vcbus_setb(VBO_VIN_CTRL + offset, vin_bpp, 11, 2);

	lcd_vbyone_lanes_set(offset, lane_count, byte_mode, region_num, hsize, vsize);
	/*set hsync/vsync polarity to let the polarity is low active
	inside the VbyOne */
	lcd_vbyone_sync_pol(offset, 0, 0);

	/* below line copy from simulation */
	/* gate the input when vsync asserted */
	lcd_vcbus_setb(VBO_VIN_CTRL + offset, 1, 0, 2);
	/* lcd_vcbus_write(VBO_VBK_CTRL_0 + offset,0x13);
	//lcd_vcbus_write(VBO_VBK_CTRL_1 + offset,0x56);
	//lcd_vcbus_write(VBO_HBK_CTRL + offset,0x3478);
	//lcd_vcbus_setb(VBO_PXL_CTRL + offset,0x2,0,4);
	//lcd_vcbus_setb(VBO_PXL_CTRL + offset,0x3,VBO_PXL_CTR1_BIT,VBO_PXL_CTR1_WID);
	//set_vbyone_ctlbits(1,0,0); */
	/* VBO_RGN_GEN clk always on */
	lcd_vcbus_setb(VBO_GCLK_MAIN + offset, 2, 2, 2);

	/* PAD select: */
	if ((lane_count == 1) || (lane_count == 2))
		lcd_vcbus_setb(LCD_PORT_SWAP + offset, 1, 9, 2);
	else if (lane_count == 4)
		lcd_vcbus_setb(LCD_PORT_SWAP + offset, 2, 9, 2);
	else
		lcd_vcbus_setb(LCD_PORT_SWAP + offset, 0, 9, 2);
	/* lcd_vcbus_setb(LCD_PORT_SWAP + offset, 1, 8, 1);//reverse lane output order */

	lcd_vbyone_hw_filter(pdrv, 1);
	lcd_vcbus_setb(VBO_INSGN_CTRL + offset, 0, 2, 2);

	lcd_vcbus_setb(VBO_CTRL_L + offset, 1, 0, 1);

	lcd_vbyone_wait_timing_stable(pdrv);
	lcd_vbyone_sw_reset(pdrv);

	/* training hold */
	if ((pdrv->config.control.vbyone_cfg.ctrl_flag) & 0x4)
		lcd_vbyone_cdr_training_hold(pdrv, 1);
}

static void lcd_vbyone_disable(struct aml_lcd_drv_s *pdrv)
{
	unsigned int reg_dphy_tx_ctrl0, reg_dphy_tx_ctrl1, offset;

	if (pdrv->data->chip_type == LCD_CHIP_T7) {
		switch (pdrv->index) {
		case 0:
			reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL0;
			reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY0_CNTL1;
			break;
		case 1:
			reg_dphy_tx_ctrl0 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL0;
			reg_dphy_tx_ctrl1 = COMBO_DPHY_EDP_LVDS_TX_PHY1_CNTL1;
			break;
		default:
			LCDERR("[%d]: %s: invalid drv_index\n", pdrv->index, __func__);
			return;
		}
		offset = pdrv->data->offset_venc_if[pdrv->index];

		lcd_vcbus_setb(VBO_CTRL_L + offset, 0, 0, 1);
		/* clear insig setting */
		lcd_vcbus_setb(VBO_INSGN_CTRL + offset, 0, 2, 1);
		lcd_vcbus_setb(VBO_INSGN_CTRL + offset, 0, 0, 1);

		/* disable fifo */
		lcd_combo_dphy_setb(reg_dphy_tx_ctrl1, 0, 6, 2);
		/* disable lane */
		lcd_combo_dphy_setb(reg_dphy_tx_ctrl0, 0, 16, 8);
	} else {
		lcd_vcbus_setb(VBO_CTRL_L, 0, 0, 1);
		/* clear insig setting */
		lcd_vcbus_setb(VBO_INSGN_CTRL, 0, 2, 1);
		lcd_vcbus_setb(VBO_INSGN_CTRL, 0, 0, 1);

		/* disable fifo */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL1, 0, 30, 2);
		/* disable lane */
		lcd_ana_setb(HHI_LVDS_TX_PHY_CNTL0, 0, 16, 12);
	}
}

void lcd_tablet_config_update(struct aml_lcd_drv_s *pdrv)
{
	/* update interface timing */
	switch (pdrv->config.basic.lcd_type) {
	case LCD_VBYONE:
		lcd_vbyone_config_set(pdrv);
		break;
	case LCD_MIPI:
		lcd_mipi_dsi_config_set(pdrv);
		break;
	case LCD_EDP:
		lcd_edp_config_set(pdrv);
		break;
	default:
		break;
	}
}

void lcd_tablet_driver_init_pre(struct aml_lcd_drv_s *pdrv)
{
	int ret;

	LCDPR("[%d]: tablet driver init(ver %s): %s\n",
		pdrv->index,
		LCD_DRV_VERSION,
		lcd_type_type_to_str(pdrv->config.basic.lcd_type));
	ret = lcd_type_supported(&pdrv->config);
	if (ret)
		return;

	lcd_set_clk(pdrv);
	lcd_set_venc(pdrv);
}

int lcd_tablet_driver_init(struct aml_lcd_drv_s *pdrv)
{
	int ret;

	ret = lcd_type_supported(&pdrv->config);
	if (ret)
		return -1;

	/* init driver */
	switch (pdrv->config.basic.lcd_type) {
	case LCD_TTL:
		lcd_ttl_control_set(pdrv);
		lcd_pinmux_set(pdrv, 1);
		break;
	case LCD_LVDS:
		lcd_lvds_control_set(pdrv);
		lcd_phy_set(pdrv, 1);
		break;
	case LCD_VBYONE:
		lcd_pinmux_set(pdrv, 1);
		lcd_vbyone_control_set(pdrv);
		lcd_vbyone_wait_hpd(pdrv);
		lcd_phy_set(pdrv, 1);
		lcd_vbyone_wait_stable(pdrv);
		break;
	case LCD_MIPI:
		lcd_phy_set(pdrv, 1);
		lcd_mipi_control_set(pdrv);
		break;
	case LCD_EDP:
		lcd_pinmux_set(pdrv, 1);
		lcd_phy_set(pdrv, 1);
		lcd_edp_control_set(pdrv);
		break;
	default:
		break;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);
	return 0;
}

void lcd_tablet_driver_disable(struct aml_lcd_drv_s *pdrv)
{
	int ret;

	LCDPR("[%d]: disable driver\n", pdrv->index);
	ret = lcd_type_supported(&pdrv->config);
	if (ret)
		return;

	switch (pdrv->config.basic.lcd_type) {
	case LCD_TTL:
		lcd_pinmux_set(pdrv, 0);
		break;
	case LCD_LVDS:
		lcd_phy_set(pdrv, 0);
		lcd_lvds_disable(pdrv);
		break;
	case LCD_VBYONE:
		lcd_phy_set(pdrv, 0);
		lcd_pinmux_set(pdrv, 0);
		lcd_vbyone_disable(pdrv);
		break;
	case LCD_MIPI:
		mipi_dsi_link_off(pdrv);
		lcd_phy_set(pdrv, 0);
		lcd_mipi_disable(pdrv);
		break;
	case LCD_EDP:
		lcd_edp_disable(pdrv);
		lcd_phy_set(pdrv, 0);
		lcd_pinmux_set(pdrv, 0);
		break;
	default:
		break;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);
}

