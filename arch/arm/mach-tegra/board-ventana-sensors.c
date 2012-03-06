/*
 * arch/arm/mach-tegra/board-ventana-sensors.c
 *
 * Copyright (c) 2011, NVIDIA CORPORATION, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of NVIDIA CORPORATION nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/akm8975.h>
#include <linux/mpu.h>
#include <linux/i2c/pca954x.h>
#include <linux/i2c/pca953x.h>
#include <linux/nct1008.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>

#include <mach/gpio.h>
//Eric  0505 begin
#include <linux/kxtf9.h>
//Eric	 end
/* Compal Indigo-Carl begin */
//#include <media/ov5650.h>
//#include <media/ov2710.h>
//#include <media/ssl3250a.h>
#ifdef CONFIG_VIDEO_MT9D115
#include <media/yuv_sensor_mt9d115.h>
#endif
#ifdef CONFIG_VIDEO_MT9P111
#include <media/yuv_sensor_mt9p111.h>
#endif
/* Compal Indigo-Carl end */
#include <generated/mach-types.h>

#include "gpio-names.h"
#include "board.h"
#include "board-ventana.h"

#define ISL29018_IRQ_GPIO	TEGRA_GPIO_PZ2
#define AKM8975_IRQ_GPIO	TEGRA_GPIO_PN5
#define CAMERA_POWER_GPIO	TEGRA_GPIO_PV4
//#define CAMERA_CSI_MUX_SEL_GPIO	TEGRA_GPIO_PBB4
//#define CAMERA_FLASH_ACT_GPIO	TEGRA_GPIO_PD2
//#define CAMERA_FLASH_STRB_GPIO	TEGRA_GPIO_PA0
#define AC_PRESENT_GPIO		TEGRA_GPIO_PV3
#define NCT1008_THERM2_GPIO	TEGRA_GPIO_PN6
//#define CAMERA_FLASH_OP_MODE		0 /*0=I2C mode, 1=GPIO mode*/
//#define CAMERA_FLASH_MAX_LED_AMP	7
//#define CAMERA_FLASH_MAX_TORCH_AMP	11
//#define CAMERA_FLASH_MAX_FLASH_AMP	31
/* Compal Indigo-Carl begin */
#ifdef CONFIG_VIDEO_MT9D115
#define YUV_SENSOR_OE_L_GPIO    TEGRA_GPIO_PBB5
#define YUV_SENSOR_RST_GPIO     TEGRA_GPIO_PD2
#endif
#ifdef CONFIG_VIDEO_MT9P111
#define YUV5_PWR_DN_GPIO        TEGRA_GPIO_PBB4
#define YUV5_RST_L_GPIO         TEGRA_GPIO_PA0
#endif
/* Compal Indigo-Carl end */

/* Compal Carl begin */
struct camera_gpios {
	const char *name;
	int gpio;
	int enabled;
        int milliseconds;
};

#define CAMERA_GPIO(_name, _gpio, _enabled, _milliseconds)		        \
	{						                        \
		.name = _name,				                        \
		.gpio = _gpio,				                        \
		.enabled = _enabled,			                        \
		.milliseconds = _milliseconds,				        \
	}
/* Compal Carl end */

extern void tegra_throttling_enable(bool enable);

/* Compal Indigo-Carl begin */
#if 0
static int ventana_camera_init(void)
{
	tegra_gpio_enable(CAMERA_POWER_GPIO);
	gpio_request(CAMERA_POWER_GPIO, "camera_power_en");
	gpio_direction_output(CAMERA_POWER_GPIO, 1);
	gpio_export(CAMERA_POWER_GPIO, false);

	tegra_gpio_enable(CAMERA_CSI_MUX_SEL_GPIO);
	gpio_request(CAMERA_CSI_MUX_SEL_GPIO, "camera_csi_sel");
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 0);
	gpio_export(CAMERA_CSI_MUX_SEL_GPIO, false);

	return 0;
}
#endif

#if 0
/* left ov5650 is CAM2 which is on csi_a */
static int ventana_left_ov5650_power_on(void)
{
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 0);
	gpio_direction_output(AVDD_DSI_CSI_ENB_GPIO, 1);
	gpio_direction_output(CAM2_LDO_SHUTDN_L_GPIO, 1);
	mdelay(5);
	gpio_direction_output(CAM2_PWR_DN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(CAM2_RST_L_GPIO, 0);
	mdelay(1);
	gpio_direction_output(CAM2_RST_L_GPIO, 1);
	mdelay(20);
	return 0;
}

static int ventana_left_ov5650_power_off(void)
{
	gpio_direction_output(AVDD_DSI_CSI_ENB_GPIO, 0);
	gpio_direction_output(CAM2_RST_L_GPIO, 0);
	gpio_direction_output(CAM2_PWR_DN_GPIO, 1);
	gpio_direction_output(CAM2_LDO_SHUTDN_L_GPIO, 0);
	return 0;
}

struct ov5650_platform_data ventana_left_ov5650_data = {
	.power_on = ventana_left_ov5650_power_on,
	.power_off = ventana_left_ov5650_power_off,
};

/* right ov5650 is CAM1 which is on csi_b */
static int ventana_right_ov5650_power_on(void)
{
	gpio_direction_output(AVDD_DSI_CSI_ENB_GPIO, 1);
	gpio_direction_output(CAM1_LDO_SHUTDN_L_GPIO, 1);
	mdelay(5);
	gpio_direction_output(CAM1_PWR_DN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(CAM1_RST_L_GPIO, 0);
	mdelay(1);
	gpio_direction_output(CAM1_RST_L_GPIO, 1);
	mdelay(20);
	return 0;
}

static int ventana_right_ov5650_power_off(void)
{
	gpio_direction_output(AVDD_DSI_CSI_ENB_GPIO, 0);
	gpio_direction_output(CAM1_RST_L_GPIO, 0);
	gpio_direction_output(CAM1_PWR_DN_GPIO, 1);
	gpio_direction_output(CAM1_LDO_SHUTDN_L_GPIO, 0);
	return 0;
}

struct ov5650_platform_data ventana_right_ov5650_data = {
	.power_on = ventana_right_ov5650_power_on,
	.power_off = ventana_right_ov5650_power_off,
};

static int ventana_ov2710_power_on(void)
{
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 1);
	gpio_direction_output(AVDD_DSI_CSI_ENB_GPIO, 1);
	gpio_direction_output(CAM3_LDO_SHUTDN_L_GPIO, 1);
	mdelay(5);
	gpio_direction_output(CAM3_PWR_DN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(CAM3_RST_L_GPIO, 0);
	mdelay(1);
	gpio_direction_output(CAM3_RST_L_GPIO, 1);
	mdelay(20);
	return 0;
}

static int ventana_ov2710_power_off(void)
{
	gpio_direction_output(CAM3_RST_L_GPIO, 0);
	gpio_direction_output(CAM3_PWR_DN_GPIO, 1);
	gpio_direction_output(CAM3_LDO_SHUTDN_L_GPIO, 0);
	gpio_direction_output(AVDD_DSI_CSI_ENB_GPIO, 0);
	gpio_direction_output(CAMERA_CSI_MUX_SEL_GPIO, 0);
	return 0;
}

struct ov2710_platform_data ventana_ov2710_data = {
	.power_on = ventana_ov2710_power_on,
	.power_off = ventana_ov2710_power_off,
};
#endif
/* Compal Indigo-Carl end */

/* Compal Carl begin */
#ifdef CONFIG_VIDEO_MT9D115
static struct camera_gpios yuv_sensor_gpio_keys[] = {
	[0] = CAMERA_GPIO("cam_power_en", CAMERA_POWER_GPIO, 1, 0),
	[1] = CAMERA_GPIO("yuv_sensor_oe_l", YUV_SENSOR_OE_L_GPIO, 0, 0),
	[2] = CAMERA_GPIO("yuv_sensor_rst_lo", YUV_SENSOR_RST_GPIO, 1, 0),
};

static int yuv_sensor_power_on(void)
{
	int i, ret;

	pr_info("yuv %s\n",__func__);
	for (i = 0; i < ARRAY_SIZE(yuv_sensor_gpio_keys); i++) {
		tegra_gpio_enable(yuv_sensor_gpio_keys[i].gpio);
		ret = gpio_request(yuv_sensor_gpio_keys[i].gpio,
				yuv_sensor_gpio_keys[i].name);
		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio #%d\n",
				__func__, i);
			goto fail;
		}
	}

	gpio_direction_output(YUV_SENSOR_OE_L_GPIO, 0);
	gpio_direction_output(YUV_SENSOR_RST_GPIO, 1);
	msleep(1);
	gpio_direction_output(CAMERA_POWER_GPIO, 1);
	msleep(1);
	gpio_direction_output(YUV_SENSOR_RST_GPIO, 0);
	msleep(1);
	gpio_direction_output(YUV_SENSOR_RST_GPIO, 1);

	for (i = 0; i < ARRAY_SIZE(yuv_sensor_gpio_keys); i++) {
		gpio_export(yuv_sensor_gpio_keys[i].gpio, false);
	}

	return 0;

fail:
	while (i--)
		gpio_free(yuv_sensor_gpio_keys[i].gpio);
	return ret;
}

static int yuv_sensor_power_off(void)
{
        int i;
	pr_info("yuv %s\n",__func__);
	gpio_direction_output(YUV_SENSOR_OE_L_GPIO, 1);
	gpio_direction_output(CAMERA_POWER_GPIO, 0);
	gpio_direction_output(YUV_SENSOR_OE_L_GPIO, 0);
	gpio_direction_output(YUV_SENSOR_RST_GPIO, 0);

	i = ARRAY_SIZE(yuv_sensor_gpio_keys);
	while (i--)
		gpio_free(yuv_sensor_gpio_keys[i].gpio);
	return 0;
}

struct yuv_sensor_platform_data yuv_sensor_data = {
	.power_on = yuv_sensor_power_on,
	.power_off = yuv_sensor_power_off,
};
#endif // MT9D115

#ifdef CONFIG_VIDEO_MT9P111
static struct camera_gpios yuv5_sensor_gpio_keys[] = {
	[0] = CAMERA_GPIO("cam_power_en", CAMERA_POWER_GPIO, 1, 0),
	[1] = CAMERA_GPIO("yuv5_sensor_pwdn", YUV5_PWR_DN_GPIO, 0, 0),
	[2] = CAMERA_GPIO("yuv5_sensor_rst_lo", YUV5_RST_L_GPIO, 1, 0),
};

static int yuv5_sensor_power_on(void)
{
	int i, ret;

	pr_info("yuv5 %s\n",__func__);
	for (i = 0; i < ARRAY_SIZE(yuv5_sensor_gpio_keys); i++) {
		tegra_gpio_enable(yuv5_sensor_gpio_keys[i].gpio);
		ret = gpio_request(yuv5_sensor_gpio_keys[i].gpio,
				yuv5_sensor_gpio_keys[i].name);
		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio #%d\n",
				__func__, i);
			goto fail;
		}
	}

	gpio_direction_output(YUV5_PWR_DN_GPIO, 0);
	gpio_direction_output(YUV5_RST_L_GPIO, 0);
	msleep(1);
	gpio_direction_output(CAMERA_POWER_GPIO, 1);
	msleep(1);
	gpio_direction_output(YUV5_RST_L_GPIO, 1);
	msleep(1);
	
	for (i = 0; i < ARRAY_SIZE(yuv5_sensor_gpio_keys); i++) {
		gpio_export(yuv5_sensor_gpio_keys[i].gpio, false);
	}

	return 0;

fail:
	while (i--)
		gpio_free(yuv5_sensor_gpio_keys[i].gpio);
	return ret;
}

static int yuv5_sensor_power_off(void)
{
	int i;
	pr_info("yuv5 %s\n",__func__);
	gpio_direction_output(YUV5_RST_L_GPIO, 0);
	msleep(1);
	gpio_direction_output(YUV5_PWR_DN_GPIO, 1);
	gpio_direction_output(CAMERA_POWER_GPIO, 0);
	gpio_direction_output(YUV5_PWR_DN_GPIO, 0);

	i = ARRAY_SIZE(yuv5_sensor_gpio_keys);
	while (i--)
		gpio_free(yuv5_sensor_gpio_keys[i].gpio);
	return 0;
};

struct yuv5_sensor_platform_data yuv5_sensor_data = {
	.power_on = yuv5_sensor_power_on,
	.power_off = yuv5_sensor_power_off,
};
#endif // MT9P111

static int ventana_camera_init(void)
{
	int i, ret;

#ifdef CONFIG_VIDEO_MT9D115
	// initialize MT9D115
	for (i = 0; i < ARRAY_SIZE(yuv_sensor_gpio_keys); i++) {
		tegra_gpio_enable(yuv_sensor_gpio_keys[i].gpio);
		ret = gpio_request(yuv_sensor_gpio_keys[i].gpio,
				yuv_sensor_gpio_keys[i].name);
		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio #%d\n",
				__func__, i);
			goto fail2;
		}
	}

	gpio_direction_output(CAMERA_POWER_GPIO, 0);
	gpio_direction_output(YUV_SENSOR_OE_L_GPIO, 0);
	gpio_direction_output(YUV_SENSOR_RST_GPIO, 0);

	for (i = 0; i < ARRAY_SIZE(yuv_sensor_gpio_keys); i++) {
		gpio_free(yuv_sensor_gpio_keys[i].gpio);
		gpio_export(yuv_sensor_gpio_keys[i].gpio, false);
	}
#endif

#ifdef CONFIG_VIDEO_MT9P111
	// initialize MT9P111
	for (i = 0; i < ARRAY_SIZE(yuv5_sensor_gpio_keys); i++) {
		tegra_gpio_enable(yuv5_sensor_gpio_keys[i].gpio);
		ret = gpio_request(yuv5_sensor_gpio_keys[i].gpio,
				yuv5_sensor_gpio_keys[i].name);
		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio #%d\n",
				__func__, i);
			goto fail3;
		}
	}

	gpio_direction_output(CAMERA_POWER_GPIO, 0);
	gpio_direction_output(YUV5_PWR_DN_GPIO, 0);
	gpio_direction_output(YUV5_RST_L_GPIO, 0);

	for (i = 0; i < ARRAY_SIZE(yuv5_sensor_gpio_keys); i++) {
		gpio_free(yuv5_sensor_gpio_keys[i].gpio);
		gpio_export(yuv5_sensor_gpio_keys[i].gpio, false);
	}
#endif

	return 0;

#ifdef CONFIG_VIDEO_MT9D115
fail2:
        while (i--)
                gpio_free(yuv_sensor_gpio_keys[i].gpio);
	return ret;
#endif

#ifdef CONFIG_VIDEO_MT9P111
fail3:
	while (i--)
		gpio_free(yuv5_sensor_gpio_keys[i].gpio);
	return ret;
#endif
}
/* Compal Indigo-Carl end */

#if 0
static int ventana_ssl3250a_init(void)
{
	gpio_request(CAMERA_FLASH_ACT_GPIO, "torch_gpio_act");
	gpio_direction_output(CAMERA_FLASH_ACT_GPIO, 0);
	tegra_gpio_enable(CAMERA_FLASH_ACT_GPIO);
	gpio_request(CAMERA_FLASH_STRB_GPIO, "torch_gpio_strb");
	gpio_direction_output(CAMERA_FLASH_STRB_GPIO, 0);
	tegra_gpio_enable(CAMERA_FLASH_STRB_GPIO);
	gpio_export(CAMERA_FLASH_STRB_GPIO, false);
	return 0;
}

static void ventana_ssl3250a_exit(void)
{
	gpio_set_value(CAMERA_FLASH_STRB_GPIO, 0);
	gpio_free(CAMERA_FLASH_STRB_GPIO);
	tegra_gpio_disable(CAMERA_FLASH_STRB_GPIO);
	gpio_set_value(CAMERA_FLASH_ACT_GPIO, 0);
	gpio_free(CAMERA_FLASH_ACT_GPIO);
	tegra_gpio_disable(CAMERA_FLASH_ACT_GPIO);
}

static int ventana_ssl3250a_gpio_strb(int val)
{
	int prev_val;
	prev_val = gpio_get_value(CAMERA_FLASH_STRB_GPIO);
	gpio_set_value(CAMERA_FLASH_STRB_GPIO, val);
	return prev_val;
};

static int ventana_ssl3250a_gpio_act(int val)
{
	int prev_val;
	prev_val = gpio_get_value(CAMERA_FLASH_ACT_GPIO);
	gpio_set_value(CAMERA_FLASH_ACT_GPIO, val);
	return prev_val;
};

static struct ssl3250a_platform_data ventana_ssl3250a_data = {
	.config		= CAMERA_FLASH_OP_MODE,
	.max_amp_indic	= CAMERA_FLASH_MAX_LED_AMP,
	.max_amp_torch	= CAMERA_FLASH_MAX_TORCH_AMP,
	.max_amp_flash	= CAMERA_FLASH_MAX_FLASH_AMP,
	.init		= ventana_ssl3250a_init,
	.exit		= ventana_ssl3250a_exit,
	.gpio_act	= ventana_ssl3250a_gpio_act,
	.gpio_en1	= NULL,
	.gpio_en2	= NULL,
	.gpio_strb	= ventana_ssl3250a_gpio_strb,
};
#endif

//Eric-0425 begin
//add light sensor driver
static void ventana_al3000a_ls_init(void)
{
	tegra_gpio_enable(ISL29018_IRQ_GPIO);
	gpio_request(ISL29018_IRQ_GPIO, "al3000a_ls");
	gpio_direction_input(ISL29018_IRQ_GPIO);
}
//#ifdef CONFIG_SENSORS_AK8975
static void ventana_akm8975_init(void)
{
	tegra_gpio_enable(AKM8975_IRQ_GPIO);
	gpio_request(AKM8975_IRQ_GPIO, "akm8975");
	gpio_direction_input(AKM8975_IRQ_GPIO);
}
//#endif
static void ventana_kxtf9_init(void)
{
	tegra_gpio_enable(TEGRA_GPIO_PN4);
	gpio_request(TEGRA_GPIO_PN4, "kxtf9");
	gpio_direction_input(TEGRA_GPIO_PN4);
}
//Eric-0425 end

/*static void ventana_bq20z75_init(void)
{
	tegra_gpio_enable(AC_PRESENT_GPIO);
	gpio_request(AC_PRESENT_GPIO, "ac_present");
	gpio_direction_input(AC_PRESENT_GPIO);
}*/

static void ventana_ECBat_init(void)
{
	tegra_gpio_enable(AC_PRESENT_GPIO);
	gpio_request(AC_PRESENT_GPIO, "ac_present");
	gpio_direction_input(AC_PRESENT_GPIO);
	/* compal indigo-Howard Chang 20110513 begin */
	/* enable DOCK_ON pin  */
	tegra_gpio_enable(TEGRA_GPIO_PS7);
      /* compal indigo-Howard Chang 20110513 end */	
}

static void ventana_nct1008_init(void)
{
	tegra_gpio_enable(NCT1008_THERM2_GPIO);
	gpio_request(NCT1008_THERM2_GPIO, "temp_alert");
	gpio_direction_input(NCT1008_THERM2_GPIO);
}

static struct nct1008_platform_data ventana_nct1008_pdata = {
	.supported_hwrev = true,
	.ext_range = false,
	.conv_rate = 0x08,
	.offset = 0,
	.hysteresis = 0,
	.shutdown_ext_limit = 105,
	.shutdown_local_limit = 115,
	.throttling_ext_limit = 90,
	.alarm_fn = tegra_throttling_enable,
};

static const struct i2c_board_info ventana_i2c0_board_info[] = {
#if 0
	{
		I2C_BOARD_INFO("isl29018", 0x44),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PZ2),
	},
#else
//Eric-0425 begin
//add light sensor driver
	{
		I2C_BOARD_INFO("al3000a_ls", 0x1C),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PZ2),
	},
//Eric-0425 end
#endif
};
/*static const struct i2c_board_info ventana_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("bq20z75-battery", 0x0B),
		.irq = TEGRA_GPIO_TO_IRQ(AC_PRESENT_GPIO),
	},
};*/

static const struct i2c_board_info ventana_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("EC_Battery", 0x58),
		.irq = TEGRA_GPIO_TO_IRQ(AC_PRESENT_GPIO),
	},
};

#if 0  // Compal Indigo-Carl
static struct pca953x_platform_data ventana_tca6416_data = {
	.gpio_base      = TEGRA_NR_GPIOS + 4, /* 4 gpios are already requested by tps6586x */
};

static struct pca954x_platform_mode ventana_pca9546_modes[] = {
	{ .adap_id = 6, .deselect_on_exit = 1 }, /* REAR CAM1 */
	{ .adap_id = 7, .deselect_on_exit = 1 }, /* REAR CAM2 */
	{ .adap_id = 8, .deselect_on_exit = 1 }, /* FRONT CAM3 */
};

static struct pca954x_platform_data ventana_pca9546_data = {
	.modes	  = ventana_pca9546_modes,
	.num_modes      = ARRAY_SIZE(ventana_pca9546_modes),
};

static const struct i2c_board_info ventana_i2c3_board_info_tca6416[] = {
	{
		I2C_BOARD_INFO("tca6416", 0x20),
		.platform_data = &ventana_tca6416_data,
	},
};

static const struct i2c_board_info ventana_i2c3_board_info_pca9546[] = {
	{
		I2C_BOARD_INFO("pca9546", 0x70),
		.platform_data = &ventana_pca9546_data,
	},
};

static const struct i2c_board_info ventana_i2c3_board_info_ssl3250a[] = {
	{
		I2C_BOARD_INFO("ssl3250a", 0x30),
		.platform_data = &ventana_ssl3250a_data,
	},
};
#endif  // Compal Indigo-Carl

///Eric 0505 begin
struct kxtf9_platform_data kxtf9_pdata = {

                .min_interval           = 1,
                .poll_interval          = 10,
                .g_range                = KXTF9_G_2G,
                .shift_adj              = SHIFT_ADJ_2G,
                .axis_map_x             = 1,
                .axis_map_y             = 0,
                .axis_map_z             = 2,
                .negate_x               = 0,
                .negate_y               = 0,
                .negate_z               = 1,
                .data_odr_init          = ODR100F,
                .ctrl_reg1_init         = ( KXTF9_G_2G | RES_12BIT ) & ~TDTE & ~TPE & ~WUFE,
                .int_ctrl_init          = 0x00,
                .tilt_timer_init        = 0x00,
                .engine_odr_init        = OTP1_6 | OWUF25 | OTDT50,
                .wuf_timer_init         = 0x00,
                .wuf_thresh_init        = 0x08,
                .tdt_timer_init         = 0x78,
                .tdt_h_thresh_init      = 0xB6,
                .tdt_l_thresh_init      = 0x1A,
                .tdt_tap_timer_init     = 0xA2,
                .tdt_total_timer_init   = 0x24,
                .tdt_latency_timer_init = 0x28,
                .tdt_window_timer_init  = 0xA0,
                .gpio = TEGRA_GPIO_PN4, // Replace with appropriate GPIO pin number 
};
///Eric end

static struct i2c_board_info ventana_i2c4_board_info[] = {
	{
		I2C_BOARD_INFO("nct1008", 0x4C),
		.irq = TEGRA_GPIO_TO_IRQ(NCT1008_THERM2_GPIO),
		.platform_data = &ventana_nct1008_pdata,
	},

//#ifdef CONFIG_SENSORS_AK8975
	{
		I2C_BOARD_INFO("akm8975", 0x0C),
		.irq = TEGRA_GPIO_TO_IRQ(AKM8975_IRQ_GPIO),
	},
	{ 
		I2C_BOARD_INFO("kxtf9", 0x0F),
              .platform_data = &kxtf9_pdata,
        },
//#endif
};

/* Compal Indigo-Carl begin */
static struct i2c_board_info ventana_i2c3_board_info[] = {
#ifdef CONFIG_VIDEO_MT9D115
	// I2C slave addr : 0x3D
	{
		I2C_BOARD_INFO("mt9d115", 0x3D),
		.platform_data = &yuv_sensor_data,
 	},
#endif
#ifdef CONFIG_VIDEO_MT9P111
	// I2C slave addr : 0x3C
	{
		I2C_BOARD_INFO("mt9p111", 0x3C),
		.platform_data = &yuv5_sensor_data,
	},
#endif
};

#if 0
static struct i2c_board_info ventana_i2c6_board_info[] = {
	{
		I2C_BOARD_INFO("ov5650R", 0x36),
		.platform_data = &ventana_right_ov5650_data,
	},
};

static struct i2c_board_info ventana_i2c7_board_info[] = {
	{
		I2C_BOARD_INFO("ov5650L", 0x36),
		.platform_data = &ventana_left_ov5650_data,
	},
	{
		I2C_BOARD_INFO("sh532u", 0x72),
	},
};

static struct i2c_board_info ventana_i2c8_board_info[] = {
	{
		I2C_BOARD_INFO("ov2710", 0x36),
		.platform_data = &ventana_ov2710_data,
	},
};
#endif
/* Compal Indigo-Carl end */

#ifdef CONFIG_MPU_SENSORS_MPU3050
#define SENSOR_MPU_NAME "mpu3050"
static struct mpu3050_platform_data mpu3050_data = {
	.int_config  = 0x10,
	.orientation = { 0, -1, 0, -1, 0, 0, 0, 0, -1 },  /* Orientation matrix for MPU on ventana */
	.level_shifter = 0,
	.accel = {
#ifdef CONFIG_MPU_SENSORS_KXTF9
	.get_slave_descr = get_accel_slave_descr,
#else
	.get_slave_descr = NULL,
#endif
	.adapt_num   = 0,
	.bus         = EXT_SLAVE_BUS_SECONDARY,
	.address     = 0x0F,
	.orientation = { 0, -1, 0, -1, 0, 0, 0, 0, -1 },  /* Orientation matrix for Kionix on ventana */
	},

	.compass = {
#ifdef CONFIG_MPU_SENSORS_AK8975
	.get_slave_descr = get_compass_slave_descr,
#else
	.get_slave_descr = NULL,
#endif
	.adapt_num   = 4,            /* bus number 4 on ventana */
	.bus         = EXT_SLAVE_BUS_PRIMARY,
	.address     = 0x0C,
	.orientation = { 1, 0, 0, 0, -1, 0, 0, 0, -1 },  /* Orientation matrix for AKM on ventana */
	},
};

static struct i2c_board_info __initdata mpu3050_i2c0_boardinfo[] = {
	{
		I2C_BOARD_INFO(SENSOR_MPU_NAME, 0x68),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PZ4),
		.platform_data = &mpu3050_data,
	},
};

static void ventana_mpuirq_init(void)
{
	pr_info("*** MPU START *** ventana_mpuirq_init...\n");
	tegra_gpio_enable(TEGRA_GPIO_PZ4);
	gpio_request(TEGRA_GPIO_PZ4, SENSOR_MPU_NAME);
	gpio_direction_input(TEGRA_GPIO_PZ4);
	pr_info("*** MPU END *** ventana_mpuirq_init...\n");
}
#endif

int __init ventana_sensors_init(void)
{
	struct board_info BoardInfo;

//Eric-0425 begin
//add light sensor driver
	ventana_al3000a_ls_init();
//#ifdef CONFIG_SENSORS_AK8975
	ventana_akm8975_init();
//#endif
	ventana_kxtf9_init();
//Eric-0425 end
#ifdef CONFIG_MPU_SENSORS_MPU3050
	ventana_mpuirq_init();
#endif
	ventana_camera_init();
	ventana_nct1008_init();

	i2c_register_board_info(0, ventana_i2c0_board_info,
		ARRAY_SIZE(ventana_i2c0_board_info));

//ec init
#if 0
	tegra_get_board_info(&BoardInfo);

	/*
	 * battery driver is supported on FAB.D boards and above only,
	 * since they have the necessary hardware rework
	 */
	if (BoardInfo.sku > 0) {
		ventana_bq20z75_init();
		i2c_register_board_info(2, ventana_i2c2_board_info,
			ARRAY_SIZE(ventana_i2c2_board_info));
	}
#else
	i2c_register_board_info(2, ventana_i2c2_board_info,
	                        ARRAY_SIZE(ventana_i2c2_board_info));

	ventana_ECBat_init();
#endif

    /* Compal Indigo-Carl begin */
	//i2c_register_board_info(3, ventana_i2c3_board_info_ssl3250a,
	//	ARRAY_SIZE(ventana_i2c3_board_info_ssl3250a));
	i2c_register_board_info(3, ventana_i2c3_board_info,
		ARRAY_SIZE(ventana_i2c3_board_info));
    /* Compal Indigo-Carl end */

	i2c_register_board_info(4, ventana_i2c4_board_info,
		ARRAY_SIZE(ventana_i2c4_board_info));

	//i2c_register_board_info(6, ventana_i2c6_board_info,
	//	ARRAY_SIZE(ventana_i2c6_board_info));

	//i2c_register_board_info(7, ventana_i2c7_board_info,
	//	ARRAY_SIZE(ventana_i2c7_board_info));

	//i2c_register_board_info(8, ventana_i2c8_board_info,
	//	ARRAY_SIZE(ventana_i2c8_board_info));


#ifdef CONFIG_MPU_SENSORS_MPU3050
	i2c_register_board_info(0, mpu3050_i2c0_boardinfo,
		ARRAY_SIZE(mpu3050_i2c0_boardinfo));
#endif

	return 0;
}

#if 0
#ifdef CONFIG_TEGRA_CAMERA

struct tegra_camera_gpios {
	const char *name;
	int gpio;
	int enabled;
};

#define TEGRA_CAMERA_GPIO(_name, _gpio, _enabled)		\
	{						\
		.name = _name,				\
		.gpio = _gpio,				\
		.enabled = _enabled,			\
	}

static struct tegra_camera_gpios ventana_camera_gpio_keys[] = {
	[0] = TEGRA_CAMERA_GPIO("en_avdd_csi", AVDD_DSI_CSI_ENB_GPIO, 1),
	[1] = TEGRA_CAMERA_GPIO("cam_i2c_mux_rst_lo", CAM_I2C_MUX_RST_GPIO, 1),

	[2] = TEGRA_CAMERA_GPIO("cam2_ldo_shdn_lo", CAM2_LDO_SHUTDN_L_GPIO, 0),
	[3] = TEGRA_CAMERA_GPIO("cam2_af_pwdn_lo", CAM2_AF_PWR_DN_L_GPIO, 0),
	[4] = TEGRA_CAMERA_GPIO("cam2_pwdn", CAM2_PWR_DN_GPIO, 0),
	[5] = TEGRA_CAMERA_GPIO("cam2_rst_lo", CAM2_RST_L_GPIO, 1),

	[6] = TEGRA_CAMERA_GPIO("cam3_ldo_shdn_lo", CAM3_LDO_SHUTDN_L_GPIO, 0),
	[7] = TEGRA_CAMERA_GPIO("cam3_af_pwdn_lo", CAM3_AF_PWR_DN_L_GPIO, 0),
	[8] = TEGRA_CAMERA_GPIO("cam3_pwdn", CAM3_PWR_DN_GPIO, 0),
	[9] = TEGRA_CAMERA_GPIO("cam3_rst_lo", CAM3_RST_L_GPIO, 1),

	[10] = TEGRA_CAMERA_GPIO("cam1_ldo_shdn_lo", CAM1_LDO_SHUTDN_L_GPIO, 0),
	[11] = TEGRA_CAMERA_GPIO("cam1_af_pwdn_lo", CAM1_AF_PWR_DN_L_GPIO, 0),
	[12] = TEGRA_CAMERA_GPIO("cam1_pwdn", CAM1_PWR_DN_GPIO, 0),
	[13] = TEGRA_CAMERA_GPIO("cam1_rst_lo", CAM1_RST_L_GPIO, 1),
};

int __init ventana_camera_late_init(void)
{
	int ret;
	int i;
	struct regulator *cam_ldo6 = NULL;

	if (!machine_is_ventana())
		return 0;

	cam_ldo6 = regulator_get(NULL, "vdd_ldo6");
	if (IS_ERR_OR_NULL(cam_ldo6)) {
		pr_err("%s: Couldn't get regulator ldo6\n", __func__);
		return PTR_ERR(cam_ldo6);
	}

	ret = regulator_set_voltage(cam_ldo6, 1800*1000, 1800*1000);
	if (ret){
		pr_err("%s: Failed to set ldo6 to 1.8v\n", __func__);
		goto fail_put_regulator;
	}

	ret = regulator_enable(cam_ldo6);
	if (ret){
		pr_err("%s: Failed to enable ldo6\n", __func__);
		goto fail_put_regulator;
	}

	i2c_new_device(i2c_get_adapter(3), ventana_i2c3_board_info_tca6416);

	for (i = 0; i < ARRAY_SIZE(ventana_camera_gpio_keys); i++) {
		ret = gpio_request(ventana_camera_gpio_keys[i].gpio,
			ventana_camera_gpio_keys[i].name);
		if (ret < 0) {
			pr_err("%s: gpio_request failed for gpio #%d\n",
				__func__, i);
			goto fail_free_gpio;
		}
		gpio_direction_output(ventana_camera_gpio_keys[i].gpio,
			ventana_camera_gpio_keys[i].enabled);
		gpio_export(ventana_camera_gpio_keys[i].gpio, false);
	}

	i2c_new_device(i2c_get_adapter(3), ventana_i2c3_board_info_pca9546);

	ventana_ov2710_power_off();
	ventana_left_ov5650_power_off();
	ventana_right_ov5650_power_off();

	ret = regulator_disable(cam_ldo6);
	if (ret){
		pr_err("%s: Failed to disable ldo6\n", __func__);
		goto fail_free_gpio;
	}

	regulator_put(cam_ldo6);
	return 0;

fail_free_gpio:
	while (i--)
		gpio_free(ventana_camera_gpio_keys[i].gpio);

fail_put_regulator:
	regulator_put(cam_ldo6);
	return ret;
}

late_initcall(ventana_camera_late_init);

#endif /* CONFIG_TEGRA_CAMERA */
#endif
