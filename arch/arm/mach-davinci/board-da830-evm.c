/*
 * TI DA830/OMAP L137 EVM board
 *
 * Author: Mark A. Greer <mgreer@mvista.com>
 * Derived from: arch/arm/mach-davinci/board-dm644x-evm.c
 *
 * 2007, 2009 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c/pcf857x.h>
#include <linux/i2c/at24.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/common.h>
#include <mach/irqs.h>
#include <mach/cp_intc.h>
#include <mach/mux.h>
#include <mach/nand.h>
#include <mach/da8xx.h>
#include <mach/asp.h>
#include <mach/usb.h>

#define DA830_EVM_PHY_MASK		0x0
#define DA830_EVM_MDIO_FREQUENCY	2200000	/* PHY bus frequency */

#define DA830_EMIF25_ASYNC_DATA_CE3_BASE	0x62000000
#define DA830_EMIF25_CONTROL_BASE		0x68000000

static struct at24_platform_data da830_evm_i2c_eeprom_info = {
	.byte_len	= SZ_256K / 8,
	.page_size	= 64,
	.flags		= AT24_FLAG_ADDR16,
	.setup		= davinci_get_mac_addr,
	.context	= (void *)0x7f00,
};

static int da830_evm_ui_expander_setup(struct i2c_client *client, int gpio,
		unsigned ngpio, void *context)
{
	gpio_request(gpio + 6, "MUX_MODE");
#ifdef CONFIG_DA830_UI_LCD
	gpio_direction_output(gpio + 6, 0);
#else /* Must be NAND or NOR */
	gpio_direction_output(gpio + 6, 1);
#endif
	return 0;
}

static int da830_evm_ui_expander_teardown(struct i2c_client *client, int gpio,
		unsigned ngpio, void *context)
{
	gpio_free(gpio + 6);
	return 0;
}

static struct pcf857x_platform_data da830_evm_ui_expander_info = {
	.gpio_base	= DAVINCI_N_GPIO,
	.setup		= da830_evm_ui_expander_setup,
	.teardown	= da830_evm_ui_expander_teardown,
};

static struct i2c_board_info __initdata da830_evm_i2c_devices[] = {
	{
		I2C_BOARD_INFO("24c256", 0x50),
		.platform_data	= &da830_evm_i2c_eeprom_info,
	},
	{
		I2C_BOARD_INFO("tlv320aic3x", 0x18),
	},
	{
		I2C_BOARD_INFO("pcf8574", 0x3f),
		.platform_data	= &da830_evm_ui_expander_info,
	},
};

static struct davinci_i2c_platform_data da830_evm_i2c_0_pdata = {
	.bus_freq	= 100,	/* kHz */
	.bus_delay	= 0,	/* usec */
};

/*
 * USB1 VBUS is controlled by GPIO1[15], over-current is reported on GPIO2[4].
 */
#define ON_BD_USB_DRV	GPIO_TO_PIN(1, 15)
#define ON_BD_USB_OVC	GPIO_TO_PIN(2, 4)

static const short da830_evm_usb11_pins[] = {
	DA830_GPIO1_15, DA830_GPIO2_4,
	-1
};

static da8xx_ocic_handler_t da830_evm_usb_ocic_handler;

static int da830_evm_usb_set_power(unsigned port, int on)
{
	gpio_set_value(ON_BD_USB_DRV, on);
	return 0;
}

static int da830_evm_usb_get_power(unsigned port)
{
	return gpio_get_value(ON_BD_USB_DRV);
}

static int da830_evm_usb_get_oci(unsigned port)
{
	return !gpio_get_value(ON_BD_USB_OVC);
}

static irqreturn_t da830_evm_usb_ocic_irq(int, void *);

static int da830_evm_usb_ocic_notify(da8xx_ocic_handler_t handler)
{
	int irq 	= gpio_to_irq(ON_BD_USB_OVC);
	int error	= 0;

	if (handler != NULL) {
		da830_evm_usb_ocic_handler = handler;

		error = request_irq(irq, da830_evm_usb_ocic_irq, IRQF_DISABLED |
				    IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				    "OHCI over-current indicator", NULL);
		if (error)
			printk(KERN_ERR "%s: could not request IRQ to watch "
			       "over-current indicator changes\n", __func__);
	} else
		free_irq(irq, NULL);

	return error;
}

static struct da8xx_ohci_root_hub da830_evm_usb11_pdata = {
	.set_power	= da830_evm_usb_set_power,
	.get_power	= da830_evm_usb_get_power,
	.get_oci	= da830_evm_usb_get_oci,
	.ocic_notify	= da830_evm_usb_ocic_notify,

	/* TPS2065 switch @ 5V */
	.potpgt		= (3 + 1) / 2,	/* 3 ms max */
};

static irqreturn_t da830_evm_usb_ocic_irq(int irq, void *dev_id)
{
	da830_evm_usb_ocic_handler(&da830_evm_usb11_pdata, 1);
	return IRQ_HANDLED;
}

static __init void da830_evm_usb_init(void)
{
	u32 cfgchip2;
	int ret;

	/*
	 * Set up USB clock/mode in the CFGCHIP2 register.
	 * FYI:  CFGCHIP2 is 0x0000ef00 initially.
	 */
	cfgchip2 = __raw_readl(DA8XX_SYSCFG_VIRT(DA8XX_CFGCHIP2_REG));

	/* USB2.0 PHY reference clock is 24 MHz */
	cfgchip2 &= ~CFGCHIP2_REFFREQ;
	cfgchip2 |=  CFGCHIP2_REFFREQ_24MHZ;

	/*
	 * Select internal reference clock for USB 2.0 PHY
	 * and use it as a clock source for USB 1.1 PHY
	 * (this is the default setting anyway).
	 */
	cfgchip2 &= ~CFGCHIP2_USB1PHYCLKMUX;
	cfgchip2 |=  CFGCHIP2_USB2PHYCLKMUX;

	__raw_writel(cfgchip2, DA8XX_SYSCFG_VIRT(DA8XX_CFGCHIP2_REG));

	ret = da8xx_pinmux_setup(da830_evm_usb11_pins);
	if (ret) {
		pr_warning("%s: USB 1.1 PinMux setup failed: %d\n",
			   __func__, ret);
		return;
	}

	ret = gpio_request(ON_BD_USB_DRV, "ON_BD_USB_DRV");
	if (ret) {
		printk(KERN_ERR "%s: failed to request GPIO for USB 1.1 port "
		       "power control: %d\n", __func__, ret);
		return;
	}
	gpio_direction_output(ON_BD_USB_DRV, 0);

	ret = gpio_request(ON_BD_USB_OVC, "ON_BD_USB_OVC");
	if (ret) {
		printk(KERN_ERR "%s: failed to request GPIO for USB 1.1 port "
		       "over-current indicator: %d\n", __func__, ret);
		return;
	}
	gpio_direction_input(ON_BD_USB_OVC);

	ret = da8xx_register_usb11(&da830_evm_usb11_pdata);
	if (ret)
		pr_warning("%s: USB 1.1 registration failed: %d\n",
			   __func__, ret);
}

static struct davinci_uart_config da830_evm_uart_config __initdata = {
	.enabled_uarts = 0x7,
};

static const short da830_evm_mcasp1_pins[] = {
	DA830_AHCLKX1, DA830_ACLKX1, DA830_AFSX1, DA830_AHCLKR1, DA830_AFSR1,
	DA830_AMUTE1, DA830_AXR1_0, DA830_AXR1_1, DA830_AXR1_2, DA830_AXR1_5,
	DA830_ACLKR1, DA830_AXR1_6, DA830_AXR1_7, DA830_AXR1_8, DA830_AXR1_10,
	DA830_AXR1_11,
	-1
};

static u8 da830_iis_serializer_direction[] = {
	RX_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	TX_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,	INACTIVE_MODE,
};

static struct snd_platform_data da830_evm_snd_data = {
	.tx_dma_offset  = 0x2000,
	.rx_dma_offset  = 0x2000,
	.op_mode        = DAVINCI_MCASP_IIS_MODE,
	.num_serializer = ARRAY_SIZE(da830_iis_serializer_direction),
	.tdm_slots      = 2,
	.serial_dir     = da830_iis_serializer_direction,
	.eventq_no      = EVENTQ_0,
	.version	= MCASP_VERSION_2,
	.txnumevt	= 1,
	.rxnumevt	= 1,
};

/*
 * GPIO2[1] is used as MMC_SD_WP and GPIO2[2] as MMC_SD_INS.
 */
static const short da830_evm_mmc_sd_pins[] = {
	DA830_MMCSD_DAT_0, DA830_MMCSD_DAT_1, DA830_MMCSD_DAT_2,
	DA830_MMCSD_DAT_3, DA830_MMCSD_DAT_4, DA830_MMCSD_DAT_5,
	DA830_MMCSD_DAT_6, DA830_MMCSD_DAT_7, DA830_MMCSD_CLK,
	DA830_MMCSD_CMD,   DA830_GPIO2_1,     DA830_GPIO2_2,
	-1
};

static int da830_evm_mmc_get_ro(int index)
{
	int val, status, gpio_num = 33;

	status = gpio_request(gpio_num, "MMC WP\n");
	if (status < 0) {
		pr_warning("%s can not open GPIO %d\n", __func__, gpio_num);
		return 0;
	}
	gpio_direction_input(gpio_num);
	val = gpio_get_value(gpio_num);
	gpio_free(gpio_num);
	return val;
}

static struct davinci_mmc_config da830_evm_mmc_config = {
	.get_ro			= da830_evm_mmc_get_ro,
	.wires			= 4,
	.version		= MMC_CTLR_VERSION_2,
};

#ifdef CONFIG_DA830_UI_NAND
static struct mtd_partition da830_evm_nand_partitions[] = {
	/* bootloader (U-Boot, etc) in first sector */
	[0] = {
		.name		= "bootloader",
		.offset		= 0,
		.size		= SZ_128K,
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	/* bootloader params in the next sector */
	[1] = {
		.name		= "params",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_128K,
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	/* kernel */
	[2] = {
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_2M,
		.mask_flags	= 0,
	},
	/* file system */
	[3] = {
		.name		= "filesystem",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
		.mask_flags	= 0,
	}
};

/* flash bbt decriptors */
static uint8_t da830_evm_nand_bbt_pattern[] = { 'B', 'b', 't', '0' };
static uint8_t da830_evm_nand_mirror_pattern[] = { '1', 't', 'b', 'B' };

static struct nand_bbt_descr da830_evm_nand_bbt_main_descr = {
	.options	= NAND_BBT_LASTBLOCK | NAND_BBT_CREATE |
			  NAND_BBT_WRITE | NAND_BBT_2BIT |
			  NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs		= 2,
	.len		= 4,
	.veroffs	= 16,
	.maxblocks	= 4,
	.pattern	= da830_evm_nand_bbt_pattern
};

static struct nand_bbt_descr da830_evm_nand_bbt_mirror_descr = {
	.options	= NAND_BBT_LASTBLOCK | NAND_BBT_CREATE |
			  NAND_BBT_WRITE | NAND_BBT_2BIT |
			  NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs		= 2,
	.len		= 4,
	.veroffs	= 16,
	.maxblocks	= 4,
	.pattern	= da830_evm_nand_mirror_pattern
};

static struct davinci_nand_pdata da830_evm_nand_pdata = {
	.parts		= da830_evm_nand_partitions,
	.nr_parts	= ARRAY_SIZE(da830_evm_nand_partitions),
	.ecc_mode	= NAND_ECC_HW,
	.ecc_bits	= 4,
	.options	= NAND_USE_FLASH_BBT,
	.bbt_td		= &da830_evm_nand_bbt_main_descr,
	.bbt_md		= &da830_evm_nand_bbt_mirror_descr,
};

static struct resource da830_evm_nand_resources[] = {
	[0] = {		/* First memory resource is NAND I/O window */
		.start	= DA830_EMIF25_ASYNC_DATA_CE3_BASE,
		.end	= DA830_EMIF25_ASYNC_DATA_CE3_BASE + PAGE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {		/* Second memory resource is AEMIF control registers */
		.start	= DA830_EMIF25_CONTROL_BASE,
		.end	= DA830_EMIF25_CONTROL_BASE + SZ_32K - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device da830_evm_nand_device = {
	.name		= "davinci_nand",
	.id		= 1,
	.dev		= {
		.platform_data	= &da830_evm_nand_pdata,
	},
	.num_resources	= ARRAY_SIZE(da830_evm_nand_resources),
	.resource	= da830_evm_nand_resources,
};
#endif

static struct platform_device *da830_evm_devices[] __initdata = {
#ifdef CONFIG_DA830_UI_NAND
	&da830_evm_nand_device,
#endif
};

/*
 * UI board NAND/NOR flashes only use 8-bit data bus.
 */
static const short da830_evm_emif25_pins[] = {
	DA830_EMA_D_0, DA830_EMA_D_1, DA830_EMA_D_2, DA830_EMA_D_3,
	DA830_EMA_D_4, DA830_EMA_D_5, DA830_EMA_D_6, DA830_EMA_D_7,
	DA830_EMA_A_0, DA830_EMA_A_1, DA830_EMA_A_2, DA830_EMA_A_3,
	DA830_EMA_A_4, DA830_EMA_A_5, DA830_EMA_A_6, DA830_EMA_A_7,
	DA830_EMA_A_8, DA830_EMA_A_9, DA830_EMA_A_10, DA830_EMA_A_11,
	DA830_EMA_A_12, DA830_EMA_BA_0, DA830_EMA_BA_1, DA830_NEMA_WE,
	DA830_NEMA_CS_2, DA830_NEMA_CS_3, DA830_NEMA_OE, DA830_EMA_WAIT_0,
	-1
};

static __init void da830_evm_init(void)
{
	struct davinci_soc_info *soc_info = &davinci_soc_info;
	int ret;

	ret = da8xx_register_edma();
	if (ret)
		pr_warning("da830_evm_init: edma registration failed: %d\n",
				ret);

	ret = da8xx_pinmux_setup(da830_i2c0_pins);
	if (ret)
		pr_warning("da830_evm_init: i2c0 mux setup failed: %d\n",
				ret);

	ret = da8xx_register_i2c(0, &da830_evm_i2c_0_pdata);
	if (ret)
		pr_warning("da830_evm_init: i2c0 registration failed: %d\n",
				ret);

	da830_evm_usb_init();

	soc_info->emac_pdata->phy_mask = DA830_EVM_PHY_MASK;
	soc_info->emac_pdata->mdio_max_freq = DA830_EVM_MDIO_FREQUENCY;
	soc_info->emac_pdata->rmii_en = 1;

	ret = da8xx_pinmux_setup(da830_cpgmac_pins);
	if (ret)
		pr_warning("da830_evm_init: cpgmac mux setup failed: %d\n",
				ret);

	ret = da8xx_register_emac();
	if (ret)
		pr_warning("da830_evm_init: emac registration failed: %d\n",
				ret);

	ret = da8xx_register_watchdog();
	if (ret)
		pr_warning("da830_evm_init: watchdog registration failed: %d\n",
				ret);

	davinci_serial_init(&da830_evm_uart_config);
	i2c_register_board_info(1, da830_evm_i2c_devices,
			ARRAY_SIZE(da830_evm_i2c_devices));

	ret = da8xx_pinmux_setup(da830_evm_mcasp1_pins);
	if (ret)
		pr_warning("da830_evm_init: mcasp1 mux setup failed: %d\n",
				ret);

	da8xx_register_mcasp(1, &da830_evm_snd_data);

	ret = da8xx_pinmux_setup(da830_evm_mmc_sd_pins);
	if (ret)
		pr_warning("da830_evm_init: mmc/sd mux setup failed: %d\n",
				ret);

	ret = da8xx_register_mmcsd0(&da830_evm_mmc_config);
	if (ret)
		pr_warning("da830_evm_init: mmc/sd registration failed: %d\n",
				ret);

#ifdef CONFIG_DA830_UI
#ifdef CONFIG_DA830_UI_LCD
	ret = da8xx_pinmux_setup(da830_lcdcntl_pins);
	if (ret)
		pr_warning("da830_evm_init: lcdcntl mux setup failed: %d\n",
				ret);

	ret = da8xx_register_lcdc(&sharp_lcd035q3dg01_pdata);
	if (ret)
		pr_warning("da830_evm_init: lcd setup failed: %d\n", ret);
#else /* Must be NAND or NOR */
	ret = da8xx_pinmux_setup(da830_evm_emif25_pins);
	if (ret)
		pr_warning("da830_evm_init: emif25 mux setup failed: %d\n",
				ret);

	ret = platform_add_devices(da830_evm_devices,
			ARRAY_SIZE(da830_evm_devices));
	if (ret)
		pr_warning("da830_evm_init: EVM devices not added\n");
#endif
#endif

	ret = da8xx_register_rtc();
	if (ret)
		pr_warning("da830_evm_init: rtc setup failed: %d\n", ret);
}

#ifdef CONFIG_SERIAL_8250_CONSOLE
static int __init da830_evm_console_init(void)
{
	return add_preferred_console("ttyS", 2, "115200");
}
console_initcall(da830_evm_console_init);
#endif

static __init void da830_evm_irq_init(void)
{
	struct davinci_soc_info *soc_info = &davinci_soc_info;

	cp_intc_init((void __iomem *)DA8XX_CP_INTC_VIRT, DA830_N_CP_INTC_IRQ,
			soc_info->intc_irq_prios);
}

static void __init da830_evm_map_io(void)
{
	da830_init();
}

MACHINE_START(DAVINCI_DA830_EVM, "DaVinci DA830/OMAP-L137 EVM")
	.phys_io	= IO_PHYS,
	.io_pg_offst	= (__IO_ADDRESS(IO_PHYS) >> 18) & 0xfffc,
	.boot_params	= (DA8XX_DDR_BASE + 0x100),
	.map_io		= da830_evm_map_io,
	.init_irq	= da830_evm_irq_init,
	.timer		= &davinci_timer,
	.init_machine	= da830_evm_init,
MACHINE_END
