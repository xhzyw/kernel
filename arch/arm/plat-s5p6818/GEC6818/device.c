/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/irq.h>
#include <linux/amba/pl022.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#if defined(CONFIG_NXP_HDMI_CEC)
#include <mach/nxp-hdmi-cec.h>
#endif

/*------------------------------------------------------------------------------
 * BUS Configure
 */
#if (CFG_BUS_RECONFIG_ENB == 1)
#include <mach/s5p6818_bus.h>

const u16 g_DrexQoS[2] = {
	0x100,		// S0
	0xFFF		// S1, Default value
};

const u8 g_TopBusSI[8] = {
	TOPBUS_SI_SLOT_DMAC0,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST0,
	TOPBUS_SI_SLOT_DMAC1,
	TOPBUS_SI_SLOT_SDMMC,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST1,
	TOPBUS_SI_SLOT_USBOTG
};

const u8 g_BottomBusSI[8] = {
	BOTBUS_SI_SLOT_1ST_ARM,
	BOTBUS_SI_SLOT_MALI,
	BOTBUS_SI_SLOT_DEINTERLACE,
	BOTBUS_SI_SLOT_1ST_CODA,
	BOTBUS_SI_SLOT_2ND_ARM,
	BOTBUS_SI_SLOT_SCALER,
	BOTBUS_SI_SLOT_TOP,
	BOTBUS_SI_SLOT_2ND_CODA
};

const u8 g_BottomQoSSI[2] = {
	1,	// Tidemark
	(1<<BOTBUS_SI_SLOT_1ST_ARM) |	// Control
	(1<<BOTBUS_SI_SLOT_2ND_ARM) |
	(1<<BOTBUS_SI_SLOT_MALI) |
	(1<<BOTBUS_SI_SLOT_TOP) |
	(1<<BOTBUS_SI_SLOT_DEINTERLACE) |
	(1<<BOTBUS_SI_SLOT_1ST_CODA)
};

const u8 g_DispBusSI[3] = {
	DISBUS_SI_SLOT_1ST_DISPLAY,
	DISBUS_SI_SLOT_2ND_DISPLAY,
	DISBUS_SI_SLOT_GMAC  // DISBUS_SI_SLOT_2ND_DISPLAY
};
#endif	/* #if (CFG_BUS_RECONFIG_ENB == 1) */

/*------------------------------------------------------------------------------
 * CPU Frequence
 */
#if defined(CONFIG_ARM_NXP_CPUFREQ)

static unsigned long dfs_freq_table[][2] = {
	//{ 1600000, 1340000, },
	//{ 1500000, 1280000, },
	{ 1400000, 1240000, },
	{ 1300000, 1180000, },
	{ 1200000, 1140000, },
	{ 1100000, 1100000, },
	{ 1000000, 1060000, },
	{  900000, 1040000, },
	{  800000, 1000000, },
	{  700000,  940000, },
	{  600000,  940000, },
	{  500000,  940000, },
	{  400000,  940000, },
};

struct nxp_cpufreq_plat_data dfs_plat_data = {
	.pll_dev	   	= CONFIG_NXP_CPUFREQ_PLLDEV,
	.supply_name	= "vdd_arm_1.3V",
	.supply_delay_us = 0,
	.freq_table	   	= dfs_freq_table,
	.table_size	   	= ARRAY_SIZE(dfs_freq_table),
};

static struct platform_device dfs_plat_device = {
	.name			= DEV_NAME_CPUFREQ,
	.dev			= {
		.platform_data	= &dfs_plat_data,
	}
};

#endif

/*------------------------------------------------------------------------------
 * Network DM9000
 */
#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
#include <linux/dm9000.h>

static struct resource dm9000_resource[] = {
	[0] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR,
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 1,		// 1 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[1] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR + 4,		// + 4 (8/16 BIT)
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 5,		// + 5 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[2] = {
		.start	= CFG_ETHER_EXT_IRQ_NUM,
		.end	= CFG_ETHER_EXT_IRQ_NUM,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	}
};

static struct dm9000_plat_data eth_plat_data = {
	.flags		= DM9000_PLATF_8BITONLY,	// DM9000_PLATF_16BITONLY
};

static struct platform_device dm9000_plat_device = {
	.name			= "dm9000",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(dm9000_resource),
	.resource		= dm9000_resource,
	.dev			= {
		.platform_data	= &eth_plat_data,
	}
};
#endif	/* CONFIG_DM9000 || CONFIG_DM9000_MODULE */


/*------------------------------------------------------------------------------
 * DW GMAC board config
 * hdc 20150413
 */
#if defined(CONFIG_NXPMAC_ETH)
#include <linux/phy.h>
#include <linux/nxpmac.h>
#include <linux/delay.h>
#include <linux/gpio.h>
int  nxpmac_init(struct platform_device *pdev)
{
    u32 addr;

	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E +  7), 3 );     // PAD_GPIOE7,     GMAC0_PHY_TXD[0]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E +  8), 3 );     // PAD_GPIOE8,     GMAC0_PHY_TXD[1]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E +  9), 3 );     // PAD_GPIOE9,     GMAC0_PHY_TXD[2]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 10), 3 );     // PAD_GPIOE10,    GMAC0_PHY_TXD[3]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 11), 3 );     // PAD_GPIOE11,    GMAC0_PHY_TXEN
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 14), 3 );     // PAD_GPIOE14,    GMAC0_PHY_RXD[0]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 15), 3 );     // PAD_GPIOE15,    GMAC0_PHY_RXD[1]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 16), 3 );     // PAD_GPIOE16,    GMAC0_PHY_RXD[2]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 17), 3 );     // PAD_GPIOE17,    GMAC0_PHY_RXD[3]
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 18), 3 );     // PAD_GPIOE18,    GMAC0_RX_CLK
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 19), 3 );     // PAD_GPIOE19,    GMAC0_PHY_RX_DV
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 20), 3 );     // PAD_GPIOE20,    GMAC0_GMII_MDC
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 21), 3 );     // PAD_GPIOE21,    GMAC0_GMII_MDI
	nxp_soc_gpio_set_io_drv( (PAD_GPIO_E + 24), 3 );     // PAD_GPIOE24,    GMAC0_GTX_CLK
	printk("NXP mac 1000Base-T gpio init ..................\n");

	// Clock control
	NX_CLKGEN_Initialize();
	addr = NX_CLKGEN_GetPhysicalAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE);
	NX_CLKGEN_SetBaseAddress( CLOCKINDEX_OF_DWC_GMAC_MODULE, (u32)IO_ADDRESS(addr) );

	NX_CLKGEN_SetClockSource( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 4);     // Sync mode for 100 & 10Base-T : External RX_clk
	NX_CLKGEN_SetClockDivisor( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 1);    // Sync mode for 100 & 10Base-T

	NX_CLKGEN_SetClockOutInv( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CFALSE);    // TX Clk invert off : 100 & 10Base-T
//	NX_CLKGEN_SetClockOutInv( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CTRUE);     // TX clk invert on : 100 & 10Base-T

	NX_CLKGEN_SetClockDivisorEnable( CLOCKINDEX_OF_DWC_GMAC_MODULE, CTRUE);

	// Reset control
	NX_RSTCON_Initialize();
	addr = NX_RSTCON_GetPhysicalAddress();
    NX_RSTCON_SetBaseAddress( (u32)IO_ADDRESS(addr) );
    NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_NEGATE);
    udelay(100);
    NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ASSERT);
    udelay(100);
    NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_NEGATE);
    udelay(100);

    gpio_request(CFG_ETHER_GMAC_PHY_RST_NUM,"Ethernet Rst pin");
	gpio_direction_output(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );

    gpio_free(CFG_ETHER_GMAC_PHY_RST_NUM);

     printk("NXP mac init ..................\n");
	return 0;
}

int gmac_phy_reset(void *priv)
{
	// Set GPIO nReset
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	msleep( 30 );

	return 0;
}

static struct stmmac_mdio_bus_data nxpmac0_mdio_bus = {
	.phy_reset = gmac_phy_reset,
	.phy_mask = 0,
	.probed_phy_irq = CFG_ETHER_GMAC_PHY_IRQ_NUM,
};

static struct plat_stmmacenet_data nxpmac_plat_data = {
    .phy_addr = 4,
    .clk_csr = 0x4, // PCLK 150~250 Mhz
    .speed = SPEED_1000,	// SPEED_1000
    .interface = PHY_INTERFACE_MODE_RGMII,
	.autoneg = AUTONEG_ENABLE, //AUTONEG_ENABLE or AUTONEG_DISABLE
	.duplex = DUPLEX_FULL,
	.pbl = 16,          /* burst 16 */
	.has_gmac = 1,      /* GMAC ethernet    */
	.enh_desc = 0,
	.mdio_bus_data = &nxpmac0_mdio_bus,
	.init = &nxpmac_init,
};

/* DWC GMAC Controller registration */
static struct resource nxpmac_resource[] = {
    [0] = DEFINE_RES_MEM(PHY_BASEADDR_GMAC, SZ_8K),
    [1] = DEFINE_RES_IRQ_NAMED(IRQ_PHY_GMAC, "macirq"),
};

static u64 nxpmac_dmamask = DMA_BIT_MASK(32);

struct platform_device nxp_gmac_dev = {
    .name           = "stmmaceth",  //"s5p4418-gmac",
    .id             = -1,
    .num_resources  = ARRAY_SIZE(nxpmac_resource),
    .resource       = nxpmac_resource,
    .dev            = {
        .dma_mask           = &nxpmac_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
        .platform_data      = &nxpmac_plat_data,
    }
};
#endif

/*------------------------------------------------------------------------------
 * DISPLAY (LVDS) / FB
 */
#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
static struct nxp_fb_plat_data fb0_plat_data = {
};

static struct platform_device fb0_device = {
	.name	= DEV_NAME_FB,
	.id		= 0,	/* FB device node num */
	.dev    = {
		.coherent_dma_mask 	= 0xffffffffUL,	/* for DMA allocate */
		.platform_data		= &fb0_plat_data
	},
};
#endif

static struct platform_device *fb_devices[] = {
	#if defined (CONFIG_FB0_NXP)
	&fb0_device,
	#endif
};
#endif /* CONFIG_FB_NXP */

/*------------------------------------------------------------------------------
 * backlight : generic pwm device
 */
#if defined(CONFIG_BACKLIGHT_PWM)
#include <linux/pwm_backlight.h>

static struct platform_pwm_backlight_data bl_plat_data = {
	.pwm_id			= CFG_LCD_PRI_PWM_CH,
	.max_brightness = 255,	/* 255 is 100%, set over 100% */
	.dft_brightness = 100,	/* 50% */
	.pwm_period_ns	= 1000000000/CFG_LCD_PRI_PWM_FREQ,
};

static struct platform_device bl_plat_device = {
	.name	= "pwm-backlight",
	.id		= -1,
	.dev	= {
		.platform_data	= &bl_plat_data,
	},
};

#endif

#if defined(CONFIG_PPM_NXP)
#include <mach/ppm.h>
struct nxp_ppm_platform_data ppm_plat_data = {
    .input_polarity = NX_PPM_INPUTPOL_INVERT,	//NX_PPM_INPUTPOL_INVERT, NX_PPM_INPUTPOL_BYPASS
};

static struct platform_device ppm_device = {
    .name           = DEV_NAME_PPM,
    .dev            = {
    	.platform_data  = &ppm_plat_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * NAND device
 */
#if defined(CONFIG_MTD_NAND_NXP)
#include <linux/mtd/partitions.h>
#include <asm-generic/sizes.h>

static struct mtd_partition nxp_nand_parts[] = {
#if 0
	{
		.name           = "root",
		.offset         =   0 * SZ_1M,
	},
#else
	{
		.name		= "system",
		.offset		=  64 * SZ_1M,
		.size		= 512 * SZ_1M,
	}, {
		.name		= "cache",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 256 * SZ_1M,
	}, {
		.name		= "userdata",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	}
#endif
};

static struct nxp_nand_plat_data nand_plat_data = {
	.parts		= nxp_nand_parts,
	.nr_parts	= ARRAY_SIZE(nxp_nand_parts),
	.chip_delay = 10,
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
		.platform_data	= &nand_plat_data,
	},
};
#endif	/* CONFIG_MTD_NAND_NXP */

#if defined(CONFIG_TOUCHSCREEN_GSLX680)
#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <linux/i2c.h>
#define	GSLX680_I2C_BUS		(1)
#define	AT24C02_I2C_BUS		(2)

static struct i2c_board_info __initdata gslX680_i2c_bdi = {
	.type	= "gslX680",
	.addr	= (0x40),
    	.irq    = PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),
};

static struct at24_platform_data at24c04 = {
	.byte_len = 4096/8,
	.page_size = 16,
	.flags = 0,
};

static struct i2c_board_info __initdata at24c02_i2c_bdi = {
	 I2C_BOARD_INFO("24c04", 0x50),
	.platform_data = &at24c04,		
};

#endif

#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <plat/ft5x0x_touch.h>
#define	FT5X0X_I2C_BUS		(1)
#define	AT24C02_I2C_BUS		(2)

static struct ft5x0x_i2c_platform_data ft5x0x_pdata = {
	.gpio_irq   = CFG_IO_TOUCH_PENDOWN_DETECT,
	.gpio_cfg    = 0xffffffff,
	.screen_max_x = 800,
	.screen_max_y = 480,
	.pressure_max = 255,

};

static struct at24_platform_data at24c04 = {
	.byte_len = 4096/8,
	.page_size = 16,
	.flags = 0,
};

static struct i2c_board_info __initdata ft5x0x_i2c_bdi = {
	.type	= "ft5x0x",
	.addr	= (0x38),
	.platform_data = &ft5x0x_pdata,
    	//.irq    = PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),
		
};

static struct i2c_board_info __initdata at24c02_i2c_bdi = {
	 I2C_BOARD_INFO("24c04", 0x50),
	.platform_data = &at24c04,		
};

static struct i2c_board_info __initdata ov5645 = {
	 I2C_BOARD_INFO("OV5645", 0x3c),
	//.platform_data = &ov5645_data,		
};

#endif

/*------------------------------------------------------------------------------
 * Keypad platform device
 */
#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)

#include <linux/input.h>

static unsigned int  button_gpio[] = CFG_KEYPAD_KEY_BUTTON;
static unsigned int  button_code[] = CFG_KEYPAD_KEY_CODE;

struct nxp_key_plat_data key_plat_data = {
	.bt_count	= ARRAY_SIZE(button_gpio),
	.bt_io		= button_gpio,
	.bt_code	= button_code,
	.bt_repeat	= CFG_KEYPAD_REPEAT,
};

static struct platform_device key_plat_device = {
	.name	= DEV_NAME_KEYPAD,
	.id		= -1,
	.dev    = {
		.platform_data	= &key_plat_data
	},
};
#endif	/* CONFIG_KEYBOARD_NXP_KEY || CONFIG_KEYBOARD_NXP_KEY_MODULE */

/*------------------------------------------------------------------------------
 * LEDs platform device
 * hdc 20150203
 */
#if defined(CONFIG_LEDS_GPIO)
static struct gpio_led GEC6818_leds[] = {
	{
		.name = "led1",	//D7
		.default_trigger = "none",
		.active_low = 1,
		.gpio = PAD_GPIO_E + 13,
	}, {
		.name = "led2",	//D8
		.default_trigger = "none",
		.active_low = 1,
		.gpio = PAD_GPIO_C + 17,
	}, {
		.name = "led3",	//D9
		.default_trigger = "none",
		.active_low = 1,
		.gpio = PAD_GPIO_C + 8,
	}, {
		.name = "led4",	//D10
		.default_trigger = "none",
		.active_low = 1,
		.gpio = PAD_GPIO_C + 7,
	},{
		.name = "led5",	//D11
		.default_trigger = "heartbeat",
		.active_low = 1,
		.gpio = PAD_GPIO_C + 12,
	},
};

static struct gpio_led_platform_data x4418_gpio_leds_platform_data = {
	.num_leds = ARRAY_SIZE(GEC6818_leds),
	.leds = GEC6818_leds,
};

static struct platform_device x4418_gpio_leds = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = &x4418_gpio_leds_platform_data,
	},
};
#endif

/*------------------------------------------------------------------------------
 * ASoC Codec platform device
 */
#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
static struct platform_device spdif_transciever = {
	.name	= "spdif-dit",
	.id		= -1,
};

struct nxp_snd_dai_plat_data spdif_trans_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device spdif_trans_dai = {
	.name	= "spdif-transciever",
	.id		= -1,
	.dev	= {
		.platform_data	= &spdif_trans_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_CODEC_ALC5623)
#include <linux/i2c.h>

#define	WM8976_I2C_BUS		(0)

/* CODEC */
static struct i2c_board_info __initdata alc5623_i2c_bdi = {
	.type	= "alc562x-codec",			// compatilbe with wm8976
	.addr	= (0x34>>1),		// 0x1A (7BIT), 0x34(8BIT)
};

/* DAI */
struct nxp_snd_dai_plat_data i2s_dai_data = {
	.i2s_ch	= 0,
	.sample_rate	= 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_B + 27,
		.detect_level	= 1,
	},
};

static struct platform_device alc5623_dai = {
	.name			= "alc5623-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &i2s_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_CODEC_ES8316) || defined(CONFIG_SND_CODEC_ES8316_MODULE)
#include <linux/i2c.h>

#define	ES8316_I2C_BUS		(0)

/* CODEC */
static struct i2c_board_info __initdata es8316_i2c_bdi = {
	.type	= "es8316",
	.addr	= (0x22>>1),		// 0x11 (7BIT), 0x22(8BIT)
};

/* DAI */
struct nxp_snd_dai_plat_data i2s_dai_data = {
	.i2s_ch	= 0,
	.sample_rate	= 48000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
#if 1
	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_B + 27,
		.detect_level	= 1,
	},
#endif
};

static struct platform_device es8316_dai = {
	.name			= "es8316-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &i2s_dai_data,
	}
};
#endif


/*------------------------------------------------------------------------------
 * G-Sensor platform device
 */
#if defined(CONFIG_SENSORS_MMA865X) || defined(CONFIG_SENSORS_MMA865X_MODULE)
#include <linux/i2c.h>

#define	MMA865X_I2C_BUS		(2)

/* CODEC */
static struct i2c_board_info __initdata mma865x_i2c_bdi = {
	.type	= "mma8653",
	.addr	= 0x1D//(0x3a),
};

#endif

#if defined(CONFIG_SENSORS_STK831X) || defined(CONFIG_SENSORS_STK831X_MODULE)
#include <linux/i2c.h>

#define	STK831X_I2C_BUS		(2)

/* CODEC */
static struct i2c_board_info __initdata stk831x_i2c_bdi = {
#if   defined CONFIG_SENSORS_STK8312
	.type	= "stk8312",
	.addr	= (0x3d),
#elif defined CONFIG_SENSORS_STK8313
	.type	= "stk8313",
	.addr	= (0x22),
#endif
};

#endif

/*------------------------------------------------------------------------------
 *  * reserve mem
 *   */
#ifdef CONFIG_CMA
#include <linux/cma.h>
extern void nxp_cma_region_reserve(struct cma_region *, const char *);

void __init nxp_reserve_mem(void)
{
    static struct cma_region regions[] = {
        {
            .name = "ion",
#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
            .size = CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K,
#else
			.size = 0,
#endif
            {
                .alignment = PAGE_SIZE,
            }
        },
        {
            .size = 0
        }
    };

    static const char map[] __initconst =
        "ion-nxp=ion;"
        "nx_vpu=ion;";

#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
    printk("%s: reserve CMA: size %d\n", __func__, CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K);
#endif
    nxp_cma_region_reserve(regions, map);
}
#endif

#if defined(CONFIG_I2C_NXP) || defined (CONFIG_I2C_SLSI)
#define I2CUDELAY(x)	1000000/x
/* gpio i2c 3 */
#define	I2C3_SCL	PAD_GPIO_C + 15
#define	I2C3_SDA	PAD_GPIO_C + 16

static struct i2c_gpio_platform_data nxp_i2c_gpio_port3 = {
	.sda_pin	= I2C3_SDA,
	.scl_pin	= I2C3_SCL,
	.udelay		= I2CUDELAY(CFG_I2C3_CLK),				/* Gpio_mode CLK Rate = 1/( udelay*2) * 1000000 */

	.timeout	= 10,
};


static struct platform_device i2c_device_ch3 = {
	.name	= "i2c-gpio",
	.id		= 3,
	.dev    = {
		.platform_data	= &nxp_i2c_gpio_port3,
	},
};

static struct platform_device *i2c_devices[] = {
	&i2c_device_ch3,
};
#endif /* CONFIG_I2C_NXP || CONFIG_I2C_SLSI */

/*------------------------------------------------------------------------------
 * v4l2 platform device
 * 摄像头 
*/
#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>

//时钟设置
static int camera_common_set_clock(ulong clk_rate)
{
    PM_DBGOUT("%s: %d\n", __func__, (int)clk_rate);
    if (clk_rate > 0)
        nxp_soc_pwm_set_frequency(1, clk_rate, 50);//设置pwm的频率
    else
        nxp_soc_pwm_set_frequency(1, 0, 0);
    msleep(1);
    return 0;
}

static bool is_camera_port_configured = false;//定义摄像头的管脚配置是否为真
static void camera_common_vin_setup_io(int module, bool force)//io设置
{
    if (!force && is_camera_port_configured)
        return;
    else {
        u_int *pad;
        int i, len;
        u_int io, fn;


        /* VIP0:0 = VCLK, VID0 ~ 7 */
        const u_int port[][2] = {
             /* VCLK, HSYNC, VSYNC */
            { PAD_GPIO_E + 4, NX_GPIO_PADFUNC_1 },	//hdc 20150410
            { PAD_GPIO_E + 5, NX_GPIO_PADFUNC_1 },	
            { PAD_GPIO_E + 6, NX_GPIO_PADFUNC_1 },	

            { PAD_GPIO_D + 28, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 29, NX_GPIO_PADFUNC_1 },//hdc 20150410
            { PAD_GPIO_D + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 31, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  0, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  1, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  3, NX_GPIO_PADFUNC_1 },
        };

        printk("%s\n", __func__);

        pad = (u_int *)port;
        len = sizeof(port)/sizeof(port[0]);

        for (i = 0; i < len; i++) {
            io = *pad++;
            fn = *pad++;
            nxp_soc_gpio_set_io_dir(io, 0);
            nxp_soc_gpio_set_io_func(io, fn);
        }

        is_camera_port_configured = true;
    }
}

static void mipi_vin_setup_io(int module, bool force)
{
    /* do nothing */
    printk("wwj %s\n",__func__);
}
static int mipi_phy_enable(bool en)
{
    printk("%s\n",__func__);
    return 0;
}
struct nxp_mipi_csi_platformdata ov5645_plat_data = {
    .module     = 0,
    .clk_rate   = 24000000, // 24MHz
    .lanes      = 2,
    .alignment = 0,
    .hs_settle  = 0,
    .width      = 1280,
    .height     = 960,
    .fixed_phy_vdd = false,
    .irq        = 0, /* not used */
    .base       = 0, /* not used */
    .phy_enable = mipi_phy_enable
};
static bool camera_power_enabled = false;
static void camera_power_control(int enable)
{
    struct regulator *cam_core_18V = NULL;

    if (enable && camera_power_enabled)
        return;
    if (!enable && !camera_power_enabled)
        return;

    cam_core_18V = regulator_get(NULL, "vcam1_1.8V");
    if (IS_ERR(cam_core_18V)) {
        printk(KERN_ERR "%s: failed to regulator_get() for vcam1_1.8V", __func__);
        return;
    }
    printk("%s: %d\n", __func__, enable);
    if (enable) {
        regulator_enable(cam_core_18V);
    } else {
        regulator_disable(cam_core_18V);
    }

    regulator_put(cam_core_18V);

    camera_power_enabled = enable ? true : false;
}

static bool is_back_camera_enabled = false;
static bool is_back_camera_power_state_changed = false;
static bool is_front_camera_enabled = false;
static bool is_front_camera_power_state_changed = false;

static int front_camera_power_enable(bool on);
static char camera_type[16] = {0};
static int back_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_BACK_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
    PM_DBGOUT("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);
    if (on) {
        front_camera_power_enable(0);
        if (!is_back_camera_enabled) {
            camera_power_control(1);

            /* First RST signal to low */
            nxp_soc_gpio_set_out_value(reset_io, 0);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);

	    nxp_soc_gpio_set_out_value(io, 1);		//ok;hdc 20150718
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
	    nxp_soc_gpio_set_out_value(io, 1);		//ok;hdc 20150718;PN's status is important
            mdelay(1);

	    camera_common_set_clock(24000000);
	    mdelay(1);
	    nxp_soc_gpio_set_out_value(io, 0);		//ok;hdc 20150718
	    mdelay(1);
	    nxp_soc_gpio_set_out_value(reset_io, 1);
	    mdelay(1);
	    nxp_soc_gpio_set_out_value(io, 1);		//ok;hdc 20150718
       	    mdelay(5);

            is_back_camera_enabled = true;
            is_back_camera_power_state_changed = true;
        } else {
            is_back_camera_power_state_changed = false;
        }
    } else {
        if (is_back_camera_enabled) {
            nxp_soc_gpio_set_out_value(io, 0);
            is_back_camera_enabled = false;
            is_back_camera_power_state_changed = true;
        } else {
            nxp_soc_gpio_set_out_value(io, 0);
            is_back_camera_power_state_changed = false;
        }
        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}

//ov5645 mipi;hdc 20151118
static int ov5645_back_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_FRONT_POWER_DOWN;
    printk("<0>""%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);
    if (on) {
        front_camera_power_enable(0);
        if (!is_back_camera_enabled) {
	    camera_common_set_clock(24000000);
	    mdelay(6);

            camera_power_control(1);
	    mdelay(5);

	    /* PWDN signal High to Low */
            nxp_soc_gpio_set_out_value(io, 0);
	    mdelay(1);
            nxp_soc_gpio_set_io_dir(io, 1);
	    mdelay(1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
	    mdelay(5);
            nxp_soc_gpio_set_out_value(io, 1);
            mdelay(6);

	    mdelay(10);
            is_back_camera_enabled = true;
            is_back_camera_power_state_changed = true;
        } else {
            is_back_camera_power_state_changed = false;
        }
    } else {
        if (is_back_camera_enabled) {
	    nxp_soc_gpio_set_out_value(io, 0);
            is_back_camera_enabled = false;
            is_back_camera_power_state_changed = true;
        } else {
	    nxp_soc_gpio_set_out_value(io, 0);
            is_back_camera_power_state_changed = false;
        }
        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }
    return 0;
}

//sp2519 camera
static int sp2519_back_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_BACK_POWER_DOWN;
    unsigned int pn_io = CFG_IO_CAMERA_FRONT_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
    printk("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);
    if (on) {
        front_camera_power_enable(0);
        if (!is_back_camera_enabled) {
            camera_power_control(1);
		
	    /*PN signal*/
	    nxp_soc_gpio_set_out_value(pn_io, 0);
            nxp_soc_gpio_set_io_dir(pn_io, 1);
            nxp_soc_gpio_set_io_func(pn_io, nxp_soc_gpio_get_altnum(pn_io));
            nxp_soc_gpio_set_out_value(pn_io, 0);
            /* PD signal */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 0);

	    /* RST signal */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);

            camera_common_set_clock(24000000);

            /*PD ON*/
	    mdelay(1);
	    nxp_soc_gpio_set_out_value(io, 1);
            mdelay(1);
	    /*PD Low*/
            nxp_soc_gpio_set_out_value(io, 0);
            /*RST ON*/
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(10);
            is_back_camera_enabled = true;
            is_back_camera_power_state_changed = true;
        } else {
            is_back_camera_power_state_changed = false;
        }
    } else {
        if (is_back_camera_enabled) {
            nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_out_value(reset_io, 0);
            is_back_camera_enabled = false;
            is_back_camera_power_state_changed = true;
        } else {
            nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            is_back_camera_power_state_changed = false;
        }

        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}

//HM5065
static int hm5065_back_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_FRONT_POWER_DOWN;    //(PAD_GPIO_C + 6)
    unsigned int reset_io = CFG_IO_CAMERA_RESET;         //(PAD_GPIO_C + 5)
    unsigned int pn_io = CFG_IO_CAMERA_BACK_POWER_DOWN;    //(PAD_GPIO_C + 4)
    printk("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);
    if (on) {
        //back_camera_power_enable(0);
	nxp_soc_gpio_set_out_value(pn_io, 1);
	nxp_soc_gpio_set_io_dir(pn_io, 1);
	nxp_soc_gpio_set_io_func(pn_io, nxp_soc_gpio_get_altnum(pn_io));
	nxp_soc_gpio_set_out_value(pn_io, 1);

        if (!is_back_camera_enabled) {
            camera_power_control(1);

	    /* PD signal */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            camera_common_set_clock(24000000);
            /* mdelay(10); */
            mdelay(1);
            nxp_soc_gpio_set_out_value(io, 0);
            /* RST signal */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);
            /* mdelay(100); */
            mdelay(1);
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(1);

            is_back_camera_enabled = true;
            is_back_camera_power_state_changed = true;
        } else {
            is_back_camera_power_state_changed = false;
        }
    } else {
        if (is_back_camera_enabled) {
	    nxp_soc_gpio_set_out_value(io, 1);
	    nxp_soc_gpio_set_out_value(pn_io, 0);
            nxp_soc_gpio_set_out_value(reset_io, 0);
            is_back_camera_enabled = false;
            is_back_camera_power_state_changed = true;
        } else {
	    nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
	   nxp_soc_gpio_set_out_value(pn_io, 0);
            is_back_camera_power_state_changed = false;
        }
        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}

static bool back_camera_power_state_changed(void)
{
    return is_back_camera_power_state_changed;
}

static struct i2c_board_info back_camera_i2c_boardinfo[] = {
    {
        //I2C_BOARD_INFO("SP2518", 0x60>>1),
	I2C_BOARD_INFO("S5K4ECGX", 0x5A>>1),
    },
    {
	I2C_BOARD_INFO("OV5645", 0x78>>1),
    },
    {
	I2C_BOARD_INFO("SP2519", 0x60>>1),
    },
     {
	I2C_BOARD_INFO("HM5065", 0x1f), 
    },
};

static int front_camera_power_enable(bool on)
{
#if 0
    unsigned int io = CFG_IO_CAMERA_FRONT_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
    PM_DBGOUT("%s: is_front_camera_enabled %d, on %d\n", __func__, is_front_camera_enabled, on);
    if (on) {
        back_camera_power_enable(0);
        if (!is_front_camera_enabled) {
            camera_power_control(1);
            /* First RST signal to low */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);
            mdelay(1);

            /* PWDN signal High to Low */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            camera_common_set_clock(24000000);
            mdelay(10);
            /* mdelay(1); */
            nxp_soc_gpio_set_out_value(io, 0);
            /* mdelay(10); */
            mdelay(10);

            /* RST signal  to High */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(5);

            is_front_camera_enabled = true;
            is_front_camera_power_state_changed = true;
        } else {
            is_front_camera_power_state_changed = false;
        }
    } else {
        if (is_front_camera_enabled) {
            nxp_soc_gpio_set_out_value(io, 1);
            is_front_camera_enabled = false;
            is_front_camera_power_state_changed = true;
        } else {
            nxp_soc_gpio_set_out_value(io, 1);
            is_front_camera_power_state_changed = false;
        }
        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }
#endif
    return 0;
}

static bool front_camera_power_state_changed(void)
{
    return is_front_camera_power_state_changed;
}

static struct i2c_board_info front_camera_i2c_boardinfo[] = {
    {
        I2C_BOARD_INFO("SP0838", 0x18),
    },
};

static struct nxp_v4l2_i2c_board_info sensor[] = {
    {	//S5K4EC
        .board_info = &back_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 0,
    },
    {
        .board_info = &front_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 0,
    },
    {	//OV5645
        .board_info = &back_camera_i2c_boardinfo[1],
	.i2c_adapter_id = 0
    },
    {	//SP2519
        .board_info = &back_camera_i2c_boardinfo[2],
	.i2c_adapter_id = 0
    },
     {	//HM5065
        .board_info = &back_camera_i2c_boardinfo[3],
	.i2c_adapter_id = 0
    }
};


static struct nxp_capture_platformdata capture_plat_data[] = {  
    {
        /* back_camera 601 interface */
	//.module = 1,	//VIP1
	.module = 0,	//VIP2
        .sensor = &sensor[0],
        .type = NXP_CAPTURE_INF_PARALLEL,
        .parallel = {
            .is_mipi        = false,
            .external_sync  = true,	
            .h_active       = 1280,
	    .h_frontporch   = 0,	
	    .h_syncwidth    = 0,	
	    .h_backporch    = 2,	
            .v_active       = 960,
	    .v_frontporch   = 0,	
	    .v_syncwidth    = 0,	
	    .v_backporch    = 2,	
            .clock_invert   = false,	
            .port           = 0,
            .data_order     = NXP_VIN_CRY1CBY0,	
            .interlace      = false,	
            .clk_rate       = 24000000,
            .late_power_down = true,	
            .power_enable   = back_camera_power_enable,
            .power_state_changed = back_camera_power_state_changed,
            .set_clock      = camera_common_set_clock,
            .setup_io       = camera_common_vin_setup_io,
        },
        .deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
    },
    {
        /* front_camera 601 interface */
        // for 5430
        //.module = 1,
	.module = 2,
        /*.module = 0,*/
        .sensor = &sensor[1],
        .type = NXP_CAPTURE_INF_PARALLEL,
        .parallel = {
            .is_mipi        = false,
            .external_sync  = true,
            .h_active       = 640,
            .h_frontporch   = 1,
            .h_syncwidth    = 1,
            .h_backporch    = 0,
            .v_active       = 480,
            .v_frontporch   = 0,
            .v_syncwidth    = 1,
            .v_backporch    = 0,
            .clock_invert   = false,
            .port           = 0,
            .data_order     = NXP_VIN_CBY0CRY1,
            .interlace      = false,
            .clk_rate       = 24000000,//24MHz
            .late_power_down = true,
            .power_enable   = NULL,
	    .power_state_changed = NULL,

            .set_clock      = NULL,
            .setup_io = NULL,
        },
        .deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
    },
    { 0, NULL, 0, },
};

static int __init cam_type_setup(char *str)
{
	if (str == NULL) {
		return 1;
	}

	if (strcmp(str, "OV5645") == 0) {
		printk("<0>""camera type is %s--------------------zr\n", str);
		strcpy(camera_type, str);
		capture_plat_data[0].sensor = &sensor[2];
		capture_plat_data[0].type = NXP_CAPTURE_INF_CSI;
		capture_plat_data[0].parallel.is_mipi = true;
		capture_plat_data[0].parallel.h_active = 1280;
		capture_plat_data[0].parallel.h_frontporch = 15;
		capture_plat_data[0].parallel.h_syncwidth = 1;
		capture_plat_data[0].parallel.h_backporch = 10;
		capture_plat_data[0].parallel.v_active  =960; 
		capture_plat_data[0].parallel.v_frontporch = 2;
		capture_plat_data[0].parallel.v_syncwidth = 1;
		capture_plat_data[0].parallel.v_backporch = 1;
		capture_plat_data[0].parallel.port = NX_VIP_INPUTPORT_B;
		capture_plat_data[0].parallel.data_order = NXP_VIN_Y1CRY0CB;
		capture_plat_data[0].parallel.clk_rate = 24000000;
		capture_plat_data[0].parallel.power_enable = ov5645_back_camera_power_enable,
		capture_plat_data[0].parallel.set_clock =camera_common_set_clock;
		capture_plat_data[0].parallel.setup_io = mipi_vin_setup_io;
		capture_plat_data[0].csi = &ov5645_plat_data;	
	}else if(strcmp(str, "sp2519") == 0){
		printk("camera type is %s--------------------zr\n", str);
		strcpy(camera_type, str);
		capture_plat_data[0].sensor = &sensor[3];
		capture_plat_data[0].parallel.h_active = 1600;
		capture_plat_data[0].parallel.h_frontporch = 1;
		capture_plat_data[0].parallel.h_syncwidth = 1;
		capture_plat_data[0].parallel.h_backporch = 0;
		capture_plat_data[0].parallel.v_active  =1200; 
		capture_plat_data[0].parallel.v_frontporch = 0;
		capture_plat_data[0].parallel.v_syncwidth = 1;
		capture_plat_data[0].parallel.v_backporch = 0;
		capture_plat_data[0].parallel.data_order =  NXP_VIN_Y0CBY1CR;
		capture_plat_data[0].parallel.power_enable =  sp2519_back_camera_power_enable;
	}else if(strcmp(str, "hm5065") == 0){
		printk("camera type is %s--------------------zr\n", str);
		strcpy(camera_type, str);
		capture_plat_data[0].sensor = &sensor[4];
		capture_plat_data[0].parallel.h_active = 608;
		capture_plat_data[0].parallel.h_frontporch = 7;
		capture_plat_data[0].parallel.h_syncwidth = 0;
		capture_plat_data[0].parallel.h_backporch = 10;
		capture_plat_data[0].parallel.v_active  =479; 
		capture_plat_data[0].parallel.v_frontporch = 0;
		capture_plat_data[0].parallel.v_syncwidth = 0;
		capture_plat_data[0].parallel.v_backporch = 2;
		capture_plat_data[0].parallel.data_order =  NXP_VIN_CBY0CRY1;
		capture_plat_data[0].parallel.power_enable =  hm5065_back_camera_power_enable;
	}
}

__setup("cam=", cam_type_setup);

/* out platformdata */
static struct i2c_board_info hdmi_edid_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_edid", 0xA0>>1),
};

static struct nxp_v4l2_i2c_board_info edid = {
    .board_info = &hdmi_edid_i2c_boardinfo,
    .i2c_adapter_id = 0,
};

static struct i2c_board_info hdmi_hdcp_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_hdcp", 0x74>>1),
};

static struct nxp_v4l2_i2c_board_info hdcp = {
    .board_info = &hdmi_hdcp_i2c_boardinfo,
    .i2c_adapter_id = 0,
};


static void hdmi_set_int_external(int gpio)
{
    nxp_soc_gpio_set_int_enable(gpio, 0);
    nxp_soc_gpio_set_int_mode(gpio, 1); /* high level */
    nxp_soc_gpio_set_int_enable(gpio, 1);
    nxp_soc_gpio_clr_int_pend(gpio);
}

static void hdmi_set_int_internal(int gpio)
{
    nxp_soc_gpio_set_int_enable(gpio, 0);
    nxp_soc_gpio_set_int_mode(gpio, 0); /* low level */
    nxp_soc_gpio_set_int_enable(gpio, 1);
    nxp_soc_gpio_clr_int_pend(gpio);
}

static int hdmi_read_hpd_gpio(int gpio)
{
    return nxp_soc_gpio_get_in_value(gpio);
}

static struct nxp_out_platformdata out_plat_data = {
    .hdmi = {
        .internal_irq = 0,
        .external_irq = 0,//PAD_GPIO_A + 19,
        .set_int_external = hdmi_set_int_external,
        .set_int_internal = hdmi_set_int_internal,
        .read_hpd_gpio = hdmi_read_hpd_gpio,
        .edid = &edid,
        .hdcp = &hdcp,
    },
};

static struct nxp_v4l2_platformdata v4l2_plat_data = {
    .captures = &capture_plat_data[0],
    .out = &out_plat_data,
};

static struct platform_device nxp_v4l2_dev = {
    .name       = NXP_V4L2_DEV_NAME,
    .id         = 0,
    .dev        = {
        .platform_data = &v4l2_plat_data,
    },
};
#endif /* CONFIG_V4L2_NXP || CONFIG_V4L2_NXP_MODULE */

/*------------------------------------------------------------------------------
 * SSP/SPI
 */
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
#include <linux/spi/spi.h>
static void spi0_cs(u32 chipselect)
{
#if (CFG_SPI0_CS_GPIO_MODE)
	if(nxp_soc_gpio_get_io_func( CFG_SPI0_CS )!= nxp_soc_gpio_get_altnum( CFG_SPI0_CS))
		nxp_soc_gpio_set_io_func( CFG_SPI0_CS, nxp_soc_gpio_get_altnum( CFG_SPI0_CS));

	nxp_soc_gpio_set_io_dir( CFG_SPI0_CS,1);
	nxp_soc_gpio_set_out_value(	 CFG_SPI0_CS , chipselect);
#else
	;
#endif
}
struct pl022_config_chip spi0_info = {
    /* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
    .com_mode = CFG_SPI0_COM_MODE,
    .iface = SSP_INTERFACE_MOTOROLA_SPI,
    /* We can only act as master but SSP_SLAVE is possible in theory */
    .hierarchy = SSP_MASTER,
    /* 0 = drive TX even as slave, 1 = do not drive TX as slave */
    .slave_tx_disable = 1,
    .rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
    .tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
    .ctrl_len = SSP_BITS_8,
    .wait_state = SSP_MWIRE_WAIT_ZERO,
    .duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
    /*
     * This is where you insert a call to a function to enable CS
     * (usually GPIO) for a certain chip.
     */
#if (CFG_SPI0_CS_GPIO_MODE)
    .cs_control = spi0_cs,
#endif
	.clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,

};

static struct spi_board_info spi_plat_board[] __initdata = {
    [0] = {
        .modalias        = "spidev",    /* fixup */
        .max_speed_hz    = 3125000,     /* max spi clock (SCK) speed in HZ */
        .bus_num         = 0,           /* Note> set bus num, must be smaller than ARRAY_SIZE(spi_plat_device) */
        .chip_select     = 0,           /* Note> set chip select num, must be smaller than spi cs_num */
        .controller_data = &spi0_info,
        .mode            = SPI_MODE_3 | SPI_CPOL | SPI_CPHA,
    },
};

#endif
/*------------------------------------------------------------------------------
 * DW MMC board config
 */
#if defined(CONFIG_MMC_DW)
static int _dwmci0_init(u32 slot_id, irq_handler_t handler, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	int io  = CFG_SDMMC0_DETECT_IO;
	int irq = IRQ_GPIO_START + io;
	int id  = 0, ret = 0;

	printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

	ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				DEV_NAME_SDHC "0", (void*)host->slot[slot_id]);
	if (0 > ret)
		pr_err("dw_mmc dw_mmc.%d: fail request interrupt %d ...\n", id, irq);
	return 0;
}

static int _dwmci0_get_ro(u32 slot_id)
{
	return 0;
}

static int _dwmci0_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC0_DETECT_IO;
	return nxp_soc_gpio_get_in_value(io);
}

static int _dwmci0_ext_cd_init(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

static int _dwmci0_ext_cd_cleanup(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

#ifdef CONFIG_MMC_NXP_CH0
static struct dw_mci_board _dwmci0_data = {
	.quirks			= DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 50 * 1000 * 1000,
	.caps			= MMC_CAP_CMD23,
	.detect_delay_ms= 200,
	.cd_type		= DW_MCI_CD_EXTERNAL,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0x1c) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
	.init			= _dwmci0_init,
	.get_ro         = _dwmci0_get_ro,
	.get_cd			= _dwmci0_get_cd,
	.ext_cd_init	= _dwmci0_ext_cd_init,
	.ext_cd_cleanup	= _dwmci0_ext_cd_cleanup,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH0_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode 			= PIO_MODE,
#endif
};
#endif

static int _dwmci1_init(u32 slot_id, irq_handler_t handler, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	int io  = CFG_SDMMC1_DETECT_IO;
	int irq = IRQ_GPIO_START + io;
	int id  = 1, ret = 0;

	printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

	ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				DEV_NAME_SDHC "1", (void*)host->slot[slot_id]);
	if (0 > ret)
		pr_err("dw_mmc dw_mmc.%d: fail request interrupt %d ...\n", id, irq);
	return 0;
}

static int _dwmci1_get_ro(u32 slot_id)
{
	return 0;
}

static int _dwmci1_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC1_DETECT_IO;
	return nxp_soc_gpio_get_in_value(io);
}

static int _dwmci1_ext_cd_init(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

static int _dwmci1_ext_cd_cleanup(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

#ifdef CONFIG_MMC_NXP_CH1
static struct dw_mci_board _dwmci1_data = {
	.quirks			= DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 50 * 1000 * 1000,
	.caps 			= MMC_CAP_CMD23,
	.detect_delay_ms= 200,
	.cd_type 		= DW_MCI_CD_EXTERNAL,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0x1c) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
	.init			= _dwmci1_init,
	.get_ro         = _dwmci1_get_ro,
	.get_cd			= _dwmci1_get_cd,
	.ext_cd_init	= _dwmci1_ext_cd_init,
	.ext_cd_cleanup	= _dwmci1_ext_cd_cleanup,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH1_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode 			= PIO_MODE,
#endif
};
#endif

#ifdef CONFIG_MMC_NXP_CH2
static struct dw_mci_board _dwmci2_data = {
    .quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
				  	  DW_MCI_QUIRK_HIGHSPEED |
				  	  DW_MMC_QUIRK_HW_RESET_PW |
				      DW_MCI_QUIRK_NO_DETECT_EBIT,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 |
					  MMC_CAP_NONREMOVABLE |
			 	  	  MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23 |
				  	  MMC_CAP_ERASE | MMC_CAP_HW_RESET,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0x1c) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),

	.desc_sz		= 4,
	.detect_delay_ms= 200,
	.sdr_timing		= 0x01010001,
	.ddr_timing		= 0x03030002,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH2_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode 			= PIO_MODE,
#endif
};
#endif

#endif /* CONFIG_MMC_DW */

/*------------------------------------------------------------------------------
 * RFKILL driver
 */
#if defined(CONFIG_NXP_RFKILL)

struct rfkill_dev_data  rfkill_dev_data =
{
	.supply_name 	= "vgps_3.3V",	// vwifi_3.3V, vgps_3.3V
	.module_name 	= "wlan",
	.initval		= RFKILL_INIT_SET | RFKILL_INIT_OFF,
    .delay_time_off	= 1000,
};

struct nxp_rfkill_plat_data rfkill_plat_data = {
	.name		= "WiFi-Rfkill",
	.type		= RFKILL_TYPE_WLAN,
	.rf_dev		= &rfkill_dev_data,
    .rf_dev_num	= 1,
};

static struct platform_device rfkill_device = {
	.name			= DEV_NAME_RFKILL,
	.dev			= {
		.platform_data	= &rfkill_plat_data,
	}
};
#endif	/* CONFIG_RFKILL_NXP */

/*------------------------------------------------------------------------------
 * USB HSIC power control.
 */
int nxp_hsic_phy_pwr_on(struct platform_device *pdev, bool on)
{
	return 0;
}
EXPORT_SYMBOL(nxp_hsic_phy_pwr_on);

/*------------------------------------------------------------------------------
 * HDMI CEC driver
 */
#if defined(CONFIG_NXP_HDMI_CEC)
static struct platform_device hdmi_cec_device = {
	.name			= NXP_HDMI_CEC_DRV_NAME,
};
#endif /* CONFIG_NXP_HDMI_CEC */

/*------------------------------------------------------------------------------
 * SLsiAP Thermal Unit
 */
#if defined(CONFIG_SENSORS_NXP_TMU)

struct nxp_tmu_trigger tmu_triggers[] = {
	{
		.trig_degree	=  80,	// 160
		.trig_duration	=  100,
		.trig_cpufreq	=  800*1000,	/* Khz */
	},
};

static struct nxp_tmu_platdata tmu_data = {
	.channel  = 0,
	.triggers = tmu_triggers,
	.trigger_size = ARRAY_SIZE(tmu_triggers),
	.poll_duration = 100,
};

static struct platform_device tmu_device = {
	.name			= "nxp-tmu",
	.dev			= {
		.platform_data	= &tmu_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * register board platform devices
 */
void __init nxp_board_devs_register(void)
{
	printk("[Register board platform devices]\n");

#if defined(CONFIG_ARM_NXP_CPUFREQ)
	printk("plat: add dynamic frequency (pll.%d)\n", dfs_plat_data.pll_dev);
	platform_device_register(&dfs_plat_device);
#endif

#if defined(CONFIG_SENSORS_NXP_TMU)
	printk("plat: add device TMU\n");
	platform_device_register(&tmu_device);
#endif

#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
	fb0_plat_data.module = CONFIG_FB0_NXP_DISPOUT,
	fb0_plat_data.layer = CFG_DISP_PRI_SCREEN_LAYER,
	fb0_plat_data.format = CFG_DISP_PRI_SCREEN_RGB_FORMAT,
	fb0_plat_data.bgcolor = CFG_DISP_PRI_BACK_GROUND_COLOR,
	fb0_plat_data.bitperpixel = CFG_DISP_PRI_SCREEN_PIXEL_BYTE * 8,
	fb0_plat_data.x_resol = CFG_DISP_PRI_RESOL_WIDTH,
	fb0_plat_data.y_resol = CFG_DISP_PRI_RESOL_HEIGHT,
	fb0_plat_data.buffers = 3,
	fb0_plat_data.skip_pan_vsync = 0,
	fb0_plat_data.lcd_with_mm = CFG_DISP_PRI_LCD_WIDTH_MM,
	fb0_plat_data.lcd_height_mm	= CFG_DISP_PRI_LCD_HEIGHT_MM,
#endif
	printk("plat: add framebuffer\n");
	platform_add_devices(fb_devices, ARRAY_SIZE(fb_devices));
#endif

#if defined(CONFIG_MMC_DW)
	#ifdef CONFIG_MMC_NXP_CH2
	nxp_mmc_add_device(2, &_dwmci2_data);
	#endif
	#ifdef CONFIG_MMC_NXP_CH0
	nxp_mmc_add_device(0, &_dwmci0_data);
	#endif
    #ifdef CONFIG_MMC_NXP_CH1
	nxp_mmc_add_device(1, &_dwmci1_data);
	#endif
#endif

#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
	printk("plat: add device dm9000 net\n");
	platform_device_register(&dm9000_plat_device);
#endif

#if defined(CONFIG_BACKLIGHT_PWM)
	printk("plat: add backlight pwm device\n");
	platform_device_register(&bl_plat_device);
#endif

#if defined(CONFIG_MTD_NAND_NXP)
	platform_device_register(&nand_plat_device);
#endif

#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)
	printk("plat: add device keypad\n");
	platform_device_register(&key_plat_device);
#endif

#if defined(CONFIG_LEDS_GPIO)
	printk("plat: add device gpio_led\n");
	platform_device_register(&x4418_gpio_leds);
#endif	//hdc 20150204

#if defined(CONFIG_I2C_NXP) || defined (CONFIG_I2C_SLSI)
    platform_add_devices(i2c_devices, ARRAY_SIZE(i2c_devices));
#endif

#if defined(CONFIG_SND_CODEC_ALC5623) || defined(CONFIG_SND_CODEC_ALC5623_MODULE)
	printk("plat: add device asoc-alc5623\n");
	i2c_register_board_info(0, &alc5623_i2c_bdi, 1);
	platform_device_register(&alc5623_dai);
#endif

#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
	printk("plat: add device spdif playback\n");
	platform_device_register(&spdif_transciever);
	platform_device_register(&spdif_trans_dai);
#endif

#if defined(CONFIG_SND_CODEC_ES8316) || defined(CONFIG_SND_CODEC_ES8316_MODULE)
	printk("plat: add device asoc-es8316\n");
	i2c_register_board_info(ES8316_I2C_BUS, &es8316_i2c_bdi, 1);
	platform_device_register(&es8316_dai);
#endif

#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
    printk("plat: add device nxp-v4l2\n");
    platform_device_register(&nxp_v4l2_dev);
#endif

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
    spi_register_board_info(spi_plat_board, ARRAY_SIZE(spi_plat_board));
    printk("plat: register spidev\n");
#endif

#if defined(CONFIG_TOUCHSCREEN_GSLX680)
	printk("plat: add touch(gslX680) device\n");
	i2c_register_board_info(GSLX680_I2C_BUS, &gslX680_i2c_bdi, 1);
	i2c_register_board_info(AT24C02_I2C_BUS, &at24c02_i2c_bdi, 1);
#endif




#if defined(CONFIG_SENSORS_MMA865X) || defined(CONFIG_SENSORS_MMA865X_MODULE)
	printk("plat: add g-sensor mma865x\n");
	i2c_register_board_info(2, &mma865x_i2c_bdi, 1);
#elif defined(CONFIG_SENSORS_MMA7660) || defined(CONFIG_SENSORS_MMA7660_MODULE)
	printk("plat: add g-sensor mma7660\n");
	i2c_register_board_info(MMA7660_I2C_BUS, &mma7660_i2c_bdi, 1);
#endif

#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
	printk("plat: add touch(ft5x0x_ts) device\n");
	//I2C_BOARD_INFO("gslX680",(0x70>>1));
	i2c_register_board_info(FT5X0X_I2C_BUS, &ft5x0x_i2c_bdi, 1);
	i2c_register_board_info(AT24C02_I2C_BUS, &at24c02_i2c_bdi, 1);
	i2c_register_board_info(0, &ov5645, 1);
#endif


#if defined(CONFIG_RFKILL_NXP)
    printk("plat: add device rfkill\n");
    platform_device_register(&rfkill_device);
#endif

#if defined(CONFIG_PPM_NXP)
    printk("plat: add device ppm\n");
    platform_device_register(&ppm_device);
#endif

#if defined(CONFIG_NXPMAC_ETH)
    printk("plat: add device nxp-gmac\n");
    platform_device_register(&nxp_gmac_dev);
#endif

#if defined(CONFIG_NXP_HDMI_CEC)
    printk("plat: add device hdmi-cec\n");
    platform_device_register(&hdmi_cec_device);
#endif

#if 0//defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	back_camera_power_enable(0);
	front_camera_power_enable(0);
	camera_power_control(0);
#endif
	/* END */
	printk("\n");
}
