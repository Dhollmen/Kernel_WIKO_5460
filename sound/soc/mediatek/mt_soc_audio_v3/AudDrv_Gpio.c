/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudDrv_Gpio.c
 *
 * Project:
 * --------
 *   MT6735  Audio Driver GPIO
 *
 * Description:
 * ------------
 *   Audio register
 *
 * Author:
 * -------
 * George
 *
 *------------------------------------------------------------------------------
 *
 *
 *******************************************************************************/


/*****************************************************************************
 *                     C O M P I L E R   F L A G S
 *****************************************************************************/


/*****************************************************************************
 *                E X T E R N A L   R E F E R E N C E S
 *****************************************************************************/

#if !defined(CONFIG_MTK_LEGACY)
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#else
#include <mt-plat/mt_gpio.h>
#endif
#include "AudDrv_Gpio.h"

#if !defined(CONFIG_MTK_LEGACY)
struct pinctrl *pinctrlaud;

enum audio_system_gpio_type {
	GPIO_DEFAULT = 0,
	GPIO_PMIC_MODE0,
	GPIO_PMIC_MODE1,
	GPIO_I2S_MODE0,
	GPIO_I2S_MODE1,
	GPIO_EXTAMP_HIGH,
	GPIO_EXTAMP_LOW,
	GPIO_EXTAMP2_HIGH,
	GPIO_EXTAMP2_LOW,
	GPIO_RCVSPK_HIGH,
	GPIO_RCVSPK_LOW,
	GPIO_NUM
};


struct audio_gpio_attr {
	const char *name;
	bool gpio_prepare;
	struct pinctrl_state *gpioctrl;
};

static struct audio_gpio_attr aud_gpios[GPIO_NUM] = {
	[GPIO_DEFAULT] = {"default", false, NULL},
	[GPIO_PMIC_MODE0] = {"audpmicclk-mode0", false, NULL},
	[GPIO_PMIC_MODE1] = {"audpmicclk-mode1", false, NULL},
	[GPIO_I2S_MODE0] = {"audi2s1-mode0", false, NULL},
	[GPIO_I2S_MODE1] = {"audi2s1-mode1", false, NULL},
	[GPIO_EXTAMP_HIGH] = {"extamp-pullhigh", false, NULL},
	[GPIO_EXTAMP_LOW] = {"extamp-pulllow", false, NULL},
	[GPIO_EXTAMP2_HIGH] = {"extamp2-pullhigh", false, NULL},
	[GPIO_EXTAMP2_LOW] = {"extamp2-pulllow", false, NULL},
	[GPIO_RCVSPK_HIGH] = {"rcvspk-pullhigh", false, NULL},
	[GPIO_RCVSPK_LOW] = {"rcvspk-pulllow", false, NULL},
};


void AudDrv_GPIO_probe(void *dev)
{
	int ret;
	int i = 0;

	pinctrlaud = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrlaud)) {
		ret = PTR_ERR(pinctrlaud);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(aud_gpios); i++) {
		aud_gpios[i].gpioctrl = pinctrl_lookup_state(pinctrlaud, aud_gpios[i].name);
		if (IS_ERR(aud_gpios[i].gpioctrl)) {
			ret = PTR_ERR(aud_gpios[i].gpioctrl);
		} else {
			aud_gpios[i].gpio_prepare = true;
		}
	}

}

int AudDrv_GPIO_PMIC_Select(int bEnable)
{
	int retval = 0;

	if (bEnable == 1) {
		if (aud_gpios[GPIO_PMIC_MODE1].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_PMIC_MODE1].gpioctrl);
		}
	} else {
		if (aud_gpios[GPIO_PMIC_MODE0].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_PMIC_MODE0].gpioctrl);
		}

	}
	return retval;
}

int AudDrv_GPIO_I2S_Select(int bEnable)
{
	int retval = 0;

	if (bEnable == 1) {
		if (aud_gpios[GPIO_I2S_MODE1].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_I2S_MODE1].gpioctrl);
		}
	} else {
		if (aud_gpios[GPIO_I2S_MODE0].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_I2S_MODE0].gpioctrl);
		}

	}
	return retval;
}

int AudDrv_GPIO_EXTAMP_Select(int bEnable)
{
	int retval = 0;

	
	if (bEnable == 1) {
		if (aud_gpios[GPIO_EXTAMP_HIGH].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_EXTAMP_HIGH].gpioctrl);
		}
	} else {
		if (aud_gpios[GPIO_EXTAMP_LOW].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_EXTAMP_LOW].gpioctrl);
		}

	}
	return retval;
}

int AudDrv_GPIO_EXTAMP2_Select(int bEnable)
{
	int retval = 0;

	if (bEnable == 1) {
		if (aud_gpios[GPIO_EXTAMP2_HIGH].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_EXTAMP2_HIGH].gpioctrl);
		}
	} else {
		if (aud_gpios[GPIO_EXTAMP2_LOW].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_EXTAMP2_LOW].gpioctrl);
		}

	}
	return retval;
}

int AudDrv_GPIO_RCVSPK_Select(int bEnable)
{
	int retval = 0;

	if (bEnable == 1) {
		if (aud_gpios[GPIO_RCVSPK_HIGH].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_RCVSPK_HIGH].gpioctrl);
		}
	} else {
		if (aud_gpios[GPIO_RCVSPK_LOW].gpio_prepare) {
			retval =
			    pinctrl_select_state(pinctrlaud, aud_gpios[GPIO_RCVSPK_LOW].gpioctrl);
		}

	}
	return retval;
}

#endif
