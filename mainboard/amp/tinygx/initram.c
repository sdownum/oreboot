/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007 Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#define _MAINOBJECT

#include <types.h>
#include <lib.h>
#include <console.h>
#include <device/device.h>
#include <device/pci.h>
#include <string.h>
#include <msr.h>
#include <io.h>
#include <amd_geodelx.h>
#include <northbridge/amd/geodelx/raminit.h>

/* #include <device/smbus.h>
 * TODO: figure out how smbus functions should be done. See smbus_ops.c
 */
extern int smbus_read_byte(u16 device, u8 address);

#define MANUALCONF 0		/* Do automatic strapped PLL config */
#define PLLMSRHI 0x00001490	/* Manual settings for the PLL */
#define PLLMSRLO 0x02000030
#define DIMM0 ((u8) 0xA0)
#define DIMM1 ((u8) 0xA2)

/**
 * Read a byte from the SPD. 
 *
 * For this board, that is really just saying 'read a byte from SMBus'.
 * So we use smbus_read_byte(). Nota Bene: leave this here as a function 
 * rather than a #define in an obscure location. This function is called 
 * only a few dozen times, and it's not performance critical. 
 *
 * @param device The device.
 * @param address The address.
 * @return The data from the SMBus packet area or an error of 0xff (i.e. -1).
 */
u8 spd_read_byte(u16 device, u8 address)
{
	u8 spdbyte;

	printk(BIOS_DEBUG, "spd_read_byte dev %04x\n", device);
	spdbyte = smbus_read_byte(device, address);
	printk(BIOS_DEBUG, " addr %02x returns %02x\n", address, spdbyte);

	return spdbyte;
}

/**
 * Placeholder in case we ever need it. Since this file is a template for
 * other boards, we want this here and we want the call in the right place.
 */
static void mb_gpio_init(void)
{
	/* Early mainboard specific GPIO setup */
}

/**
 * main() for initram.
 *
 * It might seem that you could somehow do these functions in, e.g., the CPU
 * code, but the order of operations and what those operations are is VERY
 * strongly mainboard dependent. It's best to leave it in the mainboard code.
 */
int main(void)
{
	printk(BIOS_DEBUG, "Hi there from initram (stage1) main!\n");
	post_code(POST_START_OF_MAIN);

	system_preinit();
	printk(BIOS_DEBUG, "done preinit\n");

	mb_gpio_init();
	printk(BIOS_DEBUG, "done gpio init\n");

	pll_reset(MANUALCONF, PLLMSRHI, PLLMSRLO);
	printk(BIOS_DEBUG, "done pll reset\n");

	cpu_reg_init(0, DIMM0, DIMM1, DRAM_UNTERMINATED);
	printk(BIOS_DEBUG, "done cpu reg init\n");

	sdram_set_registers();
	printk(BIOS_DEBUG, "done sdram set registers\n");

	sdram_set_spd_registers(DIMM0, DIMM1);
	printk(BIOS_DEBUG, "done sdram set spd registers\n");

	sdram_enable(DIMM0, DIMM1);
	printk(BIOS_DEBUG, "done sdram enable\n");

	/* Check low memory. */
	/* ram_check(0x00000000, 640 * 1024); */

	printk(BIOS_DEBUG, "stage1 returns\n");
	return 0;
}
