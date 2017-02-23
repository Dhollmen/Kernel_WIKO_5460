
#include <linux/delay.h>
#include <asm/div64.h>

#include <mt-plat/upmu_common.h>

#include <mt-plat/battery_meter_hal.h>
#include "cust_battery_meter.h"

#include <mach/mt_pmic.h>
#include <mt-plat/battery_meter.h>


/* ============================================================ // */
/* define */
/* ============================================================ // */
#define STATUS_OK    0
#define STATUS_UNSUPPORTED    -1
#define VOLTAGE_FULL_RANGE    1800
#define ADC_PRECISE           32768	/* 12 bits */

#define UNIT_FGCURRENT     (158122)	/* 158.122 uA */

/* ============================================================ // */
/* global variable */
/* ============================================================ // */
signed int chip_diff_trim_value_4_0 = 0;
signed int chip_diff_trim_value = 0;	/* unit = 0.1 */

signed int g_hw_ocv_tune_value = 0;

kal_bool g_fg_is_charging = 0;

void get_hw_chip_diff_trim_value(void)
{
#if defined(CONFIG_POWER_EXT)
#else

#if 1
	signed int reg_val = 0;

	reg_val = upmu_get_reg_value(0xCB8);
	chip_diff_trim_value_4_0 = (reg_val >> 7) & 0x001F;	/* chip_diff_trim_value_4_0 = (reg_val>>10)&0x001F; */
#endif

	switch (chip_diff_trim_value_4_0) {
	case 0:
		chip_diff_trim_value = 1000;
		break;
	case 1:
		chip_diff_trim_value = 1005;
		break;
	case 2:
		chip_diff_trim_value = 1010;
		break;
	case 3:
		chip_diff_trim_value = 1015;
		break;
	case 4:
		chip_diff_trim_value = 1020;
		break;
	case 5:
		chip_diff_trim_value = 1025;
		break;
	case 6:
		chip_diff_trim_value = 1030;
		break;
	case 7:
		chip_diff_trim_value = 1036;
		break;
	case 8:
		chip_diff_trim_value = 1041;
		break;
	case 9:
		chip_diff_trim_value = 1047;
		break;
	case 10:
		chip_diff_trim_value = 1052;
		break;
	case 11:
		chip_diff_trim_value = 1058;
		break;
	case 12:
		chip_diff_trim_value = 1063;
		break;
	case 13:
		chip_diff_trim_value = 1069;
		break;
	case 14:
		chip_diff_trim_value = 1075;
		break;
	case 15:
		chip_diff_trim_value = 1081;
		break;
	case 31:
		chip_diff_trim_value = 995;
		break;
	case 30:
		chip_diff_trim_value = 990;
		break;
	case 29:
		chip_diff_trim_value = 985;
		break;
	case 28:
		chip_diff_trim_value = 980;
		break;
	case 27:
		chip_diff_trim_value = 975;
		break;
	case 26:
		chip_diff_trim_value = 970;
		break;
	case 25:
		chip_diff_trim_value = 966;
		break;
	case 24:
		chip_diff_trim_value = 961;
		break;
	case 23:
		chip_diff_trim_value = 956;
		break;
	case 22:
		chip_diff_trim_value = 952;
		break;
	case 21:
		chip_diff_trim_value = 947;
		break;
	case 20:
		chip_diff_trim_value = 943;
		break;
	case 19:
		chip_diff_trim_value = 938;
		break;
	case 18:
		chip_diff_trim_value = 934;
		break;
	case 17:
		chip_diff_trim_value = 930;
		break;
	default:
		break;
	}

#endif
}

signed int use_chip_trim_value(signed int not_trim_val)
{
#if defined(CONFIG_POWER_EXT)
	return not_trim_val;
#else
	signed int ret_val = 0;

	ret_val = ((not_trim_val * chip_diff_trim_value) / 1000);

	return ret_val;
#endif
}

int get_hw_ocv(void)
{
#if defined(CONFIG_POWER_EXT)
	return 4001;
#else
	signed int adc_result_reg = 0;
	signed int adc_result = 0;
	signed int r_val_temp = 3;	/* MT6325 use 2, old chip use 4 */

#if defined(SWCHR_POWER_PATH)
	adc_result_reg = pmic_get_register_value(PMIC_AUXADC_ADC_OUT_WAKEUP_SWCHR);
	/* mt6325_upmu_get_rg_adc_out_wakeup_swchr(); */
	adc_result = (adc_result_reg * r_val_temp * VOLTAGE_FULL_RANGE) / ADC_PRECISE;
#else
	adc_result_reg = pmic_get_register_value(PMIC_AUXADC_ADC_OUT_WAKEUP_PCHR);
/* mt6325_upmu_get_rg_adc_out_wakeup_pchr(); */
	adc_result = (adc_result_reg * r_val_temp * VOLTAGE_FULL_RANGE) / ADC_PRECISE;
#endif
	adc_result += g_hw_ocv_tune_value;
	return adc_result;
#endif
}


/* ============================================================// */

#if defined(CONFIG_POWER_EXT)
/*  */
#else
static unsigned int fg_get_data_ready_status(void)
{
	unsigned int ret = 0;
	unsigned int temp_val = 0;

	ret = pmic_read_interface(MT6328_FGADC_CON0, &temp_val, 0xFFFF, 0x0);

	temp_val =
	    (temp_val & (MT6328_PMIC_FG_LATCHDATA_ST_MASK << MT6328_PMIC_FG_LATCHDATA_ST_SHIFT)) >>
	    MT6328_PMIC_FG_LATCHDATA_ST_SHIFT;

	return temp_val;
}
#endif

static signed int fgauge_read_current(void *data);
static signed int fgauge_initialization(void *data)
{
#if defined(CONFIG_POWER_EXT)
/*  */
#else
	unsigned int ret = 0;
	signed int current_temp = 0;
	int m = 0;

	get_hw_chip_diff_trim_value();

    /* HW initialization */
	pmic_set_register_value(PMIC_RG_FGADC_ANA_CK_PDN, 0);	/* mt6325_upmu_set_rg_fgadc_ana_ck_pdn(0); */
	pmic_set_register_value(PMIC_RG_FGADC_DIG_CK_PDN, 0);	/* mt6325_upmu_set_rg_fgadc_dig_ck_pdn(0); */

	/* Set current mode, auto-calibration mode and 32KHz clock source */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0028, 0x00FF, 0x0);

    /* Enable FGADC */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0029, 0x00FF, 0x0);

	/* Reset HW FG */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x7100, 0xFF00, 0x0);

	/* Set FG_OSR */
	ret = pmic_config_interface(MT6328_FGADC_CON11, 0x8, 0xF, 0x0);

	/* make sure init finish */
	m = 0;
	while (current_temp == 0) {
		fgauge_read_current(&current_temp);
		m++;
		if (m > 1000)
			break;
	}

#endif

	return STATUS_OK;
}

static signed int fgauge_read_current(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 0;
#else
	unsigned short uvalue16 = 0;
	signed int dvalue = 0;
	int m = 0;
	long long Temp_Value = 0;
	signed int Current_Compensate_Value = 0;
	unsigned int ret = 0;

    /* HW Init */
    /* Read HW Raw Data */
	/* Set READ command */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0200, 0xFF00, 0x0);
	/* Keep i2c read when status = 1 (0x06) */
	m = 0;
	while (fg_get_data_ready_status() == 0) {
		m++;
		if (m > 1000)
			break;
	}
	/* Read FG_CURRENT_OUT[07:00] */
	uvalue16 = pmic_get_register_value(PMIC_FG_CURRENT_OUT);	/* mt6325_upmu_get_fg_current_out(); */
	/* Clear status to 0 */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0800, 0xFF00, 0x0);
	/* while ( fg_get_sw_clear_status() != 0 ) */
	m = 0;
	while (fg_get_data_ready_status() != 0) {
		m++;
		if (m > 1000)
			break;
	}
	/* Recover original settings */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0000, 0xFF00, 0x0);

    /* calculate the real world data */
	dvalue = (unsigned int) uvalue16;
	if (dvalue == 0) {
		Temp_Value = (long long) dvalue;
		g_fg_is_charging = KAL_FALSE;
	} else if (dvalue > 32767) {/* > 0x8000 */

		Temp_Value = (long long) (dvalue - 65535);
		Temp_Value = Temp_Value - (Temp_Value * 2);
		g_fg_is_charging = KAL_FALSE;
	} else {
		Temp_Value = (long long) dvalue;
		g_fg_is_charging = KAL_TRUE;
	}

	Temp_Value = Temp_Value * UNIT_FGCURRENT;
	do_div(Temp_Value, 100000);
	dvalue = (unsigned int) Temp_Value;

    /* Auto adjust value */
	if (batt_meter_cust_data.r_fg_value != 20) {
		dvalue = (dvalue * 20) / batt_meter_cust_data.r_fg_value;
	}
    /* K current */
	if (batt_meter_cust_data.r_fg_board_slope != batt_meter_cust_data.r_fg_board_base) {
		dvalue =
		    ((dvalue * batt_meter_cust_data.r_fg_board_base) +
		     (batt_meter_cust_data.r_fg_board_slope / 2)) /
		    batt_meter_cust_data.r_fg_board_slope;
	}
    /* current compensate */
	if (g_fg_is_charging == KAL_TRUE)
		dvalue = dvalue + Current_Compensate_Value;
	else
		dvalue = dvalue - Current_Compensate_Value;

	dvalue = ((dvalue * batt_meter_cust_data.car_tune_value) / 100);

	dvalue = use_chip_trim_value(dvalue);

	*(signed int *) (data) = dvalue;
#endif

	return STATUS_OK;
}


signed int fgauge_read_IM_current(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 0;
#else
	unsigned short uvalue16 = 0;
	signed int dvalue = 0;
	/*int m = 0;*/
	long long Temp_Value = 0;
	signed int Current_Compensate_Value = 0;
	/*unsigned int ret = 0;*/

	uvalue16 = pmic_get_register_value(PMIC_FG_R_CURR);

    /* calculate the real world data */
	dvalue = (unsigned int) uvalue16;
	if (dvalue == 0) {
		Temp_Value = (long long) dvalue;
		g_fg_is_charging = KAL_FALSE;
	} else if (dvalue > 32767) {/* > 0x8000 */

		Temp_Value = (long long) (dvalue - 65535);
		Temp_Value = Temp_Value - (Temp_Value * 2);
		g_fg_is_charging = KAL_FALSE;
	} else {
		Temp_Value = (long long) dvalue;
		g_fg_is_charging = KAL_TRUE;
	}

	Temp_Value = Temp_Value * UNIT_FGCURRENT;
	do_div(Temp_Value, 100000);
	dvalue = (unsigned int) Temp_Value;

    /* Auto adjust value */
	if (batt_meter_cust_data.r_fg_value != 20) {
		dvalue = (dvalue * 20) / batt_meter_cust_data.r_fg_value;
	}

	/* K current */
	if (batt_meter_cust_data.r_fg_board_slope != batt_meter_cust_data.r_fg_board_base) {
		dvalue =
		    ((dvalue * batt_meter_cust_data.r_fg_board_base) +
		     (batt_meter_cust_data.r_fg_board_slope / 2)) /
		    batt_meter_cust_data.r_fg_board_slope;
	}
    /* current compensate */
	if (g_fg_is_charging == KAL_TRUE)
		dvalue = dvalue + Current_Compensate_Value;
	else
		dvalue = dvalue - Current_Compensate_Value;

	dvalue = ((dvalue * batt_meter_cust_data.car_tune_value) / 100);

	dvalue = use_chip_trim_value(dvalue);

	*(signed int *) (data) = dvalue;
#endif

	return STATUS_OK;
}


static signed int fgauge_read_current_sign(void *data)
{
	*(kal_bool *) (data) = g_fg_is_charging;

	return STATUS_OK;
}

static signed int fgauge_read_columb(void *data);

signed int fgauge_set_columb_interrupt_internal(void *data, int reset)
{
#if defined(CONFIG_POWER_EXT)
	return STATUS_OK;
#else

	unsigned int uvalue32_CAR = 0;
	unsigned int uvalue32_CAR_MSB = 0;
	signed short upperbound = 0, lowbound = 0;
	signed short pcar = 0;
	signed short m;
	unsigned int car = *(unsigned int *) (data);
	unsigned int ret = 0;
	unsigned short *plow, *phigh, *pori;

	plow = (unsigned short *) &lowbound;
	phigh = (unsigned short *) &upperbound;
	pori = (unsigned short *) &uvalue32_CAR;

	if (car == 0) {
		pmic_set_register_value(PMIC_RG_INT_EN_FG_BAT_H, 0);
		pmic_set_register_value(PMIC_RG_INT_EN_FG_BAT_L, 0);
		return STATUS_OK;
	}

	if (car == 0x1ffff) {
		pmic_set_register_value(PMIC_RG_INT_EN_FG_BAT_H, 1);
		pmic_set_register_value(PMIC_RG_INT_EN_FG_BAT_L, 1);
		return STATUS_OK;
	}

	/* HW Init */
	/* Set READ command */
	if (reset == 0)
		ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0200, 0xFF00, 0x0);
	else
		ret = pmic_config_interface(MT6328_FGADC_CON0, 0x7300, 0xFF00, 0x0);

	/* Keep i2c read when status = 1 (0x06) */
	m = 0;
	while (fg_get_data_ready_status() == 0) {
		m++;
		if (m > 1000)
			break;
	}
	/* Read FG_CURRENT_OUT[31] */
	uvalue32_CAR = (pmic_get_register_value(PMIC_FG_CAR_15_00)) >> 14;
	uvalue32_CAR |= ((pmic_get_register_value(PMIC_FG_CAR_31_16)) & 0x7FFF) << 2;

	uvalue32_CAR_MSB = (pmic_get_register_value(PMIC_FG_CAR_31_16) & 0x8000) >> 15;
	uvalue32_CAR = uvalue32_CAR & 0x7fff;

	if (uvalue32_CAR_MSB == 1)
		uvalue32_CAR = uvalue32_CAR | 0x8000;

	/* restore use_chip_trim_value() */
	car = ((car * 1000) / chip_diff_trim_value);
	car = ((car * 100) / batt_meter_cust_data.car_tune_value);
	car = ((car * batt_meter_cust_data.r_fg_value) / 20);
	car = car * 8000;
	car = car * 10;
	car = car + 5;
	car = car * 10;
	car = car / 35986;
	pcar = (signed short) car;

	upperbound = *pori;
	lowbound = *pori;

	upperbound = upperbound + pcar;
	lowbound = lowbound - pcar;

	pmic_set_register_value(PMIC_FG_BLTR, *plow);
	pmic_set_register_value(PMIC_FG_BFTR, *phigh);
	/*msleep(1);*/usleep_range(1000, 2000);
	pmic_set_register_value(PMIC_RG_INT_EN_FG_BAT_H, 1);
	pmic_set_register_value(PMIC_RG_INT_EN_FG_BAT_L, 1);

	return STATUS_OK;
#endif

}

static signed int fgauge_set_columb_interrupt(void *data)
{
	return fgauge_set_columb_interrupt_internal(data, 0);
}

static signed int fgauge_read_columb_internal(void *data, int reset, int precise)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 0;
#else
	unsigned int uvalue32_CAR = 0;
	unsigned int uvalue32_CAR_MSB = 0;
	signed int dvalue_CAR = 0;
	int m = 0;
	long long Temp_Value = 0;
	unsigned int ret = 0;

    /* HW Init */
	/* Set READ command */
	if (reset == 0)
		ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0200, 0xFF00, 0x0);
	else
		ret = pmic_config_interface(MT6328_FGADC_CON0, 0x7300, 0xFF00, 0x0);

	/* Keep i2c read when status = 1 (0x06) */
	m = 0;
	while (fg_get_data_ready_status() == 0) {
		m++;
		if (m > 1000)
			break;
	}
	/* Read FG_CURRENT_OUT[31] */
	uvalue32_CAR = (pmic_get_register_value(PMIC_FG_CAR_15_00)) >> 14;
	uvalue32_CAR |= ((pmic_get_register_value(PMIC_FG_CAR_31_16)) & 0x7FFF) << 2;

	uvalue32_CAR_MSB = (pmic_get_register_value(PMIC_FG_CAR_31_16) & 0x8000) >> 15;

	/* Clear status to 0 */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0800, 0xFF00, 0x0);
	/* while ( fg_get_sw_clear_status() != 0 ) */
	m = 0;
	while (fg_get_data_ready_status() != 0) {
		m++;
		if (m > 1000)
			break;
	}
	/* (8)    Recover original settings */
	ret = pmic_config_interface(MT6328_FGADC_CON0, 0x0000, 0xFF00, 0x0);

    /* calculate the real world data */
	dvalue_CAR = (signed int) uvalue32_CAR;

	if (uvalue32_CAR == 0) {
		Temp_Value = 0;
	} else if (uvalue32_CAR == 0x1ffff) {
		Temp_Value = 0;
	} else if (uvalue32_CAR_MSB == 0x1) {
		/* dis-charging */
		Temp_Value = (long long) (dvalue_CAR - 0x1ffff);	/* keep negative value */
		Temp_Value = Temp_Value - (Temp_Value * 2);
	} else {
		/* charging */
		Temp_Value = (long long) dvalue_CAR;
	}

#if 0
	Temp_Value = (((Temp_Value * 35986) / 10) + (5)) / 10;	/* [28:14]'s LSB=359.86 uAh */
#else
	Temp_Value = Temp_Value * 35986;
	do_div(Temp_Value, 10);
	Temp_Value = Temp_Value + 5;
	do_div(Temp_Value, 10);
#endif

#if 0
	dvalue_CAR = Temp_Value / 1000;	/* mAh */
#else
	/* dvalue_CAR = (Temp_Value/8)/1000; //mAh, due to FG_OSR=0x8 */
	if (precise == 0) {
		do_div(Temp_Value, 8);
		do_div(Temp_Value, 1000);
	} else {
		do_div(Temp_Value, 8);
		do_div(Temp_Value, 100);
	}

	if (uvalue32_CAR_MSB == 0x1)
		dvalue_CAR = (signed int) (Temp_Value - (Temp_Value * 2));	/* keep negative value */
	else
		dvalue_CAR = (signed int) Temp_Value;
#endif

    /* Auto adjust value */
	if (batt_meter_cust_data.r_fg_value != 20) {
		dvalue_CAR = (dvalue_CAR * 20) / batt_meter_cust_data.r_fg_value;
	}

	dvalue_CAR = ((dvalue_CAR * batt_meter_cust_data.car_tune_value) / 100);
	dvalue_CAR = use_chip_trim_value(dvalue_CAR);

	*(signed int *) (data) = dvalue_CAR;
#endif

	return STATUS_OK;
}

static signed int fgauge_read_columb(void *data)
{
	return fgauge_read_columb_internal(data, 0, 0);
}

static signed int fgauge_read_columb_accurate(void *data)
{
	return fgauge_read_columb_internal(data, 0, 1);
}

static signed int fgauge_hw_reset(void *data)
{
#if defined(CONFIG_POWER_EXT)
/*  */
#else
	volatile unsigned int val_car = 1;
	unsigned int val_car_temp = 1;
	unsigned int ret = 0;

	while (val_car != 0x0) {
		ret = pmic_config_interface(MT6328_FGADC_CON0, 0x7100, 0xFF00, 0x0);
		fgauge_read_columb_internal(&val_car_temp, 1, 0);
		val_car = val_car_temp;
		bm_print(BM_LOG_FULL, "#");
	}

#endif

	return STATUS_OK;
}

static signed int read_adc_v_bat_sense(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 4201;
#else
#if defined(SWCHR_POWER_PATH)
	*(signed int *) (data) =
	    PMIC_IMM_GetOneChannelValue(MT6328_AUX_ISENSE_AP, *(signed int *) (data), 1);
#else
	*(signed int *) (data) =
	    PMIC_IMM_GetOneChannelValue(MT6328_AUX_BATSNS_AP, *(signed int *) (data), 1);
#endif
#endif

	return STATUS_OK;
}

static signed int read_adc_v_i_sense(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 4202;
#else
#if defined(SWCHR_POWER_PATH)
	*(signed int *) (data) =
	    PMIC_IMM_GetOneChannelValue(MT6328_AUX_BATSNS_AP, *(signed int *) (data), 1);
#else
	*(signed int *) (data) =
	    PMIC_IMM_GetOneChannelValue(MT6328_AUX_ISENSE_AP, *(signed int *) (data), 1);
#endif
#endif

	return STATUS_OK;
}

static signed int read_adc_v_bat_temp(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 0;
#else
#if defined(MTK_PCB_TBAT_FEATURE)
	/* no HW support */
#else
	*(signed int *) (data) =
	    PMIC_IMM_GetOneChannelValue(MT6328_AUX_BATON_AP, *(signed int *) (data), 1);
#endif
#endif

	return STATUS_OK;
}

static signed int read_adc_v_charger(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 5001;
#else
	signed int val;

	val = PMIC_IMM_GetOneChannelValue(MT6328_AUX_VCDT_AP, *(signed int *) (data), 1);
	val =
	    (((batt_meter_cust_data.r_charger_1 +
	       batt_meter_cust_data.r_charger_2) * 100 * val) / batt_meter_cust_data.r_charger_2) / 100;
	*(signed int *) (data) = val;
#endif

	return STATUS_OK;
}

static signed int read_hw_ocv(void *data)
{
#if defined(CONFIG_POWER_EXT)
	*(signed int *) (data) = 3999;
#else
#if 0
	*(signed int *) (data) = PMIC_IMM_GetOneChannelValue(AUX_BATSNS_AP, 5, 1);
#else
	*(signed int *) (data) = get_hw_ocv();
#endif
#endif

	return STATUS_OK;
}

static signed int dump_register_fgadc(void *data)
{
	return STATUS_OK;
}

static signed int read_battery_plug_out_status(void *data)
{
	*(signed int *) (data) = is_battery_remove_pmic();

	return STATUS_OK;
}

static signed int(*bm_func[BATTERY_METER_CMD_NUMBER]) (void *data);

signed int bm_ctrl_cmd(BATTERY_METER_CTRL_CMD cmd, void *data)
{
	signed int status = STATUS_UNSUPPORTED;
	static signed int init = -1;

	if (init == -1) {
		init = 0;
		bm_func[BATTERY_METER_CMD_HW_FG_INIT] = fgauge_initialization;
		bm_func[BATTERY_METER_CMD_GET_HW_FG_CURRENT] = fgauge_read_current;
		bm_func[BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN] = fgauge_read_current_sign;
		bm_func[BATTERY_METER_CMD_GET_HW_FG_CAR] = fgauge_read_columb;
		bm_func[BATTERY_METER_CMD_HW_RESET] = fgauge_hw_reset;
		bm_func[BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE] = read_adc_v_bat_sense;
		bm_func[BATTERY_METER_CMD_GET_ADC_V_I_SENSE] = read_adc_v_i_sense;
		bm_func[BATTERY_METER_CMD_GET_ADC_V_BAT_TEMP] = read_adc_v_bat_temp;
		bm_func[BATTERY_METER_CMD_GET_ADC_V_CHARGER] = read_adc_v_charger;
		bm_func[BATTERY_METER_CMD_GET_HW_OCV] = read_hw_ocv;
		bm_func[BATTERY_METER_CMD_DUMP_REGISTER] = dump_register_fgadc;
		bm_func[BATTERY_METER_CMD_SET_COLUMB_INTERRUPT] = fgauge_set_columb_interrupt;
		bm_func[BATTERY_METER_CMD_GET_BATTERY_PLUG_STATUS] = read_battery_plug_out_status;
		bm_func[BATTERY_METER_CMD_GET_HW_FG_CAR_ACT] = fgauge_read_columb_accurate;
	}

	if (cmd < BATTERY_METER_CMD_NUMBER) {
		if (bm_func[cmd] != NULL)
			status = bm_func[cmd] (data);
	} else
		return STATUS_UNSUPPORTED;

	return status;
}
