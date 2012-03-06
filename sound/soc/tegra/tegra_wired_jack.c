/*
 * sound/soc/tegra/tegra_wired_jack.c
 *
 * Copyright (c) 2011, NVIDIA Corporation.
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

#include <linux/types.h>
#include <linux/gpio.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#include <linux/notifier.h>
#include <sound/jack.h>
#include <sound/soc.h>
#include <mach/audio.h>

#include "tegra_soc.h"
#include "../codecs/wm8903.h"
#include <sound/wm8903.h>
/* willy 0512 begin*/
/* codec register values */
#define B00_IN_VOL		0
#define B00_INR_ENA		0
#define B01_INL_ENA		1
#define B01_MICDET_ENA		1
#define B00_MICBIAS_ENA		0
#define B15_DRC_ENA		15
#define B01_ADCL_ENA		1
#define B00_ADCR_ENA		0
#define B06_IN_CM_ENA		6
#define B04_IP_SEL_N		4
#define B02_IP_SEL_P		2
#define B00_MODE 		0
#define B06_AIF_ADCL		7
#define B06_AIF_ADCR		6
#define B04_ADC_HPF_ENA		4
#define R20_SIDETONE_CTRL	32
#define R29_DRC_1		41
#define SET_REG_VAL(r,m,l,v) (((r)&(~((m)<<(l))))|(((v)&(m))<<(l)))
/* willy 0512 end*/

#define HEAD_DET_GPIO 0
#define MIC_DET_GPIO  1
#define SPK_EN_GPIO   3
/* willy 0512 begin*/
/* polling external mic */
struct snd_soc_codec* g_codec;
/* willy 0512 end*/
struct wired_jack_conf tegra_wired_jack_conf = {
	-1, -1, -1, -1, 0, NULL, NULL
};

/* willy 0516 begin*/
/* Based on hp_gpio and mic_gpio, hp_gpio is active low */
enum {
	ERROR_0 = 				0x0,
	ERROR_1 = 				0x1,
	DOCKING_MIC_HP = 		0x2,
	DOCKING_MIC = 			0x3,
	HEADSET_WITHOUT_MIC = 	0x4,
	NO_DEVICE = 			0x5,
	HEADSET_WITH_MIC = 		0x6,
	ERROR_3 = 				0x7,
};
/* willy 0516 end*/

/* These values are copied from WiredAccessoryObserver */
enum headset_state {
	BIT_NO_HEADSET = 0,
	BIT_HEADSET = (1 << 0),
	BIT_HEADSET_NO_MIC = (1 << 1),
/* willy 0513 begin*/
/* polling external mic */
	BIT_DOCKING_MIC = (1 << 2),
	BIT_DOCKING_MIC_HP = (1 << 3),
/* willy 0513 end*/
};

/* jack */
static struct snd_soc_jack *tegra_wired_jack;
/* willy 0512 begin*/
/* polling external mic */
int jack_state;
/* willy 0512 end*/
static struct snd_soc_jack_gpio wired_jack_gpios[] = {
	{
		/* gpio pin depends on board traits */
		.name = "headphone-detect-gpio",
		.report = SND_JACK_HEADPHONE,
		.invert = 1,
		.debounce_time = 200,
	},
	{
		/* gpio pin depens on board traits */
		.name = "mic-detect-gpio",
		.report = SND_JACK_MICROPHONE,
		.invert = 1,
		.debounce_time = 200,
	},
};

/* willy 0512 begin*/
/* polling external mic */
static int wired_jack_detect(void)
{
	int i;
	int withMic = 0;
	int withoutMic = 0;
	int MICDET_EINT_14 = 0;
	int MICSHRT_EINT_15 = 0;
	int irqStatus;
	int irq_mask = 0x3fff;
	int all_mask = 0xffff;
	int CtrlReg = 0;
/* compal indigo-Howard Chang 20110520 begin */
/* fix the issue that device can't switch path from internal MIC to cradle MIC on recording mode*/
	int backup_micbias_register;
/* compal indigo-Howard Chang 20110518 end */
	
	CtrlReg = snd_soc_read(g_codec, WM8903_VMID_CONTROL_0);
	CtrlReg &= ~(WM8903_VMID_RES_MASK);
	CtrlReg |= WM8903_VMID_RES_50K;
	snd_soc_write(g_codec, WM8903_VMID_CONTROL_0, CtrlReg);
	snd_soc_write(g_codec, WM8903_BIAS_CONTROL_0, 0xB);
	CtrlReg = snd_soc_read(g_codec, WM8903_CLOCK_RATES_2);
	CtrlReg |= (WM8903_CLK_DSP_ENA | WM8903_CLK_SYS_ENA | WM8903_TO_ENA);
	snd_soc_write(g_codec, WM8903_CLOCK_RATES_2, CtrlReg);

	snd_soc_write(g_codec, WM8903_INTERRUPT_STATUS_1_MASK, irq_mask);
/* compal indigo-Howard Chang 20110520 begin */
/* fix the issue that device can't switch path from internal MIC to cradle MIC on recording mode*/
	backup_micbias_register = snd_soc_read(g_codec, WM8903_MIC_BIAS_CONTROL_0);
/* compal indigo-Howard Chang 20110518 end */
	snd_soc_write(g_codec, WM8903_MIC_BIAS_CONTROL_0, WM8903_MICDET_ENA | WM8903_MICBIAS_ENA);

	/* debounce */
	msleep(100);


	for(i = 0; i <= 15; i++)
	{
		msleep(1);
		irqStatus = snd_soc_read(g_codec, WM8903_INTERRUPT_STATUS_1);

		MICDET_EINT_14 = (irqStatus >> 14) & 0x1;
		MICSHRT_EINT_15 = (irqStatus >> 15) & 0x1;

		if(MICDET_EINT_14 == MICSHRT_EINT_15)
			withoutMic++;
		else
			withMic++;

		if(i%2 == 0)
			snd_soc_write(g_codec, WM8903_INTERRUPT_POLARITY_1, irq_mask);
		else
			snd_soc_write(g_codec, WM8903_INTERRUPT_POLARITY_1, all_mask);
	}
/* compal indigo-Howard Chang 20110520 begin */
/* fix the issue that device can't switch path from internal MIC to cradle MIC on recording mode*/
/*
	CtrlReg &= ~(WM8903_MICDET_ENA | WM8903_MICBIAS_ENA);
	snd_soc_write(g_codec, WM8903_MIC_BIAS_CONTROL_0, CtrlReg);
*/
	snd_soc_write(g_codec, WM8903_MIC_BIAS_CONTROL_0, backup_micbias_register);
/* compal indigo-Howard Chang 20110518 end */

	if (withMic > withoutMic)
		return 1;
	else
		return 0;
}
/* willy 0512 end*/

#ifdef CONFIG_SWITCH
static struct switch_dev wired_switch_dev = {
	.name = "h2w",
};

void tegra_switch_set_state(int state)
{
	switch_set_state(&wired_switch_dev, state);
}

/* willy 0512 begin*/
/* for hot plug when recording */
static void select_mic_input(int state)
{
	int CtrlReg = 0,VolumeCtrlReg = 0;

	switch (state) {
		case BIT_HEADSET:
		{
			printk("\nRecord device : headset mic\n");
/* compal indigo-Howard Chang 20110520 begin */
/* fix the issue that device can't switch path from internal MIC to cradle MIC on recording mode*/
			snd_soc_write(g_codec, WM8903_AUDIO_INTERFACE_0,0x10);
/* compal indigo-Howard Chang 20110518 end */
			CtrlReg = (0x0<<B06_IN_CM_ENA) |
				(0x1<<B04_IP_SEL_N) | (0x0<<B02_IP_SEL_P) | (0x0<<B00_MODE);
			VolumeCtrlReg = (0x1C << B00_IN_VOL);	
/* kenny 0711 begin*/
/* mic gain tunning */                 
            snd_soc_write(g_codec, WM8903_ADC_DIGITAL_VOLUME_LEFT, 0x1c0);
            snd_soc_write(g_codec, WM8903_ADC_DIGITAL_VOLUME_RIGHT, 0x1c0);                    
/* kenny 0711 end*/  						
		}
		break;
		
		case BIT_NO_HEADSET:
		case BIT_HEADSET_NO_MIC:
		{
			printk("\nRecord device : int mic\n");
/* compal indigo-Howard Chang 20110518 begin */
			/* fix internal mic function */
			/*
			CtrlReg = (0x1<<B06_IN_CM_ENA) |
				(0x1<<B04_IP_SEL_N) | (0x0<<B02_IP_SEL_P) | (0x1<<B00_MODE);
			*/
			snd_soc_write(g_codec, WM8903_AUDIO_INTERFACE_0,0xD0);
			/* kenny 20110722 begin*/
			//For meet spec setting			
			CtrlReg = 0x52;
			/* kenny 20110722 end*/
/* compal indigo-Howard Chang 20110518 end */
			VolumeCtrlReg = (0x07 << B00_IN_VOL);
/* kenny 0711 begin*/
/* mic gain tunning */                 
            snd_soc_write(g_codec, WM8903_ADC_DIGITAL_VOLUME_LEFT, 0x1e6);
            snd_soc_write(g_codec, WM8903_ADC_DIGITAL_VOLUME_RIGHT, 0x1e6);                    
/* kenny 0711 end*/  			
			
		}
		break;
		
		case BIT_DOCKING_MIC:
		case BIT_DOCKING_MIC_HP:
		{
			printk("\nRecord device : docking mic\n");
/* compal indigo-Howard Chang 20110520 begin */
			/* fix the issue that device can't switch path from internal MIC to cradle MIC on recording mode*/
			snd_soc_write(g_codec, WM8903_AUDIO_INTERFACE_0,0x10);
/* compal indigo-Howard Chang 20110518 end */
			CtrlReg = (0x0<<B06_IN_CM_ENA) |
				(0x2<<B04_IP_SEL_N) | (0x2<<B02_IP_SEL_P) | (0x0<<B00_MODE);
			VolumeCtrlReg = (0x1C << B00_IN_VOL);
/* kenny 0711 begin*/
/* mic gain tunning */                 
            snd_soc_write(g_codec, WM8903_ADC_DIGITAL_VOLUME_LEFT, 0x1c0);
            snd_soc_write(g_codec, WM8903_ADC_DIGITAL_VOLUME_RIGHT, 0x1c0);                    
/* kenny 0711 end*/  			
		}
		break;
	}
	
	snd_soc_write(g_codec, WM8903_ANALOGUE_LEFT_INPUT_0,VolumeCtrlReg);
	snd_soc_write(g_codec, WM8903_ANALOGUE_RIGHT_INPUT_0,VolumeCtrlReg);	
	snd_soc_write(g_codec, WM8903_ANALOGUE_LEFT_INPUT_1, CtrlReg);
	snd_soc_write(g_codec, WM8903_ANALOGUE_RIGHT_INPUT_1, CtrlReg);
}
/* willy 0512 end*/

static enum headset_state get_headset_state(void)
{
	enum headset_state state = BIT_NO_HEADSET;
	int flag = 0;
	int hp_gpio = -1;
	int mic_gpio = -1;
/* willy 0513 begin*/
	int docking_mic_gpio = -1;
	/* en_mic_ext is low active pin */
	/* mic_gpio is high active */
	if (tegra_wired_jack_conf.en_mic_ext != -1){
		docking_mic_gpio = gpio_get_value(tegra_wired_jack_conf.en_mic_ext);
		mic_gpio = wired_jack_detect();
	}
/* willy 0513 end*/

/* willy 0512 begin*/
/* detect mic when plug in/out headphone */
#if 1
	/* hp_det_n is low active pin */
	if (tegra_wired_jack_conf.hp_det_n != -1){
		hp_gpio = gpio_get_value(tegra_wired_jack_conf.hp_det_n);
	}
#else
	/* hp_det_n is low active pin */
	if (tegra_wired_jack_conf.hp_det_n != -1)
		hp_gpio = gpio_get_value(tegra_wired_jack_conf.hp_det_n);
	if (tegra_wired_jack_conf.cdc_irq != -1)
		mic_gpio = gpio_get_value(tegra_wired_jack_conf.cdc_irq);
#endif
/* willy 0512 end*/

	pr_debug("hp_gpio:%d, mic_gpio:%d\n", hp_gpio, mic_gpio);
	
/* willy 0516 begin*/
/* for docking behavior */
	flag = (docking_mic_gpio << 2) | (mic_gpio << 1) | (hp_gpio << 0);

	switch (flag) {
		
	case ERROR_0:
	case ERROR_1:
	case ERROR_3:
		state = BIT_NO_HEADSET;
		break;
	case DOCKING_MIC_HP:
		state = BIT_HEADSET_NO_MIC;
		jack_state = BIT_DOCKING_MIC_HP;
		break;
	case DOCKING_MIC:
		state = BIT_NO_HEADSET;
		jack_state = BIT_DOCKING_MIC;
		break;
	case HEADSET_WITHOUT_MIC:
		state = BIT_HEADSET_NO_MIC;
		jack_state = BIT_HEADSET_NO_MIC;
		break;
	case NO_DEVICE:
		state = BIT_NO_HEADSET;
		jack_state = BIT_NO_HEADSET;
		break;
	case HEADSET_WITH_MIC:
		state = BIT_HEADSET;
		jack_state = BIT_HEADSET;
		break;
	default:
		state = BIT_NO_HEADSET;
		jack_state = BIT_NO_HEADSET;
	}
/* willy 0516 end */	

/* willy 0512 begin*/
/* set record path */	
	select_mic_input(jack_state);
/* willy 0512 end*/	
	return state;
}

static int wired_switch_notify(struct notifier_block *self,
			      unsigned long action, void* dev)
{
	tegra_switch_set_state(get_headset_state());

	return NOTIFY_OK;
}

/* willy 0512 begin*/
/* set record path */
int wired_jack_state(void)
{
	return jack_state;
}
/* willy 0512 end*/

void tegra_jack_resume(void)
{
	tegra_switch_set_state(get_headset_state());
}

static struct notifier_block wired_switch_nb = {
	.notifier_call = wired_switch_notify,
};
#endif

/* willy 0512 begin*/
/* modify jack det gpio */
#if 1
/* platform driver */
static int tegra_wired_jack_probe(struct platform_device *pdev)
{
	int ret;
	int hp_det_n;
	int en_mic_ext;
	int en_spkr;
	struct tegra_wired_jack_conf *pdata;

	pdata = (struct tegra_wired_jack_conf *)pdev->dev.platform_data;

	if (!pdata || !pdata->hp_det_n || !pdata->en_spkr ||
	    !pdata->en_mic_ext) {
		pr_err("Please set up gpio pins for jack.\n");
		return -EBUSY;
	}

	hp_det_n = pdata->hp_det_n;
	wired_jack_gpios[HEAD_DET_GPIO].gpio = hp_det_n;
	
	en_mic_ext = pdata->en_mic_ext;
	wired_jack_gpios[MIC_DET_GPIO].gpio = en_mic_ext;

	ret = snd_soc_jack_add_gpios(tegra_wired_jack,
				     ARRAY_SIZE(wired_jack_gpios),
				     wired_jack_gpios);
	if (ret) {
		pr_err("Could NOT set up gpio pins for jack.\n");
		snd_soc_jack_free_gpios(tegra_wired_jack,
					ARRAY_SIZE(wired_jack_gpios),
					wired_jack_gpios);
		return ret;
	}

	en_spkr = pdata->en_spkr;
	ret = gpio_request(en_spkr, "en_spkr");
	if (ret) {
		pr_err("Could NOT set up gpio pin for amplifier.\n");
		gpio_free(en_spkr);
	}
	gpio_direction_output(en_spkr, 0);
	gpio_export(en_spkr, false);

	if (pdata->spkr_amp_reg)
		tegra_wired_jack_conf.amp_reg =
			regulator_get(NULL, pdata->spkr_amp_reg);
	tegra_wired_jack_conf.amp_reg_enabled = 0;

	/* restore configuration of these pins */
	tegra_wired_jack_conf.hp_det_n = hp_det_n;
	tegra_wired_jack_conf.en_mic_ext = en_mic_ext;
	tegra_wired_jack_conf.en_spkr = en_spkr;

	// Communicate the jack connection state at device bootup
	tegra_switch_set_state(get_headset_state());
	
#ifdef CONFIG_SWITCH
	snd_soc_jack_notifier_register(tegra_wired_jack,
				       &wired_switch_nb);
#endif
	return ret;
}
#else
/* platform driver */
static int tegra_wired_jack_probe(struct platform_device *pdev)
{
	int ret;
	int hp_det_n, cdc_irq;
	int en_mic_int, en_mic_ext;
	int en_spkr;
	struct tegra_wired_jack_conf *pdata;

	pdata = (struct tegra_wired_jack_conf *)pdev->dev.platform_data;

	if (!pdata || !pdata->hp_det_n || !pdata->en_spkr ||
	    !pdata->cdc_irq || !pdata->en_mic_int || !pdata->en_mic_ext) {
		pr_err("Please set up gpio pins for jack.\n");
		return -EBUSY;
	}

	hp_det_n = pdata->hp_det_n;
	wired_jack_gpios[HEAD_DET_GPIO].gpio = hp_det_n;

	cdc_irq = pdata->cdc_irq;
	wired_jack_gpios[MIC_DET_GPIO].gpio = cdc_irq;

	ret = snd_soc_jack_add_gpios(tegra_wired_jack,
				     ARRAY_SIZE(wired_jack_gpios),
				     wired_jack_gpios);
	if (ret) {
		pr_err("Could NOT set up gpio pins for jack.\n");
		snd_soc_jack_free_gpios(tegra_wired_jack,
					ARRAY_SIZE(wired_jack_gpios),
					wired_jack_gpios);
		return ret;
	}

	/* Mic switch controlling pins */
	en_mic_int = pdata->en_mic_int;
	en_mic_ext = pdata->en_mic_ext;

	ret = gpio_request(en_mic_int, "en_mic_int");
	if (ret) {
		pr_err("Could NOT get gpio for internal mic controlling.\n");
		gpio_free(en_mic_int);
	}
	gpio_direction_output(en_mic_int, 0);
	gpio_export(en_mic_int, false);

	ret = gpio_request(en_mic_ext, "en_mic_ext");
	if (ret) {
		pr_err("Could NOT get gpio for external mic controlling.\n");
		gpio_free(en_mic_ext);
	}
	gpio_direction_output(en_mic_ext, 0);
	gpio_export(en_mic_ext, false);

	en_spkr = pdata->en_spkr;
	ret = gpio_request(en_spkr, "en_spkr");
	if (ret) {
		pr_err("Could NOT set up gpio pin for amplifier.\n");
		gpio_free(en_spkr);
	}
	gpio_direction_output(en_spkr, 0);
	gpio_export(en_spkr, false);

	if (pdata->spkr_amp_reg)
		tegra_wired_jack_conf.amp_reg =
			regulator_get(NULL, pdata->spkr_amp_reg);
	tegra_wired_jack_conf.amp_reg_enabled = 0;

	/* restore configuration of these pins */
	tegra_wired_jack_conf.hp_det_n = hp_det_n;
	tegra_wired_jack_conf.en_mic_int = en_mic_int;
	tegra_wired_jack_conf.en_mic_ext = en_mic_ext;
	tegra_wired_jack_conf.cdc_irq = cdc_irq;
	tegra_wired_jack_conf.en_spkr = en_spkr;

	// Communicate the jack connection state at device bootup
	tegra_switch_set_state(get_headset_state());

#ifdef CONFIG_SWITCH
	snd_soc_jack_notifier_register(tegra_wired_jack,
				       &wired_switch_nb);
#endif
	return ret;
}
#endif
/* willy 0512 end*/

static int tegra_wired_jack_remove(struct platform_device *pdev)
{
	snd_soc_jack_free_gpios(tegra_wired_jack,
				ARRAY_SIZE(wired_jack_gpios),
				wired_jack_gpios);

	gpio_free(tegra_wired_jack_conf.en_mic_int);
	gpio_free(tegra_wired_jack_conf.en_mic_ext);
	gpio_free(tegra_wired_jack_conf.en_spkr);

	if (tegra_wired_jack_conf.amp_reg) {
		if (tegra_wired_jack_conf.amp_reg_enabled)
			regulator_disable(tegra_wired_jack_conf.amp_reg);
		regulator_put(tegra_wired_jack_conf.amp_reg);
	}

	return 0;
}

static struct platform_driver tegra_wired_jack_driver = {
	.probe = tegra_wired_jack_probe,
	.remove = tegra_wired_jack_remove,
	.driver = {
		.name = "tegra_wired_jack",
		.owner = THIS_MODULE,
	},
};


int tegra_jack_init(struct snd_soc_codec *codec)
{
	int ret;

	if (!codec)
		return -1;
/* willy 0512 begin */
	g_codec = codec;
/* willy 0512 end */

	tegra_wired_jack = kzalloc(sizeof(*tegra_wired_jack), GFP_KERNEL);
	if (!tegra_wired_jack) {
		pr_err("failed to allocate tegra_wired_jack \n");
		return -ENOMEM;
	}

	/* Add jack detection */
	ret = snd_soc_jack_new(codec->socdev->card, "Wired Accessory Jack",
			       SND_JACK_HEADSET, tegra_wired_jack);
	if (ret < 0)
		goto failed;

#ifdef CONFIG_SWITCH
	/* Addd h2w swith class support */
	ret = switch_dev_register(&wired_switch_dev);
	if (ret < 0)
		goto switch_dev_failed;
#endif

	ret = platform_driver_register(&tegra_wired_jack_driver);
	if (ret < 0)
		goto platform_dev_failed;

	return 0;

#ifdef CONFIG_SWITCH
switch_dev_failed:
	switch_dev_unregister(&wired_switch_dev);
#endif
platform_dev_failed:
	platform_driver_unregister(&tegra_wired_jack_driver);
failed:
	if (tegra_wired_jack) {
		kfree(tegra_wired_jack);
		tegra_wired_jack = 0;
	}
	return ret;
}

void tegra_jack_exit(void)
{
#ifdef CONFIG_SWITCH
	switch_dev_unregister(&wired_switch_dev);
#endif
	platform_driver_unregister(&tegra_wired_jack_driver);

	if (tegra_wired_jack) {
		kfree(tegra_wired_jack);
		tegra_wired_jack = 0;
	}
}
