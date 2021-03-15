/*
 * drivers/amlogic/media/vout/vdac.h
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
*/

#ifndef _AML_VDAC_H
#define _AML_VDAC_H

#define VDAC_MODULE_MASK      (0x1f)
#define VDAC_MODULE_AVOUT_ATV (1 << 0) /*0x1*/
#define VDAC_MODULE_DTV_DEMOD (1 << 1) /*0x2*/
#define VDAC_MODULE_AVOUT_AV  (1 << 2) /*0x4*/
#define VDAC_MODULE_CVBS_OUT  (1 << 3) /*0x8*/
#define VDAC_MODULE_AUDIO_OUT (1 << 4) /*0x10*/

#define VDAC_CTRL_MAX         10

#define VDAC_REG_MAX          0xffff

enum vdac_cpu_type {
	VDAC_CPU_G12AB = 0,
	VDAC_CPU_SC2 = 1,
	VDAC_CPU_S4   = 2,
	VDAC_CPU_MAX,
};

struct meson_vdac_ctrl_s {
	unsigned int reg;
	unsigned int val;
	unsigned int bit;
	unsigned int len;
};

struct vdac_data_s {
	enum vdac_cpu_type cpu_id;
	unsigned int reg_ctrl0;
	unsigned int reg_ctrl1;
	struct meson_vdac_ctrl_s *vdac_ctrl;
};

void vdac_enable(bool on, unsigned int module_sel);
int vdac_ctrl_vref_adj(unsigned int value);
void vdac_ctrl_config_probe(void);

#endif
