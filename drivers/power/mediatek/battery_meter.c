#include <linux/init.h>		/* For init/exit macros */
#include <linux/module.h>	/* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/slab.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#include <asm/uaccess.h>

#include <mt-plat/mt_boot.h>
#include <mt-plat/mtk_rtc.h>


#include <mt-plat/mt_boot_reason.h>

#include <mt-plat/battery_meter.h>
#include <mt-plat/battery_common.h>
#include <mt-plat/battery_meter_hal.h>
//#include <mach/mt_battery_meter.h>
#include "cust_battery_meter.h"

//#include <mach/mt_battery_meter_table.h>
#include "cust_battery_meter_table.h"

#include <mach/mt_pmic.h>


#include <mt-plat/upmu_common.h>

#ifdef TINNO_GET_OCV_FORM_LK
int tinno_get_bat_ocv(void);
#endif

/* ============================================================ // */
/* define */
/* ============================================================ // */
#define PROFILE_SIZE 4

static DEFINE_MUTEX(FGADC_mutex);

int Enable_FGADC_LOG = 0;

/* ============================================================ // */
/* global variable */
/* ============================================================ // */
BATTERY_METER_CONTROL battery_meter_ctrl = NULL;

/* static struct proc_dir_entry *proc_entry_fgadc; */
static char proc_fgadc_data[32];

kal_bool gFG_Is_Charging = KAL_FALSE;
signed int g_auxadc_solution = 0;
unsigned int g_spm_timer = 600;
BOOL bat_spm_timeout = false;
unsigned int _g_bat_sleep_total_time = NORMAL_WAKEUP_PERIOD;
#ifdef MTK_ENABLE_AGING_ALGORITHM
unsigned int suspend_time = 0;
#endif
signed int g_booting_vbat = 0;
#if !defined(CONFIG_POWER_EXT)
static unsigned int temperature_change = 1;
#endif
s32 g_reflash_timer = 150;	//LINE <><optimize the soft fule-gauge><20160429> panzaoyan
s32 g_sleep_oam_time = 0;
s32 oam_delta_time = 0;
s32 sleep_reflash_oam = 0;	//LINE <><optimize the soft fule-gauge><20160429> panzaoyan

#if defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
static signed int g_currentfactor = 100;
static kal_bool g_USE_UI_SOC = KAL_TRUE;
#if defined(CUST_SYSTEM_OFF_VOLTAGE)
#define SYSTEM_OFF_VOLTAGE CUST_SYSTEM_OFF_VOLTAGE
#endif
#endif

/* ///////////////////////////////////////////////////////////////////////////////////////// */
/* // PMIC AUXADC Related Variable */
/* ///////////////////////////////////////////////////////////////////////////////////////// */
int g_R_BAT_SENSE;		/* R_BAT_SENSE; */
int g_R_I_SENSE;		/* R_I_SENSE; */
int g_R_CHARGER_1;		/* R_CHARGER_1; */
int g_R_CHARGER_2;		/* R_CHARGER_2; */

int fg_qmax_update_for_aging_flag = 1;

/* HW FG */
signed int gFG_DOD0 = 0;
signed int gFG_DOD1 = 0;
signed int gFG_columb = 0;
signed int gFG_voltage = 0;
signed int gFG_current = 0;
signed int gFG_capacity = 0;
signed int gFG_capacity_by_c = 0;
signed int gFG_capacity_by_c_init = 0;
signed int gFG_capacity_by_v = 0;
signed int gFG_capacity_by_v_init = 0;
signed int gFG_temp = 100;
signed int gFG_resistance_bat = 0;
signed int gFG_compensate_value = 0;
signed int gFG_ori_voltage = 0;
signed int gFG_BATT_CAPACITY = 0;
signed int gFG_voltage_init = 0;
signed int gFG_current_auto_detect_R_fg_total = 0;
signed int gFG_current_auto_detect_R_fg_count = 0;
signed int gFG_current_auto_detect_R_fg_result = 0;
signed int gFG_15_vlot = 3700;
signed int gFG_BATT_CAPACITY_init_high_current = 1200;
signed int gFG_BATT_CAPACITY_aging = 1200;

/* voltage mode */
signed int gfg_percent_check_point = 50;
signed int volt_mode_update_timer = 0;
signed int volt_mode_update_time_out = 6;	/* 1mins */

/* EM */
signed int g_fg_dbg_bat_volt = 0;
signed int g_fg_dbg_bat_current = 0;
signed int g_fg_dbg_bat_zcv = 0;
signed int g_fg_dbg_bat_temp = 0;
signed int g_fg_dbg_bat_r = 0;
signed int g_fg_dbg_bat_car = 0;
signed int g_fg_dbg_bat_qmax = 0;
signed int g_fg_dbg_d0 = 0;
signed int g_fg_dbg_d1 = 0;
signed int g_fg_dbg_percentage = 0;
signed int g_fg_dbg_percentage_fg = 0;
signed int g_fg_dbg_percentage_voltmode = 0;

signed int FGvbatVoltageBuffer[FG_VBAT_AVERAGE_SIZE];
signed int FGbatteryIndex = 0;
signed int FGbatteryVoltageSum = 0;
signed int gFG_voltage_AVG = 0;
signed int gFG_vbat_offset = 0;
#ifdef Q_MAX_BY_CURRENT
signed int FGCurrentBuffer[FG_CURRENT_AVERAGE_SIZE];
signed int FGCurrentIndex = 0;
signed int FGCurrentSum = 0;
signed int gFG_current_AVG = 0;
#endif
signed int g_tracking_point;	/* CUST_TRACKING_POINT; */
signed int g_rtc_fg_soc = 0;
signed int g_I_SENSE_offset = 0;

/* SW FG */
signed int oam_v_ocv_init = 0;
signed int oam_v_ocv_1 = 0;
signed int oam_v_ocv_2 = 0;
signed int oam_r_1 = 0;
signed int oam_r_2 = 0;
signed int oam_d0 = 0;
signed int oam_i_ori = 0;
signed int oam_i_1 = 0;
signed int oam_i_2 = 0;
signed int oam_car_1 = 0;
signed int oam_car_2 = 0;
signed int oam_d_1 = 1;
signed int oam_d_2 = 1;
signed int oam_d_3 = 1;
signed int oam_d_3_pre = 0;
signed int oam_d_4 = 0;
signed int oam_d_4_pre = 0;
signed int oam_d_5 = 0;
signed int oam_init_i = 0;
signed int oam_run_i = 0;
signed int d5_count = 0;
signed int d5_count_time = 60;
signed int d5_count_time_rate = 1;
signed int g_d_hw_ocv = 0;
signed int g_vol_bat_hw_ocv = 0;
signed int g_hw_ocv_before_sleep = 0;
struct timespec g_rtc_time_before_sleep, xts_before_sleep, g_sleep_total_time;
signed int g_sw_vbat_temp = 0;
struct timespec last_oam_run_time;

/* aging mechanism */
#ifdef MTK_ENABLE_AGING_ALGORITHM

#ifdef SOC_BY_HW_FG
static signed int aging_ocv_1;
static signed int aging_ocv_2;
static signed int aging_car_1;
static signed int aging_car_2;
static signed int aging_dod_1;
static signed int aging_dod_2;
#ifdef MD_SLEEP_CURRENT_CHECK
static signed int columb_before_sleep = 0x123456;
#endif
#endif
/* static time_t aging_resume_time_1 = 0; */
/* static time_t aging_resume_time_2 = 0; */

#ifndef SELF_DISCHARGE_CHECK_THRESHOLD
#define SELF_DISCHARGE_CHECK_THRESHOLD 10
#endif

#ifndef OCV_RECOVER_TIME
#define OCV_RECOVER_TIME 2100
#endif

#ifndef DOD1_ABOVE_THRESHOLD
#define DOD1_ABOVE_THRESHOLD 30
#endif

#ifndef DOD2_BELOW_THRESHOLD
#define DOD2_BELOW_THRESHOLD 70
#endif

#ifndef MIN_DOD_DIFF_THRESHOLD
#define MIN_DOD_DIFF_THRESHOLD 60
#endif

#ifndef MIN_AGING_FACTOR
#define MIN_AGING_FACTOR 90
#endif

#endif				/* aging mechanism */

/* battery info */
#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT

signed int gFG_battery_cycle = 0;
signed int gFG_aging_factor = 100;
signed int gFG_columb_sum = 0;
signed int gFG_pre_columb_count = 0;

signed int gFG_max_voltage = 0;
signed int gFG_min_voltage = 10000;
signed int gFG_max_current = 0;
signed int gFG_min_current = 0;
signed int gFG_max_temperature = -20;
signed int gFG_min_temperature = 100;

#endif				/* battery info */

/*extern char *saved_command_line;*/
/* Temperature window size */
#define TEMP_AVERAGE_SIZE	30

kal_bool gFG_Is_offset_init = KAL_FALSE;


void battery_meter_reset_sleep_time(void)
{
	g_sleep_total_time.tv_sec = 0;
	g_sleep_total_time.tv_nsec = 0;
}


#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
/*extern int IMM_GetOneChannelValue_Cali(int Channel, int *voltage);*/
unsigned int g_fg_battery_id = 0;

#ifdef MTK_GET_BATTERY_ID_BY_AUXADC
void fgauge_get_profile_id(void)
{
	int id_volt = 0;
	int id = 0;
	int ret = 0;

	ret = IMM_GetOneChannelValue_Cali(BATTERY_ID_CHANNEL_NUM, &id_volt);

	if ((sizeof(g_battery_id_voltage) / sizeof(signed int)) != TOTAL_BATTERY_NUMBER) {
		return;
	}

	for (id = 0; id < TOTAL_BATTERY_NUMBER; id++) {
		if (id_volt < g_battery_id_voltage[id]) {
			g_fg_battery_id = id;
			break;
		} else if (g_battery_id_voltage[id] == -1) {
			g_fg_battery_id = TOTAL_BATTERY_NUMBER - 1;
		}
	}

}
#elif defined(MTK_GET_BATTERY_ID_BY_GPIO)
void fgauge_get_profile_id(void)
{
	g_fg_battery_id = 0;
}
#else
void fgauge_get_profile_id(void)
{
	g_fg_battery_id = 0;
}
#endif
#endif

/* ============================================================ // */
/* function prototype */
/* ============================================================ // */
struct battery_meter_custom_data batt_meter_cust_data;

int __batt_meter_init_cust_data_from_cust_header(void)
{
	battery_log(BAT_LOG_CRTI, "__batt_meter_init_cust_data_from_cust_header\n");

	/* mt_battery_meter_table.h */
#if (BAT_NTC_10 == 1)
	batt_meter_cust_data.bat_ntc = 10;
#elif (BAT_NTC_47 == 1)
	batt_meter_cust_data.bat_ntc = 47;
#endif

#if defined(RBAT_PULL_UP_R)
	batt_meter_cust_data.rbat_pull_up_r = RBAT_PULL_UP_R;
#endif
#if defined(RBAT_PULL_UP_VOLT)
	batt_meter_cust_data.rbat_pull_up_volt = RBAT_PULL_UP_VOLT;
#endif


	/* mt_battery_meter.h */

	/* ADC resister */
#if defined(R_BAT_SENSE)
	batt_meter_cust_data.r_bat_sense = R_BAT_SENSE;
	g_R_BAT_SENSE = R_BAT_SENSE;
#endif
#if defined(R_I_SENSE)
	batt_meter_cust_data.r_i_sense = R_I_SENSE;
	g_R_I_SENSE = R_I_SENSE;
#endif
#if defined(R_CHARGER_1)
	batt_meter_cust_data.r_charger_1 = R_CHARGER_1;
	g_R_CHARGER_1 = R_CHARGER_1;
#endif
#if defined(R_CHARGER_2)
	batt_meter_cust_data.r_charger_2 = R_CHARGER_2;
	g_R_CHARGER_2 = R_CHARGER_2;
#endif

#if defined(TEMPERATURE_T0)
	batt_meter_cust_data.temperature_t0 = TEMPERATURE_T0;
#endif
#if defined(TEMPERATURE_T1)
	batt_meter_cust_data.temperature_t1 = TEMPERATURE_T1;
#endif
#if defined(TEMPERATURE_T2)
	batt_meter_cust_data.temperature_t2 = TEMPERATURE_T2;
#endif
#if defined(TEMPERATURE_T3)
	batt_meter_cust_data.temperature_t3 = TEMPERATURE_T3;
#endif
#if defined(TEMPERATURE_T)
	batt_meter_cust_data.temperature_t = TEMPERATURE_T;
#endif
#if defined(FG_METER_RESISTANCE)
	batt_meter_cust_data.fg_meter_resistance = FG_METER_RESISTANCE;
#endif


	/* Qmax for battery  */
#if defined(Q_MAX_POS_50)
	batt_meter_cust_data.q_max_pos_50 = Q_MAX_POS_50;
#endif
#if defined(Q_MAX_POS_25)
	batt_meter_cust_data.q_max_pos_25 = Q_MAX_POS_25;
#endif
#if defined(Q_MAX_POS_0)
	batt_meter_cust_data.q_max_pos_0 = Q_MAX_POS_0;
#endif
#if defined(Q_MAX_NEG_10)
	batt_meter_cust_data.q_max_neg_10 = Q_MAX_NEG_10;
#endif
#if defined(Q_MAX_POS_50_H_CURRENT)
	batt_meter_cust_data.q_max_pos_50_h_current = Q_MAX_POS_50_H_CURRENT;
#endif
#if defined(Q_MAX_POS_25_H_CURRENT)
	batt_meter_cust_data.q_max_pos_25_h_current = Q_MAX_POS_25_H_CURRENT;
#endif
#if defined(Q_MAX_POS_0_H_CURRENT)
	batt_meter_cust_data.q_max_pos_0_h_current = Q_MAX_POS_0_H_CURRENT;
#endif
#if defined(Q_MAX_NEG_10_H_CURRENT)
	batt_meter_cust_data.q_max_neg_10_h_current = Q_MAX_NEG_10_H_CURRENT;
#endif
#if defined(OAM_D5)
	batt_meter_cust_data.oam_d5 = OAM_D5;	/* 1 : D5,   0: D2 */
#endif


#if defined(CHANGE_TRACKING_POINT)
	batt_meter_cust_data.change_tracking_point = 1;
#else				/* #if defined(CHANGE_TRACKING_POINT) */
	batt_meter_cust_data.change_tracking_point = 0;
#endif				/* #if defined(CHANGE_TRACKING_POINT) */


#if defined(CUST_TRACKING_POINT)
	batt_meter_cust_data.cust_tracking_point = CUST_TRACKING_POINT;
	g_tracking_point = CUST_TRACKING_POINT;
#endif
#if defined(CUST_R_SENSE)
	batt_meter_cust_data.cust_r_sense = CUST_R_SENSE;
#endif
#if defined(CUST_HW_CC)
	batt_meter_cust_data.cust_hw_cc = CUST_HW_CC;
#endif
#if defined(AGING_TUNING_VALUE)
	batt_meter_cust_data.aging_tuning_value = AGING_TUNING_VALUE;
#endif
#if defined(CUST_R_FG_OFFSET)
	batt_meter_cust_data.cust_r_fg_offset = CUST_R_FG_OFFSET;
#endif
#if defined(OCV_BOARD_COMPESATE)
	batt_meter_cust_data.ocv_board_compesate = OCV_BOARD_COMPESATE;
#endif
#if defined(R_FG_BOARD_BASE)
	batt_meter_cust_data.r_fg_board_base = R_FG_BOARD_BASE;
#endif
#if defined(R_FG_BOARD_SLOPE)
	batt_meter_cust_data.r_fg_board_slope = R_FG_BOARD_SLOPE;
#endif
#if defined(CAR_TUNE_VALUE)
	batt_meter_cust_data.car_tune_value = CAR_TUNE_VALUE;
#endif

	/* HW Fuel gague  */
#if defined(CURRENT_DETECT_R_FG)
	batt_meter_cust_data.current_detect_r_fg = CURRENT_DETECT_R_FG;
#endif
#if defined(MinErrorOffset)
	batt_meter_cust_data.minerroroffset = MinErrorOffset;
#endif
#if defined(FG_VBAT_AVERAGE_SIZE)
	batt_meter_cust_data.fg_vbat_average_size = FG_VBAT_AVERAGE_SIZE;
#endif
#if defined(R_FG_VALUE)
	batt_meter_cust_data.r_fg_value = R_FG_VALUE;
#endif
#if defined(CUST_POWERON_DELTA_CAPACITY_TOLRANCE)
	batt_meter_cust_data.cust_poweron_delta_capacity_tolrance =
	    CUST_POWERON_DELTA_CAPACITY_TOLRANCE;
#endif
#if defined(CUST_POWERON_LOW_CAPACITY_TOLRANCE)
	batt_meter_cust_data.cust_poweron_low_capacity_tolrance =
	    CUST_POWERON_LOW_CAPACITY_TOLRANCE;
#endif
#if defined(CUST_POWERON_MAX_VBAT_TOLRANCE)
	batt_meter_cust_data.cust_poweron_max_vbat_tolrance = CUST_POWERON_MAX_VBAT_TOLRANCE;
#endif
#if defined(CUST_POWERON_DELTA_VBAT_TOLRANCE)
	batt_meter_cust_data.cust_poweron_delta_vbat_tolrance = CUST_POWERON_DELTA_VBAT_TOLRANCE;
#endif
#if defined(CUST_POWERON_DELTA_HW_SW_OCV_CAPACITY_TOLRANCE)
	batt_meter_cust_data.cust_poweron_delta_hw_sw_ocv_capacity_tolrance =
	    CUST_POWERON_DELTA_HW_SW_OCV_CAPACITY_TOLRANCE;
#endif

#if defined(FIXED_TBAT_25)
	batt_meter_cust_data.fixed_tbat_25 = 1;
#else				/* #if defined(FIXED_TBAT_25) */
	batt_meter_cust_data.fixed_tbat_25 = 0;
#endif				/* #if defined(FIXED_TBAT_25) */

	/* Dynamic change wake up period of battery thread when suspend */
#if defined(VBAT_NORMAL_WAKEUP)
	batt_meter_cust_data.vbat_normal_wakeup = VBAT_NORMAL_WAKEUP;
#endif
#if defined(VBAT_LOW_POWER_WAKEUP)
	batt_meter_cust_data.vbat_low_power_wakeup = VBAT_LOW_POWER_WAKEUP;
#endif
#if defined(NORMAL_WAKEUP_PERIOD)
	batt_meter_cust_data.normal_wakeup_period = NORMAL_WAKEUP_PERIOD;
	_g_bat_sleep_total_time = NORMAL_WAKEUP_PERIOD;
#endif
#if defined(LOW_POWER_WAKEUP_PERIOD)
	batt_meter_cust_data.low_power_wakeup_period = LOW_POWER_WAKEUP_PERIOD;
#endif
#if defined(CLOSE_POWEROFF_WAKEUP_PERIOD)
	batt_meter_cust_data.close_poweroff_wakeup_period = CLOSE_POWEROFF_WAKEUP_PERIOD;
#endif

#if defined(IS_BATTERY_REMOVE_BY_PMIC)
	batt_meter_cust_data.vbat_remove_detection = 1;
#else	/* #if defined(IS_BATTERY_REMOVE_BY_PMIC) */
	batt_meter_cust_data.vbat_remove_detection = 0;
#endif	/* #if defined(IS_BATTERY_REMOVE_BY_PMIC) */

	return 0;
}

#if defined(BATTERY_DTS_SUPPORT) && defined(CONFIG_OF)
static void __batt_meter_parse_node(const struct device_node *np,
				const char *node_srting, int *cust_val)
{
	u32 val;
	if (of_property_read_u32(np, node_srting, &val) == 0) {
		(*cust_val) = (int)val;
	}
}

static void __batt_meter_parse_table(const struct device_node *np,
				const char *node_srting, BATTERY_PROFILE_STRUCT_P profile_p)
{
	int addr, val, idx, saddles;

	/*the number of battery table is
		the same as the number of r table*/
	saddles = fgauge_get_saddles();
	idx = 0;

	while (!of_property_read_u32_index(np, node_srting, idx, &addr)) {
		idx++;
		if (!of_property_read_u32_index(np, node_srting, idx, &val)) {
		}
		profile_p->percentage = addr;
		profile_p->voltage = val;

		profile_p++;
		if ((idx++) >= (saddles * 2))
			break;
	}

	/* error handle */
	if (0 == idx) {
		return;
	}

	/* use last data to fill with the rest array
				if raw data is less than temp array */
	/* error handle */
	profile_p--;

	while (idx < (saddles * 2)) {
		profile_p++;
		profile_p->percentage = addr;
		profile_p->voltage = val;
		idx = idx + 2;
	}
}

int __batt_meter_init_cust_data_from_dt(void)
{
	struct device_node *np;
	int num;
	unsigned int idx, addr, val;

	/* check customer setting */
	np = of_find_compatible_node(NULL, NULL, "mediatek,bat_meter");
	if (!np) {
		/* printk(KERN_ERR "(E) Failed to find device-tree node: %s\n", path); */
		battery_log(BAT_LOG_CRTI, "Failed to find device-tree node: bat_meter\n");
		return -ENODEV;
	}

	__batt_meter_parse_node(np, "rbat_pull_up_r",
		&batt_meter_cust_data.rbat_pull_up_r);

	__batt_meter_parse_node(np, "rbat_pull_up_volt",
		&batt_meter_cust_data.rbat_pull_up_volt);

	__batt_meter_parse_node(np, "batt_temperature_table_num", &num);

	idx = 0;
	while (!of_property_read_u32_index(np, "batt_temperature_table", idx, &addr)) {
		idx++;
		if (!of_property_read_u32_index(np, "batt_temperature_table", idx, &val)) {
			battery_log(BAT_LOG_CRTI, "batt_temperature_table: addr: %d, val: %d\n",
				    addr, val);
		}
		Batt_Temperature_Table[idx / 2].BatteryTemp = addr;
		Batt_Temperature_Table[idx / 2].TemperatureR = val;

		idx++;
		if (idx >= num * 2)
			break;
	}

	__batt_meter_parse_node(np, "battery_profile_t0_num", &num);

	__batt_meter_parse_table(np, "battery_profile_t0",
			fgauge_get_profile(batt_meter_cust_data.temperature_t0));

	__batt_meter_parse_node(np, "battery_profile_t1_num", &num);

	__batt_meter_parse_table(np, "battery_profile_t1",
			fgauge_get_profile(batt_meter_cust_data.temperature_t1));

	__batt_meter_parse_node(np, "battery_profile_t2_num", &num);

	__batt_meter_parse_table(np, "battery_profile_t2",
			fgauge_get_profile(batt_meter_cust_data.temperature_t2));

	__batt_meter_parse_node(np, "battery_profile_t3_num", &num);

	__batt_meter_parse_table(np, "battery_profile_t3",
			fgauge_get_profile(batt_meter_cust_data.temperature_t3));

	__batt_meter_parse_node(np, "r_profile_t0_num",	&num);

	__batt_meter_parse_table(np, "r_profile_t0",
			(BATTERY_PROFILE_STRUCT *)fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t0));

	__batt_meter_parse_node(np, "r_profile_t1_num",	&num);

	__batt_meter_parse_table(np, "r_profile_t1",
			(BATTERY_PROFILE_STRUCT *)fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t1));

	__batt_meter_parse_node(np, "r_profile_t2_num",	&num);

	__batt_meter_parse_table(np, "r_profile_t2",
			(BATTERY_PROFILE_STRUCT *)fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t2));

	__batt_meter_parse_node(np, "r_profile_t3_num",	&num);

	__batt_meter_parse_table(np, "r_profile_t3",
			(BATTERY_PROFILE_STRUCT *)fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t3));

	__batt_meter_parse_node(np, "r_bat_sense",
		&batt_meter_cust_data.r_bat_sense);

	__batt_meter_parse_node(np, "r_i_sense",
		&batt_meter_cust_data.r_i_sense);

	__batt_meter_parse_node(np, "r_charger_1",
		&batt_meter_cust_data.r_charger_1);

	__batt_meter_parse_node(np, "r_charger_2",
		&batt_meter_cust_data.r_charger_2);

	__batt_meter_parse_node(np, "temperature_t0",
		&batt_meter_cust_data.temperature_t0);

	__batt_meter_parse_node(np, "temperature_t1",
		&batt_meter_cust_data.temperature_t1);

	__batt_meter_parse_node(np, "temperature_t2",
		&batt_meter_cust_data.temperature_t2);

	__batt_meter_parse_node(np, "temperature_t3",
		&batt_meter_cust_data.temperature_t3);

	__batt_meter_parse_node(np, "temperature_t",
		&batt_meter_cust_data.temperature_t);

	__batt_meter_parse_node(np, "fg_meter_resistance",
		&batt_meter_cust_data.fg_meter_resistance);

	__batt_meter_parse_node(np, "q_max_pos_50",
		&batt_meter_cust_data.q_max_pos_50);

	__batt_meter_parse_node(np, "q_max_pos_25",
		&batt_meter_cust_data.q_max_pos_25);

	__batt_meter_parse_node(np, "q_max_pos_0",
		&batt_meter_cust_data.q_max_pos_0);

	__batt_meter_parse_node(np, "q_max_neg_10",
		&batt_meter_cust_data.q_max_neg_10);

	__batt_meter_parse_node(np, "q_max_pos_50_h_current",
		&batt_meter_cust_data.q_max_pos_50_h_current);

	__batt_meter_parse_node(np, "q_max_pos_25_h_current",
		&batt_meter_cust_data.q_max_pos_25_h_current);

	__batt_meter_parse_node(np, "q_max_pos_0_h_current",
		&batt_meter_cust_data.q_max_pos_0_h_current);

	__batt_meter_parse_node(np, "oam_d5",
		&batt_meter_cust_data.oam_d5);

	__batt_meter_parse_node(np, "change_tracking_point",
		&batt_meter_cust_data.change_tracking_point);

	__batt_meter_parse_node(np, "cust_tracking_point",
		&batt_meter_cust_data.cust_tracking_point);

	__batt_meter_parse_node(np, "cust_r_sense",
		&batt_meter_cust_data.cust_r_sense);

	__batt_meter_parse_node(np, "cust_hw_cc",
		&batt_meter_cust_data.cust_hw_cc);

	__batt_meter_parse_node(np, "aging_tuning_value",
		&batt_meter_cust_data.aging_tuning_value);

	__batt_meter_parse_node(np, "cust_r_fg_offset",
		&batt_meter_cust_data.cust_r_fg_offset);

	__batt_meter_parse_node(np, "ocv_board_compesate",
		&batt_meter_cust_data.ocv_board_compesate);

	__batt_meter_parse_node(np, "r_fg_board_base",
		&batt_meter_cust_data.r_fg_board_base);

	__batt_meter_parse_node(np, "r_fg_board_slope",
		&batt_meter_cust_data.r_fg_board_slope);

	__batt_meter_parse_node(np, "car_tune_value",
		&batt_meter_cust_data.car_tune_value);

	__batt_meter_parse_node(np, "current_detect_r_fg",
		&batt_meter_cust_data.current_detect_r_fg);

	__batt_meter_parse_node(np, "minerroroffset",
		&batt_meter_cust_data.minerroroffset);

	__batt_meter_parse_node(np, "fg_vbat_average_size",
		&batt_meter_cust_data.fg_vbat_average_size);

	__batt_meter_parse_node(np, "r_fg_value",
		&batt_meter_cust_data.r_fg_value);

	__batt_meter_parse_node(np, "cust_poweron_delta_capacity_tolrance",
		&batt_meter_cust_data.cust_poweron_delta_capacity_tolrance);

	__batt_meter_parse_node(np, "cust_poweron_low_capacity_tolrance",
		&batt_meter_cust_data.cust_poweron_low_capacity_tolrance);

	__batt_meter_parse_node(np, "cust_poweron_max_vbat_tolrance",
		&batt_meter_cust_data.cust_poweron_max_vbat_tolrance);

	__batt_meter_parse_node(np, "cust_poweron_delta_vbat_tolrance",
		&batt_meter_cust_data.cust_poweron_delta_vbat_tolrance);

	__batt_meter_parse_node(np, "cust_poweron_delta_hw_sw_ocv_capacity_tolrance",
		&batt_meter_cust_data.cust_poweron_delta_hw_sw_ocv_capacity_tolrance);

	__batt_meter_parse_node(np, "fixed_tbat_25",
		&batt_meter_cust_data.fixed_tbat_25);

	__batt_meter_parse_node(np, "vbat_normal_wakeup",
		&batt_meter_cust_data.vbat_normal_wakeup);

	__batt_meter_parse_node(np, "vbat_low_power_wakeup",
		&batt_meter_cust_data.vbat_low_power_wakeup);

	__batt_meter_parse_node(np, "normal_wakeup_period",
		&batt_meter_cust_data.normal_wakeup_period);

	__batt_meter_parse_node(np, "low_power_wakeup_period",
		&batt_meter_cust_data.low_power_wakeup_period);

	__batt_meter_parse_node(np, "close_poweroff_wakeup_period",
		&batt_meter_cust_data.close_poweroff_wakeup_period);

	__batt_meter_parse_node(np, "vbat_remove_detection",
		&batt_meter_cust_data.vbat_remove_detection);

	of_node_put(np);

	return 0;
}
#endif

int batt_meter_init_cust_data(void)
{
	static int init_done;

	if (init_done == 1)
		return 0;
	init_done = 1;

	__batt_meter_init_cust_data_from_cust_header();

#if defined(BATTERY_DTS_SUPPORT) && defined(CONFIG_OF)
	__batt_meter_init_cust_data_from_dt();
#endif

	return 0;
}





/* ============================================================ // */
int get_r_fg_value(void)
{
	return batt_meter_cust_data.r_fg_value + batt_meter_cust_data.cust_r_fg_offset;
}

#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
int BattThermistorConverTemp(int Res)
{
	int i = 0;
	int RES1 = 0, RES2 = 0;
	int TBatt_Value = -200, TMP1 = 0, TMP2 = 0;

	BATT_TEMPERATURE *batt_temperature_table = &Batt_Temperature_Table[g_fg_battery_id];

	if (Res >= batt_temperature_table[0].TemperatureR) {
		TBatt_Value = -20;
	} else if (Res <= batt_temperature_table[16].TemperatureR) {
		TBatt_Value = 60;
	} else {
		RES1 = batt_temperature_table[0].TemperatureR;
		TMP1 = batt_temperature_table[0].BatteryTemp;

		for (i = 0; i <= 16; i++) {
			if (Res < batt_temperature_table[i].TemperatureR) {
				RES1 = batt_temperature_table[i].TemperatureR;
				TMP1 = batt_temperature_table[i].BatteryTemp;
			} else {
				RES2 = batt_temperature_table[i].TemperatureR;
				TMP2 = batt_temperature_table[i].BatteryTemp;
				break;
			}
		}

		TBatt_Value = (((Res - RES2) * TMP1) + ((RES1 - Res) * TMP2)) / (RES1 - RES2);
	}

	return TBatt_Value;
}

signed int fgauge_get_Q_max(signed short temperature)
{
	signed int ret_Q_max = 0;
	signed int low_temperature = 0, high_temperature = 0;
	signed int low_Q_max = 0, high_Q_max = 0;

	if (temperature <= batt_meter_cust_data.temperature_t1) {
		low_temperature = (-10);
		low_Q_max = g_Q_MAX_NEG_10[g_fg_battery_id];
		high_temperature = batt_meter_cust_data.temperature_t1;
		high_Q_max = g_Q_MAX_POS_0[g_fg_battery_id];

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else if (temperature <= batt_meter_cust_data.temperature_t2) {
		low_temperature = batt_meter_cust_data.temperature_t1;
		low_Q_max = g_Q_MAX_POS_0[g_fg_battery_id];
		high_temperature = batt_meter_cust_data.temperature_t2;
		high_Q_max = g_Q_MAX_POS_25[g_fg_battery_id];

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else {
		low_temperature = batt_meter_cust_data.temperature_t2;
		low_Q_max = g_Q_MAX_POS_25[g_fg_battery_id];
		high_temperature = batt_meter_cust_data.temperature_t3;
		high_Q_max = g_Q_MAX_POS_50[g_fg_battery_id];

		if (temperature > high_temperature)
			temperature = high_temperature;

	}

	ret_Q_max = low_Q_max + (((temperature - low_temperature) * (high_Q_max - low_Q_max)
				 ) / (high_temperature - low_temperature)
	    );

	return ret_Q_max;
}


signed int fgauge_get_Q_max_high_current(signed short temperature)
{
	signed int ret_Q_max = 0;
	signed int low_temperature = 0, high_temperature = 0;
	signed int low_Q_max = 0, high_Q_max = 0;

	if (temperature <= batt_meter_cust_data.temperature_t1) {
		low_temperature = (-10);
		low_Q_max = g_Q_MAX_NEG_10_H_CURRENT[g_fg_battery_id];
		high_temperature = batt_meter_cust_data.temperature_t1;
		high_Q_max = g_Q_MAX_POS_0_H_CURRENT[g_fg_battery_id];

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else if (temperature <= batt_meter_cust_data.temperature_t2) {
		low_temperature = batt_meter_cust_data.temperature_t1;
		low_Q_max = g_Q_MAX_POS_0_H_CURRENT[g_fg_battery_id];
		high_temperature = batt_meter_cust_data.temperature_t2;
		high_Q_max = g_Q_MAX_POS_25_H_CURRENT[g_fg_battery_id];

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else {
		low_temperature = batt_meter_cust_data.temperature_t2;
		low_Q_max = g_Q_MAX_POS_25_H_CURRENT[g_fg_battery_id];
		high_temperature = batt_meter_cust_data.temperature_t3;
		high_Q_max = g_Q_MAX_POS_50_H_CURRENT[g_fg_battery_id];

		if (temperature > high_temperature)
			temperature = high_temperature;

	}

	ret_Q_max = low_Q_max + (((temperature - low_temperature) * (high_Q_max - low_Q_max)
				 ) / (high_temperature - low_temperature)
	    );

	return ret_Q_max;
}

#else
int BattThermistorConverTemp(int Res)
{
	int i = 0;
	int RES1 = 0, RES2 = 0;
	int TBatt_Value = -200, TMP1 = 0, TMP2 = 0;
	int table_size=0;
	table_size=sizeof(Batt_Temperature_Table)/sizeof(BATT_TEMPERATURE);
	if(Res>=Batt_Temperature_Table[0].TemperatureR)
	{
		TBatt_Value = Batt_Temperature_Table[0].BatteryTemp;
	}
	else if(Res<=Batt_Temperature_Table[table_size-1].TemperatureR)
	{
		TBatt_Value = Batt_Temperature_Table[table_size-1].BatteryTemp;;
	}
	else
	{
		RES1 = Batt_Temperature_Table[0].TemperatureR;
		TMP1 = Batt_Temperature_Table[0].BatteryTemp;

		for(i=0;i<table_size;i++)
		{
			if(Res>=Batt_Temperature_Table[i].TemperatureR)
			{
				RES2 = Batt_Temperature_Table[i].TemperatureR;
				TMP2 = Batt_Temperature_Table[i].BatteryTemp;
				break;
			}
			else
			{
				RES1 = Batt_Temperature_Table[i].TemperatureR;
				TMP1 = Batt_Temperature_Table[i].BatteryTemp;
			}
		}

		TBatt_Value = (((Res - RES2) * TMP1) + ((RES1 - Res) * TMP2)) / (RES1 - RES2);
	}

	return TBatt_Value;
}

signed int fgauge_get_Q_max(signed short temperature)
{
	signed int ret_Q_max = 0;
	signed int low_temperature = 0, high_temperature = 0;
	signed int low_Q_max = 0, high_Q_max = 0;

	if (temperature <= batt_meter_cust_data.temperature_t1) {
		low_temperature = (-10);
		low_Q_max = batt_meter_cust_data.q_max_neg_10;
		high_temperature = batt_meter_cust_data.temperature_t1;
		high_Q_max = batt_meter_cust_data.q_max_pos_0;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else if (temperature <= batt_meter_cust_data.temperature_t2) {
		low_temperature = batt_meter_cust_data.temperature_t1;
		low_Q_max = batt_meter_cust_data.q_max_pos_0;
		high_temperature = batt_meter_cust_data.temperature_t2;
		high_Q_max = batt_meter_cust_data.q_max_pos_25;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else {
		low_temperature = batt_meter_cust_data.temperature_t2;
		low_Q_max = batt_meter_cust_data.q_max_pos_25;
		high_temperature = batt_meter_cust_data.temperature_t3;
		high_Q_max = batt_meter_cust_data.q_max_pos_50;

		if (temperature > high_temperature)
			temperature = high_temperature;

	}

	ret_Q_max = low_Q_max + (((temperature - low_temperature) * (high_Q_max - low_Q_max)
				 ) / (high_temperature - low_temperature)
	    );

	return ret_Q_max;
}


signed int fgauge_get_Q_max_high_current(signed short temperature)
{
	signed int ret_Q_max = 0;
	signed int low_temperature = 0, high_temperature = 0;
	signed int low_Q_max = 0, high_Q_max = 0;

	if (temperature <= batt_meter_cust_data.temperature_t1) {
		low_temperature = (-10);
		low_Q_max = batt_meter_cust_data.q_max_neg_10_h_current;
		high_temperature = batt_meter_cust_data.temperature_t1;
		high_Q_max = batt_meter_cust_data.q_max_pos_0_h_current;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else if (temperature <= batt_meter_cust_data.temperature_t2) {
		low_temperature = batt_meter_cust_data.temperature_t1;
		low_Q_max = batt_meter_cust_data.q_max_pos_0_h_current;
		high_temperature = batt_meter_cust_data.temperature_t2;
		high_Q_max = batt_meter_cust_data.q_max_pos_25_h_current;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else {
		low_temperature = batt_meter_cust_data.temperature_t2;
		low_Q_max = batt_meter_cust_data.q_max_pos_25_h_current;
		high_temperature = batt_meter_cust_data.temperature_t3;
		high_Q_max = batt_meter_cust_data.q_max_pos_50_h_current;

		if (temperature > high_temperature)
			temperature = high_temperature;

	}

	ret_Q_max = low_Q_max + (((temperature - low_temperature) * (high_Q_max - low_Q_max)
				 ) / (high_temperature - low_temperature)
	    );

	return ret_Q_max;
}

#endif

int BattVoltToTemp(int dwVolt)
{
	long long TRes_temp;
	long long TRes;
	int sBaTTMP = -100;

	/* TRes_temp = ((long long)RBAT_PULL_UP_R*(long long)dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt); */
	/* TRes = (TRes_temp * (long long)RBAT_PULL_DOWN_R)/((long long)RBAT_PULL_DOWN_R - TRes_temp); */

	TRes_temp = (batt_meter_cust_data.rbat_pull_up_r * (long long) dwVolt);
	do_div(TRes_temp, (batt_meter_cust_data.rbat_pull_up_volt - dwVolt));

#ifdef RBAT_PULL_DOWN_R
	TRes = (TRes_temp * RBAT_PULL_DOWN_R);
	do_div(TRes, abs(RBAT_PULL_DOWN_R - TRes_temp));
#else
	TRes = TRes_temp;
#endif

	/* convert register to temperature */
	sBaTTMP = BattThermistorConverTemp((int)TRes);

	return sBaTTMP;
}

int force_get_tbat(kal_bool update)
{
#if defined(CONFIG_POWER_EXT) || defined(FIXED_TBAT_25)
	return 25;
#else
	int bat_temperature_volt = 0;
	int bat_temperature_val = 0;
	static int pre_bat_temperature_val = -1;
	int ret = 0;

	if (batt_meter_cust_data.fixed_tbat_25) {
		return 25;
	}

	if (update == KAL_TRUE || pre_bat_temperature_val == -1) {
		/* Get V_BAT_Temperature */
		bat_temperature_volt = 2;
		ret =
		    battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_TEMP, &bat_temperature_volt);

		if (bat_temperature_volt != 0) {
#if defined(SOC_BY_HW_FG)
			fg_r_value = get_r_fg_value();

			ret =
			    battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT,
					       &fg_current_temp);
			ret =
			    battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN,
					       &fg_current_state);
			fg_current_temp = fg_current_temp / 10;

			if (fg_current_state == KAL_TRUE) {
				bat_temperature_volt_temp = bat_temperature_volt;
				bat_temperature_volt =
				    bat_temperature_volt - ((fg_current_temp * fg_r_value) / 1000);
			} else {
				bat_temperature_volt_temp = bat_temperature_volt;
				bat_temperature_volt =
				    bat_temperature_volt + ((fg_current_temp * fg_r_value) / 1000);
			}
#endif

			bat_temperature_val = BattVoltToTemp(bat_temperature_volt);
		}
#ifdef CONFIG_MTK_BIF_SUPPORT
		battery_charging_control(CHARGING_CMD_GET_BIF_TBAT, &bat_temperature_val);
#endif
		pre_bat_temperature_val = bat_temperature_val;
	} else {
		bat_temperature_val = pre_bat_temperature_val;
	}
	return bat_temperature_val;
#endif
}
EXPORT_SYMBOL(force_get_tbat);

#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
int fgauge_get_saddles(void)
{
	return sizeof(battery_profile_temperature) / sizeof(BATTERY_PROFILE_STRUCT);
}

int fgauge_get_saddles_r_table(void)
{
	return sizeof(r_profile_temperature) / sizeof(R_PROFILE_STRUCT);
}

BATTERY_PROFILE_STRUCT_P fgauge_get_profile(unsigned int temperature)
{
	switch (temperature) {
	case batt_meter_cust_data.temperature_t0:
		return &battery_profile_t0[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t1:
		return &battery_profile_t1[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t2:
		return &battery_profile_t2[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t3:
		return &battery_profile_t3[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t:
		return &battery_profile_temperature[0];
		/*break;*/
	default:
		return NULL;
		/*break;*/
	}
}

R_PROFILE_STRUCT_P fgauge_get_profile_r_table(unsigned int temperature)
{
	switch (temperature) {
	case batt_meter_cust_data.temperature_t0:
		return &r_profile_t0[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t1:
		return &r_profile_t1[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t2:
		return &r_profile_t2[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t3:
		return &r_profile_t3[g_fg_battery_id][0];
		/*break;*/
	case batt_meter_cust_data.temperature_t:
		return &r_profile_temperature[0];
		/*break;*/
	default:
		return NULL;
		/*break;*/
	}
}
#else
int fgauge_get_saddles(void)
{
	return sizeof(battery_profile_t2) / sizeof(BATTERY_PROFILE_STRUCT);
}

int fgauge_get_saddles_r_table(void)
{
	return sizeof(r_profile_t2) / sizeof(R_PROFILE_STRUCT);
}

BATTERY_PROFILE_STRUCT_P fgauge_get_profile(unsigned int temperature)
{
	if (temperature == batt_meter_cust_data.temperature_t0)
		return &battery_profile_t0[0];

	if (temperature == batt_meter_cust_data.temperature_t1)
		return &battery_profile_t1[0];

	if (temperature == batt_meter_cust_data.temperature_t2)
		return &battery_profile_t2[0];

	if (temperature == batt_meter_cust_data.temperature_t3)
		return &battery_profile_t3[0];

	if (temperature == batt_meter_cust_data.temperature_t)
		return &battery_profile_temperature[0];


	return NULL;

}

R_PROFILE_STRUCT_P fgauge_get_profile_r_table(unsigned int temperature)
{
	if (temperature == batt_meter_cust_data.temperature_t0)
		return &r_profile_t0[0];

	if (temperature == batt_meter_cust_data.temperature_t1)
		return &r_profile_t1[0];

	if (temperature == batt_meter_cust_data.temperature_t2)
		return &r_profile_t2[0];

	if (temperature == batt_meter_cust_data.temperature_t3)
		return &r_profile_t3[0];

	if (temperature == batt_meter_cust_data.temperature_t)
		return &r_profile_temperature[0];

	return NULL;
}
#endif

signed int fgauge_read_capacity_by_v(signed int voltage)
{
	int i = 0, saddles = 0;
	BATTERY_PROFILE_STRUCT_P profile_p;
	signed int ret_percent = 0;

	profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t);
	if (profile_p == NULL) {
		return 100;
	}

	saddles = fgauge_get_saddles();

	if (voltage > (profile_p + 0)->voltage)
		return 100;	/* battery capacity, not dod */

	if (voltage < (profile_p + saddles - 1)->voltage)
		return 0;	/* battery capacity, not dod */


	for (i = 0; i < saddles - 1; i++) {
		if ((voltage <= (profile_p + i)->voltage)
		    && (voltage >= (profile_p + i + 1)->voltage)) {
			ret_percent =
			    (profile_p + i)->percentage +
			    (((((profile_p + i)->voltage) -
			       voltage) * (((profile_p + i + 1)->percentage) -
					   ((profile_p + i)->percentage))
			     ) / (((profile_p + i)->voltage) - ((profile_p + i + 1)->voltage))
			    );

			break;
		}

	}
	ret_percent = 100 - ret_percent;

	return ret_percent;
}

signed int fgauge_read_v_by_capacity(int bat_capacity)
{
	int i = 0, saddles = 0;
	BATTERY_PROFILE_STRUCT_P profile_p;
	signed int ret_volt = 0;

	profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t);
	if (profile_p == NULL) {
		return 3700;
	}

	saddles = fgauge_get_saddles();

	if (bat_capacity < (profile_p + 0)->percentage)
		return 3700;

	if (bat_capacity > (profile_p + saddles - 1)->percentage)
		return 3700;


	for (i = 0; i < saddles - 1; i++) {
		if ((bat_capacity >= (profile_p + i)->percentage)
		    && (bat_capacity <= (profile_p + i + 1)->percentage)) {
			ret_volt =
			    (profile_p + i)->voltage -
			    (((bat_capacity -
			       ((profile_p + i)->percentage)) * (((profile_p + i)->voltage) -
								 ((profile_p + i + 1)->voltage))
			     ) / (((profile_p + i + 1)->percentage) - ((profile_p + i)->percentage))
			    );

			break;
		}
	}

	return ret_volt;
}

signed int fgauge_read_d_by_v(signed int volt_bat)
{
	int i = 0, saddles = 0;
	BATTERY_PROFILE_STRUCT_P profile_p;
	signed int ret_d = 0;

	profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t);
	if (profile_p == NULL) {
		return 100;
	}

	saddles = fgauge_get_saddles();

	if (volt_bat > (profile_p + 0)->voltage)
		return 0;

	if (volt_bat < (profile_p + saddles - 1)->voltage)
		return 100;


	for (i = 0; i < saddles - 1; i++) {
		if ((volt_bat <= (profile_p + i)->voltage)
		    && (volt_bat >= (profile_p + i + 1)->voltage)) {
			ret_d =
			    (profile_p + i)->percentage +
			    (((((profile_p + i)->voltage) -
			       volt_bat) * (((profile_p + i + 1)->percentage) -
					    ((profile_p + i)->percentage))
			     ) / (((profile_p + i)->voltage) - ((profile_p + i + 1)->voltage))
			    );

			break;
		}

	}

	return ret_d;
}

signed int fgauge_read_v_by_d(int d_val)
{
	int i = 0, saddles = 0;
	BATTERY_PROFILE_STRUCT_P profile_p;
	signed int ret_volt = 0;

	profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t);
	if (profile_p == NULL) {
		return 3700;
	}

	saddles = fgauge_get_saddles();

	if (d_val < (profile_p + 0)->percentage)
		return 3700;

	if (d_val > (profile_p + saddles - 1)->percentage)
		return 3700;


	for (i = 0; i < saddles - 1; i++) {
		if ((d_val >= (profile_p + i)->percentage)
		    && (d_val <= (profile_p + i + 1)->percentage)) {
			ret_volt =
			    (profile_p + i)->voltage -
			    (((d_val -
			       ((profile_p + i)->percentage)) * (((profile_p + i)->voltage) -
								 ((profile_p + i + 1)->voltage))
			     ) / (((profile_p + i + 1)->percentage) - ((profile_p + i)->percentage))
			    );

			break;
		}
	}

	return ret_volt;
}

signed int fgauge_read_r_bat_by_v(signed int voltage)
{
	int i = 0, saddles = 0;
	R_PROFILE_STRUCT_P profile_p;
	signed int ret_r = 0;

	profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t);
	if (profile_p == NULL) {
		return (profile_p + 0)->resistance;
	}

	saddles = fgauge_get_saddles_r_table();

	if (voltage > (profile_p + 0)->voltage)
		return (profile_p + 0)->resistance;

	if (voltage < (profile_p + saddles - 1)->voltage)
		return (profile_p + saddles - 1)->resistance;


	for (i = 0; i < saddles - 1; i++) {
		if ((voltage <= (profile_p + i)->voltage)
		    && (voltage >= (profile_p + i + 1)->voltage)) {
			ret_r =
			    (profile_p + i)->resistance +
			    (((((profile_p + i)->voltage) -
			       voltage) * (((profile_p + i + 1)->resistance) -
					   ((profile_p + i)->resistance))
			     ) / (((profile_p + i)->voltage) - ((profile_p + i + 1)->voltage))
			    );
			break;
		}
	}

	return ret_r;
}

void fgauge_construct_battery_profile_init(void)
{
	BATTERY_PROFILE_STRUCT_P temp_profile_p, profile_p[PROFILE_SIZE];
	int i, j, saddles, profile_index;
	signed int low_p = 0, high_p = 0, now_p = 0, low_vol = 0, high_vol = 0;

	profile_p[0] = fgauge_get_profile(batt_meter_cust_data.temperature_t0);
	profile_p[1] = fgauge_get_profile(batt_meter_cust_data.temperature_t1);
	profile_p[2] = fgauge_get_profile(batt_meter_cust_data.temperature_t2);
	profile_p[3] = fgauge_get_profile(batt_meter_cust_data.temperature_t3);
	saddles = fgauge_get_saddles();
	temp_profile_p =
	    (BATTERY_PROFILE_STRUCT_P) kmalloc(51 * sizeof(*temp_profile_p), GFP_KERNEL);
	memset(temp_profile_p, 0, 51 * sizeof(*temp_profile_p));
	for (i = 0; i < PROFILE_SIZE; i++) {
		profile_index = 0;
		for (j = 0; j * 2 <= 100; j++) {
			while (profile_index < saddles && profile_index >= 0) {
				if (((profile_p[i] + profile_index)->percentage) < j * 2) {
					profile_index++;
					continue;
				} else if (((profile_p[i] + profile_index)->percentage) == j * 2) {
					(temp_profile_p + j)->voltage =
					    (profile_p[i] + profile_index)->voltage;
					(temp_profile_p + j)->percentage =
					    (profile_p[i] + profile_index)->percentage;
					break;
				}
				low_p = (profile_p[i] + profile_index - 1)->percentage;
				high_p = (profile_p[i] + profile_index)->percentage;
				now_p = j * 2;
				low_vol = (profile_p[i] + profile_index)->voltage;
				high_vol = (profile_p[i] + profile_index - 1)->voltage;
				(temp_profile_p + j)->voltage =
				    (low_vol * 1000 +
				     ((high_vol - low_vol) * 1000 * (now_p - low_p) / (high_p -
										       low_p))) /
				    1000;
				(temp_profile_p + j)->percentage = j * 2;

				break;
			}
		}

		for (j = 0; j * 2 <= 100; j++) {
			(profile_p[i] + j)->voltage = (temp_profile_p + j)->voltage;
			(profile_p[i] + j)->percentage = (temp_profile_p + j)->percentage;
		}
	}
	kfree(temp_profile_p);
}

void fgauge_construct_battery_profile(signed int temperature, BATTERY_PROFILE_STRUCT_P temp_profile_p)
{
	BATTERY_PROFILE_STRUCT_P low_profile_p, high_profile_p;
	signed int low_temperature, high_temperature;
	int i, saddles;
	signed int temp_v_1 = 0, temp_v_2 = 0;

	if (temperature <= batt_meter_cust_data.temperature_t1) {
		low_profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t0);
		high_profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t1);
		low_temperature = (-10);
		high_temperature = batt_meter_cust_data.temperature_t1;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else if (temperature <= batt_meter_cust_data.temperature_t2) {
		low_profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t1);
		high_profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t2);
		low_temperature = batt_meter_cust_data.temperature_t1;
		high_temperature = batt_meter_cust_data.temperature_t2;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else {
		low_profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t2);
		high_profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t3);
		low_temperature = batt_meter_cust_data.temperature_t2;
		high_temperature = batt_meter_cust_data.temperature_t3;

		if (temperature > high_temperature)
			temperature = high_temperature;

	}

	saddles = fgauge_get_saddles();

	for (i = 0; i < saddles; i++) {
		if (((high_profile_p + i)->voltage) > ((low_profile_p + i)->voltage)) {
			temp_v_1 = (high_profile_p + i)->voltage;
			temp_v_2 = (low_profile_p + i)->voltage;

			(temp_profile_p + i)->voltage = temp_v_2 +
			    (((temperature - low_temperature) * (temp_v_1 - temp_v_2)
			     ) / (high_temperature - low_temperature)
			    );
		} else {
			temp_v_1 = (low_profile_p + i)->voltage;
			temp_v_2 = (high_profile_p + i)->voltage;

			(temp_profile_p + i)->voltage = temp_v_2 +
			    (((high_temperature - temperature) * (temp_v_1 - temp_v_2)
			     ) / (high_temperature - low_temperature)
			    );
		}

		(temp_profile_p + i)->percentage = (high_profile_p + i)->percentage;
	}

}

void fgauge_construct_r_table_profile(signed int temperature, R_PROFILE_STRUCT_P temp_profile_p)
{
	R_PROFILE_STRUCT_P low_profile_p, high_profile_p;
	signed int low_temperature, high_temperature;
	int i, saddles;
	signed int temp_v_1 = 0, temp_v_2 = 0;
	signed int temp_r_1 = 0, temp_r_2 = 0;

	if (temperature <= batt_meter_cust_data.temperature_t1) {
		low_profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t0);
		high_profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t1);
		low_temperature = (-10);
		high_temperature = batt_meter_cust_data.temperature_t1;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else if (temperature <= batt_meter_cust_data.temperature_t2) {
		low_profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t1);
		high_profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t2);
		low_temperature = batt_meter_cust_data.temperature_t1;
		high_temperature = batt_meter_cust_data.temperature_t2;

		if (temperature < low_temperature)
			temperature = low_temperature;

	} else {
		low_profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t2);
		high_profile_p = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t3);
		low_temperature = batt_meter_cust_data.temperature_t2;
		high_temperature = batt_meter_cust_data.temperature_t3;

		if (temperature > high_temperature)
			temperature = high_temperature;

	}

	saddles = fgauge_get_saddles_r_table();

	/* Interpolation for V_BAT */
	for (i = 0; i < saddles; i++) {
		if (((high_profile_p + i)->voltage) > ((low_profile_p + i)->voltage)) {
			temp_v_1 = (high_profile_p + i)->voltage;
			temp_v_2 = (low_profile_p + i)->voltage;

			(temp_profile_p + i)->voltage = temp_v_2 +
			    (((temperature - low_temperature) * (temp_v_1 - temp_v_2)
			     ) / (high_temperature - low_temperature)
			    );
		} else {
			temp_v_1 = (low_profile_p + i)->voltage;
			temp_v_2 = (high_profile_p + i)->voltage;

			(temp_profile_p + i)->voltage = temp_v_2 +
			    (((high_temperature - temperature) * (temp_v_1 - temp_v_2)
			     ) / (high_temperature - low_temperature)
			    );
		}

#if 0
		/* (temp_profile_p + i)->resistance = (high_profile_p + i)->resistance; */

		(temp_profile_p + i)->voltage = temp_v_2 +
		    (((temperature - low_temperature) * (temp_v_1 - temp_v_2)
		     ) / (high_temperature - low_temperature)
		    );
#endif
	}

	/* Interpolation for R_BAT */
	for (i = 0; i < saddles; i++) {
		if (((high_profile_p + i)->resistance) > ((low_profile_p + i)->resistance)) {
			temp_r_1 = (high_profile_p + i)->resistance;
			temp_r_2 = (low_profile_p + i)->resistance;

			(temp_profile_p + i)->resistance = temp_r_2 +
			    (((temperature - low_temperature) * (temp_r_1 - temp_r_2)
			     ) / (high_temperature - low_temperature)
			    );
		} else {
			temp_r_1 = (low_profile_p + i)->resistance;
			temp_r_2 = (high_profile_p + i)->resistance;

			(temp_profile_p + i)->resistance = temp_r_2 +
			    (((high_temperature - temperature) * (temp_r_1 - temp_r_2)
			     ) / (high_temperature - low_temperature)
			    );
		}

	}

}

void fgauge_construct_table_by_temp(void)
{
#if defined(CONFIG_POWER_EXT)
#else
	unsigned int i;
	static signed int init_temp = KAL_TRUE;
	static signed int curr_temp, last_temp, avg_temp;
	static signed int battTempBuffer[TEMP_AVERAGE_SIZE];
	static signed int temperature_sum;

	static unsigned char tempIndex;

	curr_temp = battery_meter_get_battery_temperature();

	/* Temperature window init */
	if (init_temp == KAL_TRUE) {
		for (i = 0; i < TEMP_AVERAGE_SIZE; i++)
			battTempBuffer[i] = curr_temp;

		last_temp = curr_temp;
		temperature_sum = curr_temp * TEMP_AVERAGE_SIZE;
		init_temp = KAL_FALSE;
	}
	/* Temperature sliding window */
	temperature_sum -= battTempBuffer[tempIndex];
	temperature_sum += curr_temp;
	battTempBuffer[tempIndex] = curr_temp;
	avg_temp = (temperature_sum) / TEMP_AVERAGE_SIZE;

	if (avg_temp != last_temp) {
		fgauge_construct_r_table_profile(curr_temp,
						 fgauge_get_profile_r_table
						 (batt_meter_cust_data.temperature_t));
		fgauge_construct_battery_profile(curr_temp,
						 fgauge_get_profile
						 (batt_meter_cust_data.temperature_t));
		last_temp = avg_temp;
		temperature_change = 1;
	}

	tempIndex = (tempIndex + 1) % TEMP_AVERAGE_SIZE;

#endif
}

#if defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
/*
	ZCV table is created by 600mA loading.
	Here we calculate average current and get a factor based on 600mA.
*/
void fgauge_get_current_factor(void)
{
#if defined(CONFIG_POWER_EXT)
#else
	unsigned int i;
	static signed int init_current = KAL_TRUE;
	static signed int inst_current, avg_current;
	static signed int battCurrentBuffer[TEMP_AVERAGE_SIZE];
	static signed int current_sum;
	static unsigned char tempcurrentIndex;

	if (KAL_TRUE == gFG_Is_Charging) {
		init_current = KAL_TRUE;
		g_currentfactor = 100;
		return;
	}

	inst_current = gFG_current;

	if (init_current == KAL_TRUE) {
		for (i = 0; i < TEMP_AVERAGE_SIZE; i++)
			battCurrentBuffer[i] = inst_current;

		current_sum = inst_current * TEMP_AVERAGE_SIZE;
		init_current = KAL_FALSE;
	}

	/* current sliding window */
	current_sum -= battCurrentBuffer[tempcurrentIndex];
	current_sum += inst_current;
	battCurrentBuffer[tempcurrentIndex] = inst_current;
	avg_current = (current_sum) / TEMP_AVERAGE_SIZE;

	g_currentfactor = avg_current * 100 / CV_CURRENT;	/* calculate factor by 600ma */

	tempcurrentIndex = (tempcurrentIndex + 1) % TEMP_AVERAGE_SIZE;
#endif
}

/*
	ZCV table has battery OCV-to-resistance information.
	Based on a given discharging current value, we can get a new estimated Qmax.
	Qmax is defined as OCV -I*R < power off voltage.
	Default power off voltage is 3400mV.
*/

signed int fgauge_get_Q_max_high_current_by_current(signed int i_current, signed short val_temp)
{
	signed int ret_Q_max = 0;
	signed int iIndex = 0, saddles = 0;
	signed int OCV_temp = 0, Rbat_temp = 0, V_drop = 0;
	R_PROFILE_STRUCT_P p_profile_r;
	BATTERY_PROFILE_STRUCT_P p_profile_battery;
	signed int threshold = SYSTEM_OFF_VOLTAGE;
	/* for Qmax initialization */
	ret_Q_max = fgauge_get_Q_max_high_current(val_temp);

	/* get Rbat and OCV table of the current temperature */
	p_profile_r = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t);
	p_profile_battery = fgauge_get_profile(batt_meter_cust_data.temperature_t);
	if (p_profile_r == NULL || p_profile_battery == NULL) {
		return ret_Q_max;
	}

	if (0 == p_profile_r->resistance || 0 == p_profile_battery->voltage) {
		return ret_Q_max;
	}

	saddles = fgauge_get_saddles();

	/* get Qmax in current temperature (>3.4) */
	for (iIndex = 0; iIndex < saddles - 1; iIndex++) {
		OCV_temp = (p_profile_battery + iIndex)->voltage;
		Rbat_temp = (p_profile_r + iIndex)->resistance;
		V_drop = (i_current * Rbat_temp) / 10000;

		if (OCV_temp - V_drop < threshold) {
			if (iIndex <= 1)
				ret_Q_max = STEP_OF_QMAX;
			else
				ret_Q_max = (iIndex - 1) * STEP_OF_QMAX;
			break;
		}
	}

	return ret_Q_max;
}
#endif

void fg_qmax_update_for_aging(void)
{
#if defined(CONFIG_POWER_EXT)
#else
	kal_bool hw_charging_done = bat_is_charging_full();

	if (hw_charging_done == KAL_TRUE) {	/* charging full, g_HW_Charging_Done == 1 */
		if (gFG_DOD0 > 85) {
			if (gFG_columb < 0)
				gFG_columb = gFG_columb - gFG_columb * 2;	/* absolute value */

			gFG_BATT_CAPACITY_aging =
			    (((gFG_columb * 1000) + (5 * gFG_DOD0)) / gFG_DOD0) / 10;

			/* tuning */
			gFG_BATT_CAPACITY_aging =
			    (gFG_BATT_CAPACITY_aging * 100) /
			    batt_meter_cust_data.aging_tuning_value;

			if (gFG_BATT_CAPACITY_aging == 0) {
				gFG_BATT_CAPACITY_aging =
				    fgauge_get_Q_max(battery_meter_get_battery_temperature());
			}

		}
	}
#endif
}


#if defined(SW_OAM_INIT_V2)
char bootbuf[100];
void sw_oam_init_v2(void)
{
	int ret = 0;
	int plugout_status = 0;
	int type = 0;

	/* use get_hw_ocv----------------------------------------------------------------- */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);
	gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);

#if defined(CONFIG_POWER_EXT)
	g_rtc_fg_soc = gFG_capacity_by_v;
#else
	g_rtc_fg_soc = get_rtc_spare_fg_value();
#endif

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_BATTERY_PLUG_STATUS, &plugout_status);

	if (plugout_status == 0 && bat_is_charger_exist() == KAL_FALSE) {
		if (g_rtc_fg_soc == 0) {
			gFG_capacity_by_v = gFG_capacity_by_v_init;	/* g_booting_vbat */
			type = 1;
		} else {
			gFG_capacity_by_v = g_rtc_fg_soc;
			type = 2;
		}
	} else {
		if ((abs(gFG_capacity_by_v - g_rtc_fg_soc) >
		     batt_meter_cust_data.cust_poweron_delta_capacity_tolrance)
		    && (abs(gFG_capacity_by_v - gFG_capacity_by_v_init) <
			abs(gFG_capacity_by_v_init - g_rtc_fg_soc))) {
			if (abs(gFG_capacity_by_v - gFG_capacity_by_v_init) >
			    batt_meter_cust_data.cust_poweron_delta_hw_sw_ocv_capacity_tolrance) {
				gFG_capacity_by_v = gFG_capacity_by_v_init;
				type = 3;
			} else {
				/* use hw ocv; */
				type = 4;
			}

		} else {
			if ((abs(g_rtc_fg_soc - gFG_capacity_by_v_init) >
			     batt_meter_cust_data.cust_poweron_delta_hw_sw_ocv_capacity_tolrance)
			    || g_rtc_fg_soc == 0) {
				gFG_capacity_by_v = gFG_capacity_by_v_init;
				type = 5;
			} else {
				gFG_capacity_by_v = g_rtc_fg_soc;
				type = 6;
			}
		}
	}

}
#endif

#ifdef TINNO_GET_OCV_FORM_LK
#define BAT_VOL_STR  "tinno_vbat="
extern char *saved_command_line;
#define LK_CURRENT 90
static int str_to_int(char *str)
{
	int count=0;
	int value=0;
	int t_index=0;
	for(count=0;count<15;count++)
	{
		if((str[count]>='0')&&(str[count]<='9'))
		{
			continue;
		}
		break;
	}

	for(;count>0;count--)
	{
	
		if(count>1)
		{
			value=value*10+(str[t_index]-'0')*10;
		}else{
			value=value+(str[t_index]-'0');
		}
		t_index++;
	}
	return value;
}

int  get_battery_voltage_in_cmdline(char * start)
{
	char buf[15];
	unsigned int temp_voltage=0,i;
	memset(buf,0,15);
	for(i=0;i<15;i++)
	{
		if(start[i]=='m')
		{
			break;
		}
		buf[i]=start[i];
	}
	temp_voltage=str_to_int(buf);
	return temp_voltage;
}

int track_bat_voltage(int voltage,int current_temp)
{
	int rbat=0;
	int ocv_vbat=0;
	int delta_v=0;
	rbat = fgauge_read_r_bat_by_v(voltage);	/* Ohm */
	delta_v = (current_temp * (rbat + R_FG_VALUE)) / 1000;
	ocv_vbat = voltage+delta_v;
	printk("track_bat_voltage rbat=%d \n",rbat);
	return ocv_vbat;
}
int tinno_get_bat_ocv(void)
{
	int aboot_batvol=0;
	int hw_ocv=1;
	char * start;
	battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &hw_ocv);
	start=strstr(saved_command_line,BAT_VOL_STR);
	    if(start)
	    {
		  aboot_batvol=get_battery_voltage_in_cmdline(start+strlen(BAT_VOL_STR));
	    }else{
		   aboot_batvol=-1;
		}
		
	 if(aboot_batvol>200000) //charging boot mode!
	{
		aboot_batvol=aboot_batvol/100;
		aboot_batvol=track_bat_voltage(aboot_batvol,-360);
	}else if(aboot_batvol>20000) //charging boot mode!,but has disconnected the chg! 
		{
			aboot_batvol=aboot_batvol/10;
			aboot_batvol=track_bat_voltage(aboot_batvol,LK_CURRENT);
		}else{ //no charger ,use hw ocv!
			aboot_batvol=track_bat_voltage(aboot_batvol,LK_CURRENT);
			/*
			if((aboot_batvol>hw_ocv+15)||(hw_ocv>aboot_batvol+15))
			{
				printk("hw ocv is error ,use aboot_batvol!");
			}else{
				aboot_batvol=hw_ocv;
				printk("no chg,use ocv voltage!");
				}
*/
				printk("keep use lk ocv voltage!");			
		}
		printk("aboot_batvol=%d hw_ocv=%d \n",aboot_batvol,hw_ocv);
		return aboot_batvol;
}
#endif

void dod_init(void)
{
#if defined(SOC_BY_HW_FG)
	int ret = 0;

#if defined(IS_BATTERY_REMOVE_BY_PMIC)
	signed int gFG_capacity_by_sw_ocv = gFG_capacity_by_v;
#endif				/* #if defined(IS_BATTERY_REMOVE_BY_PMIC) */

	/* use get_hw_ocv----------------------------------------------------------------- */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);
	gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);

#if defined(EXTERNAL_SWCHR_SUPPORT)
	/* compare with hw_ocv & sw_ocv, check if less than or equal to 5% tolerance */
	if ((abs(gFG_capacity_by_v_init - gFG_capacity_by_v) > 5)
	    && (bat_is_charger_exist() == KAL_TRUE)) {
		gFG_capacity_by_v = gFG_capacity_by_v_init;
	}
#endif
#if defined(HW_FG_FORCE_USE_SW_OCV)
	gFG_capacity_by_v = gFG_capacity_by_v_init;
#endif
	/* ------------------------------------------------------------------------------- */
#endif

#if defined(CONFIG_POWER_EXT)
	g_rtc_fg_soc = gFG_capacity_by_v;
#else
	g_rtc_fg_soc = get_rtc_spare_fg_value();
#endif


#if defined(IS_BATTERY_REMOVE_BY_PMIC)
	if (is_battery_remove_pmic() == 0 && (g_rtc_fg_soc != 0)
		&& batt_meter_cust_data.vbat_remove_detection) {
		gFG_capacity_by_v = g_rtc_fg_soc;
	} else {

#if defined(INIT_SOC_BY_SW_SOC)
if (((g_rtc_fg_soc != 0)
		     &&
		     (((abs(g_rtc_fg_soc - gFG_capacity_by_v)) <=
		       batt_meter_cust_data.cust_poweron_delta_capacity_tolrance)
		      || (abs(gFG_capacity_by_v_init - g_rtc_fg_soc) <
			  abs(gFG_capacity_by_v - gFG_capacity_by_v_init))))
		    || ((g_rtc_fg_soc != 0)
			&& (get_boot_reason() == BR_WDT_BY_PASS_PWK || get_boot_reason() == BR_WDT
			    || get_boot_reason() == BR_TOOL_BY_PASS_PWK
			    || get_boot_reason() == BR_2SEC_REBOOT || get_boot_mode() == RECOVERY_BOOT)))
#else
if (((g_rtc_fg_soc != 0)
		     &&
		     (((abs(g_rtc_fg_soc - gFG_capacity_by_v)) <
		       batt_meter_cust_data.cust_poweron_delta_capacity_tolrance))
		     &&
		     ((gFG_capacity_by_v > batt_meter_cust_data.cust_poweron_low_capacity_tolrance
		       || bat_is_charger_exist() == KAL_TRUE)))
		    || ((g_rtc_fg_soc != 0)
			&& (get_boot_reason() == BR_WDT_BY_PASS_PWK || get_boot_reason() == BR_WDT
			    || get_boot_reason() == BR_TOOL_BY_PASS_PWK
			    || get_boot_reason() == BR_2SEC_REBOOT || get_boot_mode() == RECOVERY_BOOT)))
#endif
		{
			gFG_capacity_by_v = g_rtc_fg_soc;
		} else {
			if (abs(gFG_capacity_by_v - gFG_capacity_by_sw_ocv) >
			    batt_meter_cust_data.cust_poweron_delta_hw_sw_ocv_capacity_tolrance) {
				gFG_capacity_by_v = gFG_capacity_by_sw_ocv;
			}
		}

	}

#else

#if defined(SOC_BY_HW_FG)
#if defined(INIT_SOC_BY_SW_SOC)
if (((g_rtc_fg_soc != 0)
	     &&
	     (((abs(g_rtc_fg_soc - gFG_capacity_by_v)) <=
	       batt_meter_cust_data.cust_poweron_delta_capacity_tolrance)
	      || (abs(gFG_capacity_by_v_init - g_rtc_fg_soc) <
		  abs(gFG_capacity_by_v - gFG_capacity_by_v_init))))
	    || ((g_rtc_fg_soc != 0)
		&& (get_boot_reason() == BR_WDT_BY_PASS_PWK || get_boot_reason() == BR_WDT
		    || get_boot_reason() == BR_TOOL_BY_PASS_PWK || get_boot_reason() == BR_2SEC_REBOOT
		    || get_boot_mode() == RECOVERY_BOOT)))
#else
if (((g_rtc_fg_soc != 0)
	     &&
	     (((abs(g_rtc_fg_soc - gFG_capacity_by_v)) <
	       batt_meter_cust_data.cust_poweron_delta_capacity_tolrance))
	     &&
	     ((gFG_capacity_by_v > batt_meter_cust_data.cust_poweron_low_capacity_tolrance
	       || bat_is_charger_exist() == KAL_TRUE)))
	    || ((g_rtc_fg_soc != 0)
		&& (get_boot_reason() == BR_WDT_BY_PASS_PWK || get_boot_reason() == BR_WDT
		    || get_boot_reason() == BR_TOOL_BY_PASS_PWK || get_boot_reason() == BR_2SEC_REBOOT
		    || get_boot_mode() == RECOVERY_BOOT)))
#endif
	{
		gFG_capacity_by_v = g_rtc_fg_soc;
	}
#elif defined(SOC_BY_SW_FG)
	if (((g_rtc_fg_soc != 0)
	     &&
	     (((abs(g_rtc_fg_soc - gFG_capacity_by_v)) <
	       batt_meter_cust_data.cust_poweron_delta_capacity_tolrance)
	      || (abs(g_rtc_fg_soc - g_booting_vbat) <
		  batt_meter_cust_data.cust_poweron_delta_capacity_tolrance))
	     &&
	     ((gFG_capacity_by_v > batt_meter_cust_data.cust_poweron_low_capacity_tolrance
	       || bat_is_charger_exist() == KAL_TRUE)))
	    || ((g_rtc_fg_soc != 0)
		&& (get_boot_reason() == BR_WDT_BY_PASS_PWK || get_boot_reason() == BR_WDT
		    || get_boot_reason() == BR_TOOL_BY_PASS_PWK || get_boot_reason() == BR_2SEC_REBOOT
		    || get_boot_mode() == RECOVERY_BOOT))) {
		gFG_capacity_by_v = g_rtc_fg_soc;
	}
#endif
#endif

#if defined(SW_OAM_INIT_V2)
	sw_oam_init_v2();
#endif
#ifdef TINNO_GET_OCV_FORM_LK
#ifndef IS_BATTERY_REMOVE_BY_PMIC
	kal_int32 gFG_capacity_by_sw_ocv;	
#endif 
	gFG_voltage = tinno_get_bat_ocv();
	gFG_capacity_by_sw_ocv= fgauge_read_capacity_by_v(gFG_voltage);
	g_rtc_fg_soc = get_rtc_spare_fg_value();
	
	if((g_rtc_fg_soc<=0)||(g_rtc_fg_soc>100)) //rtc soc is invalid, set sw soc to rtc soc. 
	{
		g_rtc_fg_soc=gFG_capacity_by_sw_ocv;
	}
	
	if(gFG_capacity_by_sw_ocv<5) //sw soc <5,force use sw soc!!!
	{
		gFG_capacity_by_v=gFG_capacity_by_sw_ocv;
	}else{
	 //delta soc >25 jump to sw soc!
		 if((gFG_capacity_by_sw_ocv>g_rtc_fg_soc+25)||(g_rtc_fg_soc>gFG_capacity_by_sw_ocv+25))
		 {
				gFG_capacity_by_v=gFG_capacity_by_sw_ocv;
		 }else{
		 //delta soc >15  reduce or not change.
		 		 if((gFG_capacity_by_sw_ocv>g_rtc_fg_soc+15)||(g_rtc_fg_soc>gFG_capacity_by_sw_ocv+15))
		 		 {
		 		  //rtc soc >sw soc	  reduce soc .
		 		 	if(g_rtc_fg_soc>gFG_capacity_by_sw_ocv)
		 		 	{
						gFG_capacity_by_v=g_rtc_fg_soc-2;
		 		 	}else{ //sw soc >rtc soc ,keep rtc soc.
						gFG_capacity_by_v=g_rtc_fg_soc;
					}
				 }else{ //delta soc <10,use rtc soc!
					gFG_capacity_by_v=g_rtc_fg_soc;
				 }
				
			}
	}

	printk("g_rtc_fg_soc=%d, gFG_capacity_by_v=%d",g_rtc_fg_soc,gFG_capacity_by_v);
#endif

	if (gFG_capacity_by_v == 0 && bat_is_charger_exist() == KAL_TRUE) {
		gFG_capacity_by_v = 1;
	}
	gFG_capacity = gFG_capacity_by_v;
	gFG_capacity_by_c_init = gFG_capacity;
	gFG_capacity_by_c = gFG_capacity;

	gFG_DOD0 = 100 - gFG_capacity;
	gFG_DOD1 = gFG_DOD0;

	gfg_percent_check_point = gFG_capacity;

	if (batt_meter_cust_data.change_tracking_point) {
		gFG_15_vlot = fgauge_read_v_by_capacity((100 - g_tracking_point));
	} else {
		/* gFG_15_vlot = fgauge_read_v_by_capacity(86); //14% */
		gFG_15_vlot = fgauge_read_v_by_capacity((100 - g_tracking_point));
		if ((gFG_15_vlot > 3800) || (gFG_15_vlot < 3600)) {
			gFG_15_vlot = 3700;
		}
	}
}

/* ============================================================ // SW FG */
signed int mtk_imp_tracking(signed int ori_voltage, signed int ori_current, signed int recursion_time)
{
	signed int ret_compensate_value = 0;
	signed int temp_voltage_1 = ori_voltage;
	signed int temp_voltage_2 = temp_voltage_1;
	int i = 0;

	for (i = 0; i < recursion_time; i++) {
		gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2);
		ret_compensate_value =
		    ((ori_current) * (gFG_resistance_bat + batt_meter_cust_data.r_fg_value)) / 1000;
		ret_compensate_value = (ret_compensate_value + (10 / 2)) / 10;
		temp_voltage_2 = temp_voltage_1 + ret_compensate_value;
	}

	gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2);
	ret_compensate_value =
	    ((ori_current) *
	     (gFG_resistance_bat + batt_meter_cust_data.r_fg_value +
	      batt_meter_cust_data.fg_meter_resistance)) / 1000;
	ret_compensate_value = (ret_compensate_value + (10 / 2)) / 10;

	gFG_compensate_value = ret_compensate_value;

	return ret_compensate_value;
}

void oam_init(void)
{
	int ret = 0;
	signed int vbat_capacity = 0;
	kal_bool charging_enable = KAL_FALSE;

	/*stop charging for vbat measurement */
	battery_charging_control(CHARGING_CMD_ENABLE, &charging_enable);

	msleep(50);

	g_booting_vbat = 5;	/* set avg times */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &g_booting_vbat);


	gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);
	vbat_capacity = fgauge_read_capacity_by_v(g_booting_vbat);

	if (bat_is_charger_exist() == KAL_TRUE) {
		/* to avoid plug in cable without battery, then plug in battery to make hw soc = 100% */
		/* if the difference bwtween ZCV and vbat is too large, using vbat instead ZCV */
		if (((gFG_capacity_by_v == 100)
		     && (vbat_capacity < batt_meter_cust_data.cust_poweron_max_vbat_tolrance))
		    || (abs(gFG_capacity_by_v - vbat_capacity) >
			batt_meter_cust_data.cust_poweron_delta_vbat_tolrance)) {
			gFG_voltage = g_booting_vbat;
			gFG_capacity_by_v = vbat_capacity;
		}
	}

	gFG_capacity_by_v_init = gFG_capacity_by_v;

	dod_init();

	gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(force_get_tbat(KAL_FALSE));

	/* oam_v_ocv_1 = gFG_voltage; */
	/* oam_v_ocv_2 = gFG_voltage; */


	oam_v_ocv_init = fgauge_read_v_by_d(gFG_DOD0);
	oam_v_ocv_2 = oam_v_ocv_1 = oam_v_ocv_init;
	g_vol_bat_hw_ocv = gFG_voltage;

	/* vbat = 5; //set avg times */
	/* ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &vbat); */
	/* oam_r_1 = fgauge_read_r_bat_by_v(vbat); */
	oam_r_1 = fgauge_read_r_bat_by_v(gFG_voltage);
	oam_r_2 = oam_r_1;

	oam_d0 = gFG_DOD0;
	oam_d_5 = oam_d0;
	oam_i_ori = gFG_current;
	g_d_hw_ocv = oam_d0;

	if (oam_init_i == 0) {
		oam_init_i = 1;
	}

}

void oam_run(void)
{
	int vol_bat = 0;
	int ret = 0;
	struct timespec now_time;
	signed int delta_time = 0;

	get_monotonic_boottime(&now_time);

	delta_time = now_time.tv_sec - last_oam_run_time.tv_sec;

#if defined(SW_OAM_INIT_V2)
	printk(bootbuf);
#endif

	last_oam_run_time = now_time;

	/* Reconstruct table if temp changed; */
	fgauge_construct_table_by_temp();

#if defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
	fgauge_get_current_factor();
#endif

	vol_bat = 15;		/* set avg times */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &vol_bat);

	oam_i_1 = (((oam_v_ocv_1 - vol_bat) * 1000) * 10) / oam_r_1;	/* 0.1mA */
	oam_i_2 = (((oam_v_ocv_2 - vol_bat) * 1000) * 10) / oam_r_2;	/* 0.1mA */

	oam_car_1 = (oam_i_1 * delta_time / 3600) + oam_car_1;	/* 0.1mAh */
	oam_car_2 = (oam_i_2 * delta_time / 3600) + oam_car_2;	/* 0.1mAh */

	oam_d_1 = oam_d0 + (oam_car_1 * 100 / 10) / gFG_BATT_CAPACITY_aging;
	if (oam_d_1 < 0)
		oam_d_1 = 0;
	if (oam_d_1 > 100)
		oam_d_1 = 100;

	oam_d_2 = oam_d0 + (oam_car_2 * 100 / 10) / gFG_BATT_CAPACITY_aging;
	if (oam_d_2 < 0)
		oam_d_2 = 0;
	if (oam_d_2 > 100)
		oam_d_2 = 100;

	oam_v_ocv_1 = vol_bat + mtk_imp_tracking(vol_bat, oam_i_2, 5);

	oam_d_3 = fgauge_read_d_by_v(oam_v_ocv_1);
	if (oam_d_3 < 0)
		oam_d_3 = 0;
	if (oam_d_3 > 100)
		oam_d_3 = 100;

	oam_r_1 = fgauge_read_r_bat_by_v(oam_v_ocv_1);

	oam_v_ocv_2 = fgauge_read_v_by_d(oam_d_2);
	oam_r_2 = fgauge_read_r_bat_by_v(oam_v_ocv_2);

#if 0
	oam_d_4 = (oam_d_2 + oam_d_3) / 2;
#else
	oam_d_4 = oam_d_3;
#endif

	gFG_columb = oam_car_2 / 10;	/* mAh */

	if ((oam_i_1 < 0) || (oam_i_2 < 0))
		gFG_Is_Charging = KAL_TRUE;
	else
		gFG_Is_Charging = KAL_FALSE;

#if 0
	if (gFG_Is_Charging == KAL_FALSE) {
		d5_count_time = 60;
	} else {
		charging_current = get_charging_setting_current();
		charging_current = charging_current / 100;
		d5_count_time_rate =
		    (((gFG_BATT_CAPACITY_aging * 60 * 60 / 100 / (charging_current - 50)) * 10) +
		     5) / 10;

		if (d5_count_time_rate < 1)
			d5_count_time_rate = 1;

		d5_count_time = d5_count_time_rate;
	}
#else
	d5_count_time = 60;
#endif
	d5_count = d5_count + delta_time;
	if (d5_count >= d5_count_time) {
		if (gFG_Is_Charging == KAL_FALSE) {
			if (oam_d_3 > oam_d_5) {
              if(!bat_spm_timeout)
                oam_d_5 = oam_d_5 + 1;
              else {
                if(oam_d_3 - oam_d_5 <= 4)
                    oam_d_5 = oam_d_5 + 1;
                else
                    oam_d_5 = oam_d_3;
              }
			} else {
				if (oam_d_4 > oam_d_5) {
				  if(!bat_spm_timeout)
				    oam_d_5 = oam_d_5 + 1;
				  else {
				    if(oam_d_4 - oam_d_5 <= 4)
				      oam_d_5 = oam_d_5 + 1;
				    else
				      oam_d_5 = oam_d_4;
				  }
				}
			}
		} else {
			if (oam_d_5 > oam_d_3)
				oam_d_5 = oam_d_5 - 1;
			else
				if (oam_d_4 < oam_d_5)
					oam_d_5 = oam_d_5 - 1;


		}
		d5_count = 0;
		if(bat_spm_timeout) { if(oam_d_3 - oam_d_5 <= 3) d5_count = 50; }
		oam_d_3_pre = oam_d_3;
		oam_d_4_pre = oam_d_4;
	}

	if (oam_run_i == 0) {
		oam_run_i = 1;
	}

}

/* ============================================================ // */



void table_init(void)
{
	BATTERY_PROFILE_STRUCT_P profile_p;
	R_PROFILE_STRUCT_P profile_p_r_table;

	int temperature = force_get_tbat(KAL_FALSE);

	/* Re-constructure r-table profile according to current temperature */
	profile_p_r_table = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t);
	fgauge_construct_r_table_profile(temperature, profile_p_r_table);

	/* Re-constructure battery profile according to current temperature */
	profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t);

	fgauge_construct_battery_profile(temperature, profile_p);
}

signed int auxadc_algo_run(void)
{
	signed int val = 0;

	gFG_voltage = battery_meter_get_battery_voltage(KAL_FALSE);
	val = fgauge_read_capacity_by_v(gFG_voltage);

	return val;
}

#if defined(SOC_BY_HW_FG)
void update_fg_dbg_tool_value(void)
{
	g_fg_dbg_bat_volt = gFG_voltage_init;

	if (gFG_Is_Charging == KAL_TRUE)
		g_fg_dbg_bat_current = 1 - gFG_current - 1;
	else
		g_fg_dbg_bat_current = gFG_current;

	g_fg_dbg_bat_zcv = gFG_voltage;

	g_fg_dbg_bat_temp = gFG_temp;

	g_fg_dbg_bat_r = gFG_resistance_bat;

	g_fg_dbg_bat_car = gFG_columb;

	g_fg_dbg_bat_qmax = gFG_BATT_CAPACITY_aging;

	g_fg_dbg_d0 = gFG_DOD0;

	g_fg_dbg_d1 = gFG_DOD1;

	g_fg_dbg_percentage = bat_get_ui_percentage();

	g_fg_dbg_percentage_fg = gFG_capacity_by_c;

	g_fg_dbg_percentage_voltmode = gfg_percent_check_point;
}

signed int fgauge_compensate_battery_voltage(signed int ori_voltage)
{
	signed int ret_compensate_value = 0;

	gFG_ori_voltage = ori_voltage;
	gFG_resistance_bat = fgauge_read_r_bat_by_v(ori_voltage);	/* Ohm */
	ret_compensate_value =
	    (gFG_current * (gFG_resistance_bat + batt_meter_cust_data.r_fg_value)) / 1000;
	ret_compensate_value = (ret_compensate_value + (10 / 2)) / 10;

	if (gFG_Is_Charging == KAL_TRUE)
		ret_compensate_value = ret_compensate_value - (ret_compensate_value * 2);


	gFG_compensate_value = ret_compensate_value;

	return ret_compensate_value;
}

signed int fgauge_compensate_battery_voltage_recursion(signed int ori_voltage,
						      signed int recursion_time)
{
	signed int ret_compensate_value = 0;
	signed int temp_voltage_1 = ori_voltage;
	signed int temp_voltage_2 = temp_voltage_1;
	int i = 0;

	for (i = 0; i < recursion_time; i++) {
		gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2);	/* Ohm */
		ret_compensate_value =
		    (gFG_current * (gFG_resistance_bat + batt_meter_cust_data.r_fg_value)) / 1000;
		ret_compensate_value = (ret_compensate_value + (10 / 2)) / 10;

		if (gFG_Is_Charging == KAL_TRUE)
			ret_compensate_value = ret_compensate_value - (ret_compensate_value * 2);

		temp_voltage_2 = temp_voltage_1 + ret_compensate_value;
	}

	gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2);	/* Ohm */
	ret_compensate_value =
	    (gFG_current *
	     (gFG_resistance_bat + batt_meter_cust_data.r_fg_value +
	      batt_meter_cust_data.fg_meter_resistance)) / 1000;
	ret_compensate_value = (ret_compensate_value + (10 / 2)) / 10;

	if (gFG_Is_Charging == KAL_TRUE)
		ret_compensate_value = ret_compensate_value - (ret_compensate_value * 2);


	gFG_compensate_value = ret_compensate_value;

	return ret_compensate_value;
}


signed int fgauge_get_dod0(signed int voltage, signed int temperature, kal_bool bOcv)
{
	signed int dod0 = 0;
	int i = 0, saddles = 0, jj = 0;
	BATTERY_PROFILE_STRUCT_P profile_p;
	R_PROFILE_STRUCT_P profile_p_r_table;
	int ret = 0;

/* R-Table (First Time) */
	/* Re-constructure r-table profile according to current temperature */
	profile_p_r_table = fgauge_get_profile_r_table(batt_meter_cust_data.temperature_t);
	fgauge_construct_r_table_profile(temperature, profile_p_r_table);

	/* Re-constructure battery profile according to current temperature */
	profile_p = fgauge_get_profile(batt_meter_cust_data.temperature_t);
	if (profile_p == NULL) {
		return 100;
	}
	fgauge_construct_battery_profile(temperature, profile_p);

	/* Get total saddle points from the battery profile */
	saddles = fgauge_get_saddles();

	/* If the input voltage is not OCV, compensate to ZCV due to battery loading */
	/* Compasate battery voltage from current battery voltage */
	jj = 0;
	if (bOcv == KAL_FALSE) {
		while (gFG_current == 0) {
			ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
			if (jj > 10)
				break;
			jj++;
		}
		/* voltage = voltage + fgauge_compensate_battery_voltage(voltage); //mV */
		voltage = voltage + fgauge_compensate_battery_voltage_recursion(voltage, 5);	/* mV */
	}
	/* If battery voltage is less then mimimum profile voltage, then return 100 */
	/* If battery voltage is greater then maximum profile voltage, then return 0 */
	if (voltage > (profile_p + 0)->voltage)
		return 0;

	if (voltage < (profile_p + saddles - 1)->voltage)
		return 100;

	/* get DOD0 according to current temperature */
	for (i = 0; i < saddles - 1; i++) {
		if ((voltage <= (profile_p + i)->voltage)
		    && (voltage >= (profile_p + i + 1)->voltage)) {
			dod0 =
			    (profile_p + i)->percentage +
			    (((((profile_p + i)->voltage) -
			       voltage) * (((profile_p + i + 1)->percentage) -
					   ((profile_p + i)->percentage))
			     ) / (((profile_p + i)->voltage) - ((profile_p + i + 1)->voltage))
			    );

			break;
		}
	}

	return dod0;
}


signed int fgauge_update_dod(void)
{
	signed int FG_dod_1 = 0;
	int adjust_coulomb_counter = batt_meter_cust_data.car_tune_value;
#ifdef Q_MAX_BY_CURRENT
	signed int C_0mA = 0;
	signed int C_400mA = 0;
	signed int C_FGCurrent = 0;
#endif

	if (gFG_DOD0 > 100) {
		gFG_DOD0 = 100;
	} else if (gFG_DOD0 < 0) {
		gFG_DOD0 = 0;
	} else {
	}

	gFG_temp = force_get_tbat(KAL_FALSE);

#if !defined(CONFIG_POWER_EXT)
	if (temperature_change == 1) {
		gFG_BATT_CAPACITY = fgauge_get_Q_max(gFG_temp);
		temperature_change = 0;
	}
#endif
#if 0
	C_0mA = fgauge_get_Q_max(gFG_temp);
	C_400mA = fgauge_get_Q_max_high_current(gFG_temp);
	C_FGCurrent = C_0mA - (C_0mA - C_400mA) * gFG_current_AVG / 4000;
	if (C_FGCurrent != 0)
		FG_dod_1 = gFG_DOD0 - ((gFG_columb * 100) / gFG_BATT_CAPACITY_aging) * C_0mA / C_FGCurrent;
#else
	FG_dod_1 = gFG_DOD0 - ((gFG_columb * 100) / gFG_BATT_CAPACITY_aging);

#endif
	if (FG_dod_1 > 100) {
		FG_dod_1 = 100;
	} else if (FG_dod_1 < 0) {
		FG_dod_1 = 0;
	} else {
	}

	return FG_dod_1;
}


signed int fgauge_read_capacity(signed int type)
{
	signed int voltage;
	signed int temperature;
	signed int dvalue = 0;
	signed int temp_val = 0;

	if (type == 0) {	/* for initialization */
		/* Use voltage to calculate capacity */
		voltage = battery_meter_get_battery_voltage(KAL_TRUE);	/* in unit of mV */
		temperature = force_get_tbat(KAL_FALSE);
		dvalue = fgauge_get_dod0(voltage, temperature, KAL_FALSE);	/* need compensate vbat */
	} else {
		/* Use DOD0 and columb counter to calculate capacity */
		dvalue = fgauge_update_dod();	/* DOD1 = DOD0 + (-CAR)/Qmax */
	}

	gFG_DOD1 = dvalue;

	temp_val = dvalue;
	dvalue = 100 - temp_val;

	if (dvalue <= 1) {
		dvalue = 1;
	}

	return dvalue;
}


void fg_voltage_mode(void)
{
#if defined(CONFIG_POWER_EXT)
#else
	if (bat_is_charger_exist() == KAL_TRUE) {
		/* SOC only UP when charging */
		if (gFG_capacity_by_v > gfg_percent_check_point)
			gfg_percent_check_point++;

	} else {
		/* SOC only Done when dis-charging */
		if (gFG_capacity_by_v < gfg_percent_check_point)
			gfg_percent_check_point--;

	}

#endif
}


void fgauge_algo_run(void)
{
	int i = 0;
	int ret = 0;
#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
	int columb_delta = 0;
	int charge_current = 0;
#endif

	/* Reconstruct table if temp changed; */
	fgauge_construct_table_by_temp();

/* 1. Get Raw Data */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &gFG_Is_Charging);

	gFG_voltage = battery_meter_get_battery_voltage(KAL_FALSE);
	gFG_voltage_init = gFG_voltage;
	gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage, 5);	/* mV */
	gFG_voltage = gFG_voltage + batt_meter_cust_data.ocv_board_compesate;

#if defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
	fgauge_get_current_factor();
#endif

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);

#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
	if (gFG_Is_Charging) {
		charge_current -= gFG_current;
		if (charge_current < gFG_min_current)
			gFG_min_current = charge_current;
	} else {
		if (gFG_current > gFG_max_current)
			gFG_max_current = gFG_current;
	}

	columb_delta = gFG_pre_columb_count - gFG_columb;

	if (columb_delta < 0)
		columb_delta = columb_delta - 2 * columb_delta;	/* absolute value */

	gFG_pre_columb_count = gFG_columb;
	gFG_columb_sum += columb_delta;

	/* should we use gFG_BATT_CAPACITY or gFG_BATT_CAPACITY_aging ?? */
	if (gFG_columb_sum >= 2 * gFG_BATT_CAPACITY_aging) {
		gFG_battery_cycle++;
		gFG_columb_sum -= 2 * gFG_BATT_CAPACITY_aging;
	}
#endif

	/* add by willcai 2014-12-18 begin */
	if (BMT_status.charger_exist == KAL_FALSE) {
		if (gFG_Is_offset_init == KAL_FALSE) {
			for (i = 0; i < batt_meter_cust_data.fg_vbat_average_size; i++)
				FGvbatVoltageBuffer[i] = gFG_voltage;


			FGbatteryVoltageSum =
			    gFG_voltage * batt_meter_cust_data.fg_vbat_average_size;
			gFG_voltage_AVG = gFG_voltage;
			gFG_Is_offset_init = KAL_TRUE;
		}
/* 1.1 Average FG_voltage */
    /**************** Averaging : START ****************/
		if (gFG_voltage >= gFG_voltage_AVG)
			gFG_vbat_offset = (gFG_voltage - gFG_voltage_AVG);
		else
			gFG_vbat_offset = (gFG_voltage_AVG - gFG_voltage);


		if (gFG_vbat_offset <= batt_meter_cust_data.minerroroffset) {
			FGbatteryVoltageSum -= FGvbatVoltageBuffer[FGbatteryIndex];
			FGbatteryVoltageSum += gFG_voltage;
			FGvbatVoltageBuffer[FGbatteryIndex] = gFG_voltage;

			gFG_voltage_AVG =
			    FGbatteryVoltageSum / batt_meter_cust_data.fg_vbat_average_size;
			gFG_voltage = gFG_voltage_AVG;

			FGbatteryIndex++;
			if (FGbatteryIndex >= batt_meter_cust_data.fg_vbat_average_size)
				FGbatteryIndex = 0;
		} else {
			gFG_voltage = gFG_voltage_AVG;
		}
	} else
		gFG_Is_offset_init = KAL_FALSE;

#ifdef Q_MAX_BY_CURRENT
/* 1.2 Average FG_current */
    /**************** Averaging : START ****************/
	if (gFG_current_AVG == 0) {
		for (i = 0; i < FG_CURRENT_AVERAGE_SIZE; i++)
			FGCurrentBuffer[i] = gFG_current;


		FGCurrentSum = gFG_current * FG_CURRENT_AVERAGE_SIZE;
		gFG_current_AVG = gFG_current;
	} else {
		FGCurrentSum -= FGCurrentBuffer[FGCurrentIndex];
		FGCurrentSum += gFG_current;
		FGCurrentBuffer[FGCurrentIndex] = gFG_current;

		gFG_current_AVG = FGCurrentSum / FG_CURRENT_AVERAGE_SIZE;

		FGCurrentIndex++;
		if (FGCurrentIndex >= FG_CURRENT_AVERAGE_SIZE)
			FGCurrentIndex = 0;
	}
#endif
/* 2. Calculate battery capacity by VBAT */
	gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);

/* 3. Calculate battery capacity by Coulomb Counter */
	gFG_capacity_by_c = fgauge_read_capacity(1);

/* 4. voltage mode */
	if (volt_mode_update_timer >= volt_mode_update_time_out) {
		volt_mode_update_timer = 0;

		fg_voltage_mode();
	} else {
		volt_mode_update_timer++;
	}

	update_fg_dbg_tool_value();
}

void fgauge_algo_run_init(void)
{
	int i = 0;
	int ret = 0;

#ifdef INIT_SOC_BY_SW_SOC
	kal_bool charging_enable = KAL_FALSE;
#if defined(CONFIG_MTK_KERNEL_POWER_OFF_CHARGING) && !defined(SWCHR_POWER_PATH)
	if (LOW_POWER_OFF_CHARGING_BOOT != get_boot_mode())
#endif
		/*stop charging for vbat measurement */
		battery_charging_control(CHARGING_CMD_ENABLE, &charging_enable);

	msleep(50);
#endif
/* 1. Get Raw Data */
	gFG_voltage = battery_meter_get_battery_voltage(KAL_TRUE);
	gFG_voltage_init = gFG_voltage;
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &gFG_Is_Charging);

	gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage, 5);	/* mV */
	gFG_voltage = gFG_voltage + batt_meter_cust_data.ocv_board_compesate;

#ifdef INIT_SOC_BY_SW_SOC
	charging_enable = KAL_TRUE;
	battery_charging_control(CHARGING_CMD_ENABLE, &charging_enable);
#endif
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);

/* 1.1 Average FG_voltage */
	for (i = 0; i < batt_meter_cust_data.fg_vbat_average_size; i++)
		FGvbatVoltageBuffer[i] = gFG_voltage;


	FGbatteryVoltageSum = gFG_voltage * batt_meter_cust_data.fg_vbat_average_size;
	gFG_voltage_AVG = gFG_voltage;

#ifdef Q_MAX_BY_CURRENT
/* 1.2 Average FG_current */
	for (i = 0; i < FG_CURRENT_AVERAGE_SIZE; i++)
		FGCurrentBuffer[i] = gFG_current;


	FGCurrentSum = gFG_current * FG_CURRENT_AVERAGE_SIZE;
	gFG_current_AVG = gFG_current;
#endif

/* 2. Calculate battery capacity by VBAT */
	gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);
	gFG_capacity_by_v_init = gFG_capacity_by_v;

/* 3. Calculate battery capacity by Coulomb Counter */
	gFG_capacity_by_c = fgauge_read_capacity(1);

/* 4. update DOD0 */

	dod_init();

	gFG_current_auto_detect_R_fg_count = 0;

	for (i = 0; i < 10; i++) {
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);

		gFG_current_auto_detect_R_fg_total += gFG_current;
		gFG_current_auto_detect_R_fg_count++;
	}

	/* double check */
	if (gFG_current_auto_detect_R_fg_total <= 0) {
		gFG_current_auto_detect_R_fg_count = 0;

		for (i = 0; i < 10; i++) {
			ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
			gFG_current_auto_detect_R_fg_total += gFG_current;
			gFG_current_auto_detect_R_fg_count++;
		}
	}

	gFG_current_auto_detect_R_fg_result =
	    gFG_current_auto_detect_R_fg_total / gFG_current_auto_detect_R_fg_count;
#if !defined(DISABLE_RFG_EXIST_CHECK)
	if (gFG_current_auto_detect_R_fg_result <= batt_meter_cust_data.current_detect_r_fg) {
		g_auxadc_solution = 1;
	} else {
		if (g_auxadc_solution == 0) {
			g_auxadc_solution = 0;
		}
	}
#endif
	update_fg_dbg_tool_value();
}


#ifdef FG_BAT_INT
unsigned char reset_fg_bat_int = KAL_TRUE;
void fg_bat_int_handler(void)
{
	reset_fg_bat_int = KAL_TRUE;
	wake_up_bat2();
}
#endif

void fgauge_initialization(void)
{
#if defined(CONFIG_POWER_EXT)
#else
	int i = 0;
	unsigned int ret = 0;

	/* gFG_BATT_CAPACITY_init_high_current = fgauge_get_Q_max_high_current(25); */
	/* gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(25); */

	/* 1. HW initialization */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_HW_FG_INIT, NULL);

	/* 2. SW algorithm initialization */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
	i = 0;
	while (gFG_current == 0) {
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
		if (i > 10) {
			break;
		}
		i++;
	}

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);
#if !defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
	fgauge_construct_battery_profile_init();
#endif
	gFG_temp = force_get_tbat(KAL_FALSE);
	gFG_capacity = fgauge_read_capacity(0);

	gFG_capacity_by_c_init = gFG_capacity;
	gFG_capacity_by_c = gFG_capacity;
	gFG_capacity_by_v = gFG_capacity;

	gFG_DOD0 = 100 - gFG_capacity;
	gFG_BATT_CAPACITY = fgauge_get_Q_max(gFG_temp);

	gFG_BATT_CAPACITY_init_high_current = fgauge_get_Q_max_high_current(gFG_temp);
	gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(gFG_temp);

	ret = battery_meter_ctrl(BATTERY_METER_CMD_DUMP_REGISTER, NULL);

#if defined(FG_BAT_INT)
	/*pmic_register_interrupt_callback(41, fg_bat_int_handler);*/
	/*pmic_register_interrupt_callback(40, fg_bat_int_handler);*/
#endif
#endif
}
#endif

signed int get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level)
{
#if defined(CONFIG_POWER_EXT)

	return first_wakeup_time;

#elif defined(SOC_BY_AUXADC) ||  defined(SOC_BY_SW_FG)

	signed int vbat_val = 0;

#ifdef CONFIG_MTK_POWER_EXT_DETECT
	if (KAL_TRUE == bat_is_ext_power())
		return batt_meter_cust_data.normal_wakeup_period;
#endif

	vbat_val = g_sw_vbat_temp;

	/* change wake up period when system suspend. */
	if (vbat_val > batt_meter_cust_data.vbat_normal_wakeup)	/* 3.6v */
		g_spm_timer = batt_meter_cust_data.normal_wakeup_period;	/* 90 min */
	else if (vbat_val > batt_meter_cust_data.vbat_low_power_wakeup)	/* 3.5v */
		g_spm_timer = batt_meter_cust_data.low_power_wakeup_period;	/* 5 min */
	else
		g_spm_timer = batt_meter_cust_data.close_poweroff_wakeup_period;	/* 0.5 min */

	return g_spm_timer;
#else

	signed int car_instant = 0;
	signed int current_instant = 0;
	static signed int last_time;
	signed int vbat_val = 0;
	int ret = 0;

#if defined(FG_BAT_INT)
#else
	signed int I_sleep = 0;
	signed int new_time = 0;
	signed int ret_val = -1;
	signed int car_wakeup = 0;
	static signed int car_sleep = 0x12345678;

#endif

	vbat_val = g_sw_vbat_temp;

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &current_instant);
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &car_instant);

	if (car_instant < 0)
		car_instant = car_instant - (car_instant * 2);


	if (BMT_status.UI_SOC != BMT_status.SOC) {
		last_time = 10;
		g_spm_timer = 10;
		return g_spm_timer;
	}


	if (vbat_val > batt_meter_cust_data.vbat_normal_wakeup) {	/* 3.6v */

#if defined(FG_BAT_INT)
				g_spm_timer = LOW_POWER_WAKEUP_PERIOD * 3;
#else

		car_wakeup = car_instant;

		if (last_time == 0)
			last_time = 1;

		if (car_sleep > car_wakeup || car_sleep == 0x12345678) {
			car_sleep = car_wakeup;
		}

		I_sleep = ((car_wakeup - car_sleep) * 3600) / last_time;	/* unit: second */

		if (I_sleep == 0) {
			ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &I_sleep);
			I_sleep = I_sleep / 10;
		}

		if (I_sleep == 0) {
			new_time = first_wakeup_time;
		} else {
			new_time =
			    ((gFG_BATT_CAPACITY * battery_capacity_level * 3600) / 100) / I_sleep;
		}
		ret_val = new_time;

		if (ret_val == 0)
			ret_val = first_wakeup_time;

		/* update parameter */
		car_sleep = car_wakeup;
		last_time = ret_val;
		g_spm_timer = ret_val;
#endif
	} else if (vbat_val > batt_meter_cust_data.vbat_low_power_wakeup) {	/* 3.5v */
		g_spm_timer = batt_meter_cust_data.low_power_wakeup_period;	/* 5 min */
	} else {
		g_spm_timer = batt_meter_cust_data.close_poweroff_wakeup_period;	/* 0.5 min */
	}

	return g_spm_timer;

#endif
}

/* ============================================================ // */
signed int battery_meter_get_battery_voltage(kal_bool update)
{
	int ret = 0;
	int val = 5;
	static int pre_val = -1;

	if (update == KAL_TRUE || pre_val == -1) {
		val = 5;	/* set avg times */
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &val);
		pre_val = val;
	} else {
		val = pre_val;
	}
	g_sw_vbat_temp = val;

#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
	if (g_sw_vbat_temp > gFG_max_voltage)
		gFG_max_voltage = g_sw_vbat_temp;


	if (g_sw_vbat_temp < gFG_min_voltage)
		gFG_min_voltage = g_sw_vbat_temp;

#endif

	return val;
}

signed int battery_meter_get_charging_current_imm(void)
{
#ifdef AUXADC_SUPPORT_IMM_CURRENT_MODE
	return PMIC_IMM_GetCurrent();
#else
	int ret;
	signed int ADC_I_SENSE = 1;	/* 1 measure time */
	signed int ADC_BAT_SENSE = 1;	/* 1 measure time */
	int ICharging = 0;

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &ADC_BAT_SENSE);
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_I_SENSE, &ADC_I_SENSE);

	ICharging =
	    (ADC_I_SENSE - ADC_BAT_SENSE +
	     g_I_SENSE_offset) * 1000 / batt_meter_cust_data.cust_r_sense;
	return ICharging;
#endif
}

signed int battery_meter_get_charging_current(void)
{
#ifdef DISABLE_CHARGING_CURRENT_MEASURE
	return 0;
#elif !defined(EXTERNAL_SWCHR_SUPPORT)
	signed int ADC_BAT_SENSE_tmp[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	signed int ADC_BAT_SENSE_sum = 0;
	signed int ADC_BAT_SENSE = 0;
	signed int ADC_I_SENSE_tmp[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	signed int ADC_I_SENSE_sum = 0;
	signed int ADC_I_SENSE = 0;
	int repeat = 20;
	int i = 0;
	int j = 0;
	signed int temp = 0;
	int ICharging = 0;
	int ret = 0;
	int val = 1;

	for (i = 0; i < repeat; i++) {
		val = 1;	/* set avg times */
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &val);
		ADC_BAT_SENSE_tmp[i] = val;

		val = 1;	/* set avg times */
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_I_SENSE, &val);
		ADC_I_SENSE_tmp[i] = val;

		ADC_BAT_SENSE_sum += ADC_BAT_SENSE_tmp[i];
		ADC_I_SENSE_sum += ADC_I_SENSE_tmp[i];
	}

	/* sorting    BAT_SENSE */
	for (i = 0; i < repeat; i++) {
		for (j = i; j < repeat; j++) {
			if (ADC_BAT_SENSE_tmp[j] < ADC_BAT_SENSE_tmp[i]) {
				temp = ADC_BAT_SENSE_tmp[j];
				ADC_BAT_SENSE_tmp[j] = ADC_BAT_SENSE_tmp[i];
				ADC_BAT_SENSE_tmp[i] = temp;
			}
		}
	}

	/* sorting    I_SENSE */
	for (i = 0; i < repeat; i++) {
		for (j = i; j < repeat; j++) {
			if (ADC_I_SENSE_tmp[j] < ADC_I_SENSE_tmp[i]) {
				temp = ADC_I_SENSE_tmp[j];
				ADC_I_SENSE_tmp[j] = ADC_I_SENSE_tmp[i];
				ADC_I_SENSE_tmp[i] = temp;
			}
		}
	}

	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[0];
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[1];
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[18];
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[19];
	ADC_BAT_SENSE = ADC_BAT_SENSE_sum / (repeat - 4);

	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[0];
	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[1];
	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[18];
	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[19];
	ADC_I_SENSE = ADC_I_SENSE_sum / (repeat - 4);

	if (ADC_I_SENSE > ADC_BAT_SENSE) {
		ICharging =
		    (ADC_I_SENSE - ADC_BAT_SENSE +
		     g_I_SENSE_offset) * 1000 / batt_meter_cust_data.cust_r_sense;
	} else {
		ICharging = 0;
	}

	return ICharging;
#else
	return 0;
#endif
}

signed int battery_meter_get_battery_current(void)
{
	int ret = 0;
	signed int val = 0;

	if (g_auxadc_solution == 1)
		val = oam_i_2;
	else
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &val);

	return val;
}

kal_bool battery_meter_get_battery_current_sign(void)
{
	int ret = 0;
	kal_bool val = 0;

	if (g_auxadc_solution == 1)
		val = 0;	/* discharging */
	else
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &val);

	return val;
}

signed int battery_meter_get_car(void)
{
	int ret = 0;
	signed int val = 0;

	if (g_auxadc_solution == 1)
		val = oam_car_2;
	else
		ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &val);

	return val;
}

signed int battery_meter_get_battery_temperature(void)
{
#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
	signed int batt_temp = force_get_tbat(KAL_TRUE);

	if (batt_temp > gFG_max_temperature)
		gFG_max_temperature = batt_temp;
	if (batt_temp < gFG_min_temperature)
		gFG_min_temperature = batt_temp;

	return batt_temp;
#else
	return force_get_tbat(KAL_TRUE);
#endif
}

signed int battery_meter_get_charger_voltage(void)
{
	int ret = 0;
	int val = 0;

	val = 5;		/* set avg times */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_CHARGER, &val);

	/* val = (((R_CHARGER_1+R_CHARGER_2)*100*val)/R_CHARGER_2)/100; */
	return val;
}

#if defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
void battery_meter_set_reset_soc(kal_bool bUSE_UI_SOC)
{
	g_USE_UI_SOC = bUSE_UI_SOC;
}

signed int battery_meter_get_battery_soc(void)
{
#if defined(SOC_BY_HW_FG)
	return gFG_capacity_by_c;
#else
	return 50;
#endif

#if defined(SOC_BY_SW_FG)
#if (OAM_D5 == 1)
	return 100 - oam_d_5;
#else
	return 100 - oam_d_2;
#endif
#else
	return 50;
#endif
}

/* Here we compensate D1 by a factor from Qmax with loading. */
signed int battery_meter_trans_battery_percentage(kal_bool d_val)
{
	signed int d_val_before = 0;
	signed int temp_val = 0;
	signed int C_0mA = 0;
	signed int C_600mA = 0;
	signed int C_current = 0;
	signed int i_avg_current = 0;

	d_val_before = d_val;
	temp_val = battery_meter_get_battery_temperature();
	C_0mA = fgauge_get_Q_max(temp_val);

	/* discharging and current > 600ma */
	i_avg_current = g_currentfactor * CV_CURRENT / 100;
	if (KAL_FALSE == gFG_Is_Charging && g_currentfactor > 100) {
		C_600mA = fgauge_get_Q_max_high_current(temp_val);
		C_current = fgauge_get_Q_max_high_current_by_current(i_avg_current, temp_val);
		if (C_current < C_600mA)
			C_600mA = C_current;
	} else
		C_600mA = fgauge_get_Q_max_high_current(temp_val);

	if (C_0mA > C_600mA)
		d_val = d_val + (((C_0mA - C_600mA) * (d_val)) / C_600mA);

	if (d_val > 100)
		d_val = 100;

	return d_val;
}
#endif

#if defined(FG_BAT_INT)
signed int battery_meter_set_columb_interrupt(unsigned int val)
{
	battery_log(BAT_LOG_FULL, "battery_meter_set_columb_interrupt=%d\n", val);
	battery_meter_ctrl(BATTERY_METER_CMD_SET_COLUMB_INTERRUPT, &val);
	return 0;
}
#endif				/* #if defined(FG_BAT_INT) */

signed int battery_meter_get_battery_percentage(void)
{
#if defined(CONFIG_POWER_EXT)
	return 50;
#else

	if (bat_is_charger_exist() == KAL_FALSE)
		fg_qmax_update_for_aging_flag = 1;

#if defined(SOC_BY_AUXADC)
	return auxadc_algo_run();
#endif

#if defined(SOC_BY_HW_FG)
	if (g_auxadc_solution == 1)
		return auxadc_algo_run();
/*else {*/
		fgauge_algo_run();
#if !defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
		return gFG_capacity_by_c;	/* hw fg, //return gfg_percent_check_point; // voltage mode */
#else
		/* We keep gFG_capacity_by_c as capacity before compensation */
		/* Compensated capacity is returned for UI SOC tracking */
		return 100 - battery_meter_trans_battery_percentage(100 - gFG_capacity_by_c);
#endif
	/*}*/
#endif

#if defined(SOC_BY_SW_FG)
	oam_run();
#if !defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
#if (OAM_D5 == 1)
	return 100 - oam_d_5;
#else
	return 100 - oam_d_2;
#endif
#else
#if (OAM_D5 == 1)
	return 100 - battery_meter_trans_battery_percentage(oam_d_5);
#else
	return 100 - battery_meter_trans_battery_percentage(oam_d_2);
#endif
#endif
#endif

#endif
}


signed int battery_meter_initial(void)
{
#if defined(CONFIG_POWER_EXT)
	return 0;
#else
	static kal_bool meter_initilized = KAL_FALSE;

	mutex_lock(&FGADC_mutex);
	if (meter_initilized == KAL_FALSE) {
#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
		fgauge_get_profile_id();
#endif

#if defined(SOC_BY_AUXADC)
		g_auxadc_solution = 1;
		table_init();
#endif

#if defined(SOC_BY_HW_FG)
		fgauge_initialization();
		fgauge_algo_run_init();
#endif

#if defined(SOC_BY_SW_FG)
		g_auxadc_solution = 1;
		table_init();
		oam_init();
#endif

		meter_initilized = KAL_TRUE;
	}
	mutex_unlock(&FGADC_mutex);
	return 0;
#endif
}

void reset_parameter_car(void)
{
#if defined(SOC_BY_HW_FG)
	int ret = 0
;
	ret = battery_meter_ctrl(BATTERY_METER_CMD_HW_RESET, NULL);
	gFG_columb = 0;

#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
	gFG_pre_columb_count = 0;
#endif

#ifdef MTK_ENABLE_AGING_ALGORITHM
	aging_ocv_1 = 0;
	aging_ocv_2 = 0;
#ifdef MD_SLEEP_CURRENT_CHECK
	columb_before_sleep = 0x123456;
#endif
#endif

#endif

#if defined(SOC_BY_SW_FG)
	oam_car_1 = 0;
	oam_car_2 = 0;
	gFG_columb = 0;
#endif
}

void reset_parameter_dod_change(void)
{
#if defined(SOC_BY_HW_FG)
	gFG_DOD0 = gFG_DOD1;
#endif

#if defined(SOC_BY_SW_FG)
	oam_d0 = oam_d_5;
	gFG_DOD0 = oam_d0;
	oam_d_1 = oam_d_5;
	oam_d_2 = oam_d_5;
	oam_d_3 = oam_d_5;
	oam_d_4 = oam_d_5;
#endif
}

void reset_parameter_dod_full(unsigned int ui_percentage)
{
#if defined(SOC_BY_HW_FG)
	gFG_DOD0 = 100 - ui_percentage;
	gFG_DOD1 = gFG_DOD0;
#endif

#if defined(SOC_BY_SW_FG)
	oam_d0 = 100 - ui_percentage;
	gFG_DOD0 = oam_d0;
	gFG_DOD1 = oam_d0;
	oam_d_1 = oam_d0;
	oam_d_2 = oam_d0;
	oam_d_3 = oam_d0;
	oam_d_4 = oam_d0;
	oam_d_5 = oam_d0;
#endif
}

signed int battery_meter_reset(void)
{
#if defined(CONFIG_POWER_EXT)
	return 0;
#else
	unsigned int ui_percentage = bat_get_ui_percentage();

#if defined(CUST_CAPACITY_OCV2CV_TRANSFORM)
	if (KAL_FALSE == g_USE_UI_SOC) {
		ui_percentage = battery_meter_get_battery_soc();
		g_USE_UI_SOC = KAL_TRUE;
	}
#endif

	reset_parameter_car();
	reset_parameter_dod_full(ui_percentage);

	return 0;
#endif
}

signed int battery_meter_sync(signed int bat_i_sense_offset)
{
#if defined(CONFIG_POWER_EXT)
	return 0;
#else
	g_I_SENSE_offset = bat_i_sense_offset;
	return 0;
#endif
}

signed int battery_meter_get_battery_zcv(void)
{
#if defined(CONFIG_POWER_EXT)
	return 3987;
#else
	return gFG_voltage;
#endif
}

signed int battery_meter_get_battery_nPercent_zcv(void)
{
#if defined(CONFIG_POWER_EXT)
	return 3700;
#else
	return gFG_15_vlot;	/* 15% zcv,  15% can be customized by 100-g_tracking_point */
#endif
}

signed int battery_meter_get_battery_nPercent_UI_SOC(void)
{
#if defined(CONFIG_POWER_EXT)
	return 15;
#else
	return g_tracking_point;	/* tracking point */
#endif
}

signed int battery_meter_get_tempR(signed int dwVolt)
{
#if defined(CONFIG_POWER_EXT)
	return 0;
#else
	int TRes;

	TRes =
	    (batt_meter_cust_data.rbat_pull_up_r * dwVolt) /
	    (batt_meter_cust_data.rbat_pull_up_volt - dwVolt);

	return TRes;
#endif
}

signed int battery_meter_get_tempV(void)
{
#if defined(CONFIG_POWER_EXT)
	return 0;
#else
	int ret = 0;
	int val = 0;

	val = 1;		/* set avg times */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_TEMP, &val);
	return val;
#endif
}

signed int battery_meter_get_VSense(void)
{
#if defined(CONFIG_POWER_EXT)
	return 0;
#else
	int ret = 0;
	int val = 0;

	val = 1;		/* set avg times */
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_I_SENSE, &val);
	return val;
#endif
}

/* ============================================================ // */
static ssize_t fgadc_log_write(struct file *filp, const char __user *buff,
			       size_t len, loff_t *data)
{
	if (copy_from_user(&proc_fgadc_data, buff, len)) {
		return -EFAULT;
	}

	Enable_FGADC_LOG = 0;

	return len;
}

static const struct file_operations fgadc_proc_fops = {
	.write = fgadc_log_write,
};

int init_proc_log_fg(void)
{
	int ret = 0;

#if 1
	proc_create("fgadc_log", 0644, NULL, &fgadc_proc_fops);
#else
	proc_entry_fgadc = create_proc_entry("fgadc_log", 0644, NULL);

	if (proc_entry_fgadc == NULL) {
		ret = -ENOMEM;
	} else {
		proc_entry_fgadc->write_proc = fgadc_log_write;
	}
#endif

	return ret;
}

#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT

/* ============================================================ // */

#ifdef CUSTOM_BATTERY_CYCLE_AGING_DATA

signed int get_battery_aging_factor(signed int cycle)
{
	signed int i, f1, f2, c1, c2;
	signed int saddles;

	saddles = sizeof(battery_aging_table) / sizeof(BATTERY_CYCLE_STRUCT);

	for (i = 0; i < saddles; i++) {
		if (battery_aging_table[i].cycle == cycle)
			return battery_aging_table[i].aging_factor;


		if (battery_aging_table[i].cycle > cycle) {
			if (i == 0)
				return 100;


			if (battery_aging_table[i].aging_factor >
			    battery_aging_table[i - 1].aging_factor) {
				f1 = battery_aging_table[i].aging_factor;
				f2 = battery_aging_table[i - 1].aging_factor;
				c1 = battery_aging_table[i].cycle;
				c2 = battery_aging_table[i - 1].cycle;
				return f2 + ((cycle - c2) * (f1 - f2)) / (c1 - c2);
			} /*else {*/
				f1 = battery_aging_table[i - 1].aging_factor;
				f2 = battery_aging_table[i].aging_factor;
				c1 = battery_aging_table[i].cycle;
				c2 = battery_aging_table[i - 1].cycle;
				return f2 + ((cycle - c2) * (f1 - f2)) / (c1 - c2);
			/*}*/
		}
	}

	return battery_aging_table[saddles - 1].aging_factor;
}

#endif

static ssize_t show_FG_Battery_Cycle(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gFG_battery_cycle);
}

static ssize_t store_FG_Battery_Cycle(struct device *dev, struct device_attribute *attr,
				      const char *buf, size_t size)
{
	signed int cycle;

#ifdef CUSTOM_BATTERY_CYCLE_AGING_DATA
	signed int aging_capacity;
	signed int factor;
#endif

	if (1 == kstrtoint(buf, 0, &cycle)) {
		gFG_battery_cycle = cycle;

#ifdef CUSTOM_BATTERY_CYCLE_AGING_DATA
		/* perform cycle aging calculation */

		factor = get_battery_aging_factor(gFG_battery_cycle);
		if (factor > 0 && factor < 100) {
			aging_capacity = gFG_BATT_CAPACITY * factor / 100;
			if (aging_capacity < gFG_BATT_CAPACITY_aging) {
				gFG_BATT_CAPACITY_aging = aging_capacity;
			}
		}
#endif
	}
	return size;
}

static DEVICE_ATTR(FG_Battery_Cycle, 0664, show_FG_Battery_Cycle, store_FG_Battery_Cycle);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Max_Battery_Voltage(struct device *dev, struct device_attribute *attr,
					   char *buf)
{
	return sprintf(buf, "%d\n", gFG_max_voltage);
}

static ssize_t store_FG_Max_Battery_Voltage(struct device *dev, struct device_attribute *attr,
					    const char *buf, size_t size)
{
	signed int voltage;

	if (1 == kstrtoint(buf, 0, &voltage)) {
		if (voltage > gFG_max_voltage) {
			gFG_max_voltage = voltage;
		}
	}
	return size;
}

static DEVICE_ATTR(FG_Max_Battery_Voltage, 0664, show_FG_Max_Battery_Voltage,
		   store_FG_Max_Battery_Voltage);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Min_Battery_Voltage(struct device *dev, struct device_attribute *attr,
					   char *buf)
{
	return sprintf(buf, "%d\n", gFG_min_voltage);
}

static ssize_t store_FG_Min_Battery_Voltage(struct device *dev, struct device_attribute *attr,
					    const char *buf, size_t size)
{
	signed int voltage;

	if (1 == kstrtoint(buf, 0, &voltage)) {
		if (voltage < gFG_min_voltage) {
			gFG_min_voltage = voltage;
		}
	}
	return size;
}

static DEVICE_ATTR(FG_Min_Battery_Voltage, 0664, show_FG_Min_Battery_Voltage,
		   store_FG_Min_Battery_Voltage);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Max_Battery_Current(struct device *dev, struct device_attribute *attr,
					   char *buf)
{
	return sprintf(buf, "%d\n", gFG_max_current);
}

static ssize_t store_FG_Max_Battery_Current(struct device *dev, struct device_attribute *attr,
					    const char *buf, size_t size)
{
	signed int bat_current;

	if (1 == kstrtoint(buf, 0, &bat_current)) {
		if (bat_current > gFG_max_current) {
			gFG_max_current = bat_current;
		}
	}
	return size;
}

static DEVICE_ATTR(FG_Max_Battery_Current, 0664, show_FG_Max_Battery_Current,
		   store_FG_Max_Battery_Current);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Min_Battery_Current(struct device *dev, struct device_attribute *attr,
					   char *buf)
{
	return sprintf(buf, "%d\n", gFG_min_current);
}

static ssize_t store_FG_Min_Battery_Current(struct device *dev, struct device_attribute *attr,
					    const char *buf, size_t size)
{
	signed int bat_current;

	if (1 == kstrtoint(buf, 0, &bat_current)) {
		if (bat_current < gFG_min_current) {
			gFG_min_current = bat_current;
		}
	}
	return size;
}

static DEVICE_ATTR(FG_Min_Battery_Current, 0664, show_FG_Min_Battery_Current,
		   store_FG_Min_Battery_Current);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Max_Battery_Temperature(struct device *dev, struct device_attribute *attr,
					       char *buf)
{
	return sprintf(buf, "%d\n", gFG_max_temperature);
}

static ssize_t store_FG_Max_Battery_Temperature(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t size)
{
	signed int temp;

	if (1 == kstrtoint(buf, 0, &temp)) {
		if (temp > gFG_max_temperature) {
			gFG_max_temperature = temp;
		}
	}
	return size;
}

static DEVICE_ATTR(FG_Max_Battery_Temperature, 0664, show_FG_Max_Battery_Temperature,
		   store_FG_Max_Battery_Temperature);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Min_Battery_Temperature(struct device *dev, struct device_attribute *attr,
					       char *buf)
{
	return sprintf(buf, "%d\n", gFG_min_temperature);
}

static ssize_t store_FG_Min_Battery_Temperature(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t size)
{
	signed int temp;

	if (1 == kstrtoint(buf, 0, &temp)) {
		if (temp < gFG_min_temperature) {
			gFG_min_temperature = temp;
		}
	}
	return size;
}

static DEVICE_ATTR(FG_Min_Battery_Temperature, 0664, show_FG_Min_Battery_Temperature,
		   store_FG_Min_Battery_Temperature);

/* ------------------------------------------------------------------------------------------- */

static ssize_t show_FG_Aging_Factor(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gFG_aging_factor);
}

static ssize_t store_FG_Aging_Factor(struct device *dev, struct device_attribute *attr,
				     const char *buf, size_t size)
{
	signed int factor;
	signed int aging_capacity;

	if (1 == kstrtoint(buf, 0, &factor)) {
		if (factor <= 100 && factor >= 0) {
			gFG_aging_factor = factor;

			if (gFG_aging_factor != 100) {
				aging_capacity = gFG_BATT_CAPACITY * gFG_aging_factor / 100;
				if (aging_capacity < gFG_BATT_CAPACITY_aging) {
					gFG_BATT_CAPACITY_aging = aging_capacity;
				}
			}
		}
	}

	return size;
}

static DEVICE_ATTR(FG_Aging_Factor, 0664, show_FG_Aging_Factor, store_FG_Aging_Factor);

/* ------------------------------------------------------------------------------------------- */

#endif

/* ============================================================ // */
static ssize_t show_FG_Current(struct device *dev, struct device_attribute *attr, char *buf)
{
	signed int ret = 0;
	signed int fg_current_inout_battery = 0;
	signed int val = 0;
	kal_bool is_charging = 0;

	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &val);
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &is_charging);

	if (is_charging == KAL_TRUE)
		fg_current_inout_battery = 0 - val;
	else
		fg_current_inout_battery = val;

	return sprintf(buf, "%d\n", fg_current_inout_battery);
}

static ssize_t store_FG_Current(struct device *dev, struct device_attribute *attr, const char *buf,
				size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_Current, 0664, show_FG_Current, store_FG_Current);

/* ============================================================ // */
static ssize_t show_FG_g_fg_dbg_bat_volt(struct device *dev, struct device_attribute *attr,
					 char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_volt);
}

static ssize_t store_FG_g_fg_dbg_bat_volt(struct device *dev, struct device_attribute *attr,
					  const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_volt, 0664, show_FG_g_fg_dbg_bat_volt,
		   store_FG_g_fg_dbg_bat_volt);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_bat_current(struct device *dev, struct device_attribute *attr,
					    char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_current);
}

static ssize_t store_FG_g_fg_dbg_bat_current(struct device *dev, struct device_attribute *attr,
					     const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_current, 0664, show_FG_g_fg_dbg_bat_current,
		   store_FG_g_fg_dbg_bat_current);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_bat_zcv(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_zcv);
}

static ssize_t store_FG_g_fg_dbg_bat_zcv(struct device *dev, struct device_attribute *attr,
					 const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_zcv, 0664, show_FG_g_fg_dbg_bat_zcv, store_FG_g_fg_dbg_bat_zcv);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_bat_temp(struct device *dev, struct device_attribute *attr,
					 char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_temp);
}

static ssize_t store_FG_g_fg_dbg_bat_temp(struct device *dev, struct device_attribute *attr,
					  const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_temp, 0664, show_FG_g_fg_dbg_bat_temp,
		   store_FG_g_fg_dbg_bat_temp);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_bat_r(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_r);
}

static ssize_t store_FG_g_fg_dbg_bat_r(struct device *dev, struct device_attribute *attr,
				       const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_r, 0664, show_FG_g_fg_dbg_bat_r, store_FG_g_fg_dbg_bat_r);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_bat_car(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_car);
}

static ssize_t store_FG_g_fg_dbg_bat_car(struct device *dev, struct device_attribute *attr,
					 const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_car, 0664, show_FG_g_fg_dbg_bat_car, store_FG_g_fg_dbg_bat_car);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_bat_qmax(struct device *dev, struct device_attribute *attr,
					 char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_bat_qmax);
}

static ssize_t store_FG_g_fg_dbg_bat_qmax(struct device *dev, struct device_attribute *attr,
					  const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_bat_qmax, 0664, show_FG_g_fg_dbg_bat_qmax,
		   store_FG_g_fg_dbg_bat_qmax);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_d0(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_d0);
}

static ssize_t store_FG_g_fg_dbg_d0(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_d0, 0664, show_FG_g_fg_dbg_d0, store_FG_g_fg_dbg_d0);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_d1(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_d1);
}

static ssize_t store_FG_g_fg_dbg_d1(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_d1, 0664, show_FG_g_fg_dbg_d1, store_FG_g_fg_dbg_d1);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_percentage(struct device *dev, struct device_attribute *attr,
					   char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_percentage);
}

static ssize_t store_FG_g_fg_dbg_percentage(struct device *dev, struct device_attribute *attr,
					    const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_percentage, 0664, show_FG_g_fg_dbg_percentage,
		   store_FG_g_fg_dbg_percentage);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_percentage_fg(struct device *dev, struct device_attribute *attr,
					      char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_percentage_fg);
}

static ssize_t store_FG_g_fg_dbg_percentage_fg(struct device *dev, struct device_attribute *attr,
					       const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_percentage_fg, 0664, show_FG_g_fg_dbg_percentage_fg,
		   store_FG_g_fg_dbg_percentage_fg);
/* ------------------------------------------------------------------------------------------- */
static ssize_t show_FG_g_fg_dbg_percentage_voltmode(struct device *dev,
						    struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_fg_dbg_percentage_voltmode);
}

static ssize_t store_FG_g_fg_dbg_percentage_voltmode(struct device *dev,
						     struct device_attribute *attr, const char *buf,
						     size_t size)
{
	return size;
}

static DEVICE_ATTR(FG_g_fg_dbg_percentage_voltmode, 0664, show_FG_g_fg_dbg_percentage_voltmode,
		   store_FG_g_fg_dbg_percentage_voltmode);

/* ============================================================ // */
static int battery_meter_probe(struct platform_device *dev)
{
	int ret_device_file = 0;
#if defined(CONFIG_MTK_KERNEL_POWER_OFF_CHARGING)
	char *temp_strptr;
#endif
	battery_meter_ctrl = bm_ctrl_cmd;

	batt_meter_init_cust_data();

	/* select battery meter control method */
	battery_meter_ctrl = bm_ctrl_cmd;
#if defined(CONFIG_MTK_KERNEL_POWER_OFF_CHARGING)
	if (get_boot_mode() == LOW_POWER_OFF_CHARGING_BOOT
	    || get_boot_mode() == KERNEL_POWER_OFF_CHARGING_BOOT) {
		temp_strptr =
		    kzalloc(strlen(saved_command_line) + strlen(" androidboot.mode=charger") + 1,
			    GFP_KERNEL);
		strcpy(temp_strptr, saved_command_line);
		strcat(temp_strptr, " androidboot.mode=charger");
		saved_command_line = temp_strptr;
	}
#endif
	/* LOG System Set */
	init_proc_log_fg();

	/* last_oam_run_time = rtc_read_hw_time(); */
	get_monotonic_boottime(&last_oam_run_time);
	/* Create File For FG UI DEBUG */
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Current);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_volt);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_current);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_zcv);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_temp);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_r);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_car);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_qmax);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_d0);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_d1);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_percentage);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_percentage_fg);
	ret_device_file =
	    device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_percentage_voltmode);

#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Battery_Cycle);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Aging_Factor);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Max_Battery_Voltage);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Min_Battery_Voltage);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Max_Battery_Current);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Min_Battery_Current);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Max_Battery_Temperature);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Min_Battery_Temperature);
#endif

	return 0;
}

static int battery_meter_remove(struct platform_device *dev)
{
	return 0;
}

static void battery_meter_shutdown(struct platform_device *dev)
{
}

static int battery_meter_suspend(struct platform_device *dev, pm_message_t state)
{

#if defined(FG_BAT_INT)
#if defined(CONFIG_POWER_EXT)
#elif defined(SOC_BY_HW_FG)
	if (reset_fg_bat_int == KAL_TRUE) {
		battery_meter_set_columb_interrupt(gFG_BATT_CAPACITY / 100);
		reset_fg_bat_int = KAL_FALSE;
	} else {
		battery_meter_set_columb_interrupt(0x1ffff);
	}
#endif
#endif				/* #if defined(FG_BAT_INT) */

	/* -- hibernation path */
	if (state.event == PM_EVENT_FREEZE) {
		pr_warn("[%s] %p:%p\n", __func__, battery_meter_ctrl, &bm_ctrl_cmd);
		battery_meter_ctrl = bm_ctrl_cmd;
	}
	/* -- end of hibernation path */
#if defined(CONFIG_POWER_EXT)

#elif defined(SOC_BY_SW_FG) || defined(SOC_BY_HW_FG)
	{
#ifdef MTK_POWER_EXT_DETECT
		if (KAL_TRUE == bat_is_ext_power())
			return 0;
#endif
		get_monotonic_boottime(&xts_before_sleep);
		get_monotonic_boottime(&g_rtc_time_before_sleep);
		//if (_g_bat_sleep_total_time < g_spm_timer) 
		if ( (_g_bat_sleep_total_time < g_spm_timer) && (oam_delta_time < g_reflash_timer) ) 
			return 0;


		g_sleep_total_time.tv_sec = 0;
		g_sleep_total_time.tv_nsec = 0;

		battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &g_hw_ocv_before_sleep);
	}
#endif
	return 0;
}

#if defined(SOC_BY_HW_FG)
#ifdef MTK_ENABLE_AGING_ALGORITHM
void battery_aging_check(void)
{
	signed int hw_ocv_after_sleep;
	struct timespec xts;
	signed int vbat;
	signed int qmax_aging = 0;
	signed int dod_gap = 10;
	signed int columb_after_sleep = 0;
#if defined(MD_SLEEP_CURRENT_CHECK)
	signed int DOD_hwocv;
	signed int DOD_now;
	signed int suspend_current = 0;
#endif

	battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &hw_ocv_after_sleep);
	vbat = battery_meter_get_battery_voltage(KAL_TRUE);
	/* gauge correct */
	battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &columb_after_sleep);
	/* update columb counter to get DOD_now. */

	get_monotonic_boottime(&xts);
	suspend_time += abs(xts.tv_sec - xts_before_sleep.tv_sec);
	_g_bat_sleep_total_time += abs(xts.tv_sec - xts_before_sleep.tv_sec);
#if defined(MD_SLEEP_CURRENT_CHECK)
	if (columb_before_sleep == 0x123456) {
		columb_before_sleep = columb_after_sleep;
		suspend_time = 0;
		return;
	}
	if (hw_ocv_after_sleep != g_hw_ocv_before_sleep) {
		if (suspend_time > OCV_RECOVER_TIME) {	/* 35 mins */
			suspend_current =
			    abs(columb_after_sleep - columb_before_sleep) * 3600 / suspend_time;
			if (suspend_current < 10) {	/* 10mA */
				columb_before_sleep = columb_after_sleep;
				suspend_time = 0;
			} else {
				columb_before_sleep = columb_after_sleep;
				suspend_time = 0;
				return;
			}
		} else {
			return;
		}
	} else {
		return;
	}
#endif
	/* aging */
#if !defined(MD_SLEEP_CURRENT_CHECK)
if (suspend_time > OCV_RECOVER_TIME)
#endif
	{
		if (aging_ocv_1 == 0) {
			aging_ocv_1 = hw_ocv_after_sleep;
			aging_car_1 = columb_after_sleep;
			/* aging_resume_time_1 = time_after_sleep.tv_sec; */

			if (fgauge_read_d_by_v(aging_ocv_1) > DOD1_ABOVE_THRESHOLD) {
				aging_ocv_1 = 0;
			}
		} else if (aging_ocv_2 == 0) {
			aging_ocv_2 = hw_ocv_after_sleep;
			aging_car_2 = columb_after_sleep;
			/* aging_resume_time_2 = time_after_sleep.tv_sec; */

			if (fgauge_read_d_by_v(aging_ocv_2) < DOD2_BELOW_THRESHOLD) {
				aging_ocv_2 = 0;
			}
		} else {
			aging_ocv_1 = aging_ocv_2;
			aging_car_1 = aging_car_2;
			/* aging_resume_time_1 = aging_resume_time_2; */

			aging_ocv_2 = hw_ocv_after_sleep;
			aging_car_2 = columb_after_sleep;
			/* aging_resume_time_2 = time_after_sleep.tv_sec; */
		}
	}

	if (aging_ocv_2 > 0) {
		aging_dod_1 = fgauge_read_d_by_v(aging_ocv_1);
		aging_dod_2 = fgauge_read_d_by_v(aging_ocv_2);

		/* check dod region to avoid hwocv error margin */
		dod_gap = MIN_DOD_DIFF_THRESHOLD;

		/* check if DOD gap bigger than setting */
		if (aging_dod_2 > aging_dod_1 && (aging_dod_2 - aging_dod_1) >= dod_gap) {
			/* do aging calculation */
			qmax_aging =
			    (100 * (aging_car_1 - aging_car_2)) / (aging_dod_2 - aging_dod_1);

			/* update if aging over 10%. */
			if (gFG_BATT_CAPACITY > qmax_aging
			    && ((gFG_BATT_CAPACITY - qmax_aging) >
				(gFG_BATT_CAPACITY / (100 - MIN_AGING_FACTOR)))) {

#ifdef MTK_BATTERY_LIFETIME_DATA_SUPPORT
				gFG_aging_factor =
				    ((gFG_BATT_CAPACITY - qmax_aging) * 100) / gFG_BATT_CAPACITY;
#endif

				if (gFG_BATT_CAPACITY_aging > qmax_aging) {
					gFG_BATT_CAPACITY_aging = qmax_aging;
					gFG_DOD0 = aging_dod_2;
					gFG_DOD1 = gFG_DOD0;
					reset_parameter_car();
				}
			} else {
				aging_ocv_2 = 0;
			}
		} else {
			aging_ocv_2 = 0;
		}
	}
#if defined(MD_SLEEP_CURRENT_CHECK)
	/* self-discharging */
	if (hw_ocv_after_sleep < vbat) {
	} else {

		DOD_hwocv = fgauge_read_d_by_v(hw_ocv_after_sleep);

		battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);
		/* update columb counter to get DOD_now. */
		DOD_now = 100 - fgauge_read_capacity(1);

		if (DOD_hwocv > DOD_now && (DOD_hwocv - DOD_now > SELF_DISCHARGE_CHECK_THRESHOLD)) {
			gFG_DOD0 = DOD_hwocv;
			gFG_DOD1 = gFG_DOD0;
			reset_parameter_car();
		}
	}
#endif
}
#endif
#endif

//BEGIN <><optimize the soft fule-gauge><20160429> panzaoyan
static int battery_meter_resume(struct platform_device *dev)
{

#if defined(SOC_BY_SW_FG)
	int vol_bat1=0;
	 int HW_OCV1=0;
	 int D0=0;
	 int ret=0;
#endif
#if defined(CONFIG_POWER_EXT)

#elif defined(SOC_BY_SW_FG) || defined(SOC_BY_HW_FG)
#if defined(SOC_BY_SW_FG)
	signed int hw_ocv_after_sleep;
#endif
	struct timespec rtc_time_after_sleep;
	struct timespec now_time;
#ifdef MTK_POWER_EXT_DETECT
	if (KAL_TRUE == bat_is_ext_power())
		return 0;
#endif
	get_monotonic_boottime(&now_time);
	oam_delta_time = now_time.tv_sec - last_oam_run_time.tv_sec;

	get_monotonic_boottime(&rtc_time_after_sleep);

	g_sleep_total_time = timespec_add(g_sleep_total_time,
		timespec_sub(rtc_time_after_sleep, g_rtc_time_before_sleep));
	_g_bat_sleep_total_time = g_sleep_total_time.tv_sec;

	oam_delta_time = oam_delta_time + _g_bat_sleep_total_time;
	g_sleep_oam_time = g_sleep_oam_time + (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec);

	battery_log(BAT_LOG_CRTI,
			"[battery_meter_resume] sleep time = %d, g_spm_timer = %d , %ld %ld %ld %ld %ld %ld %d %d %d\n",
			_g_bat_sleep_total_time, g_spm_timer,
			g_rtc_time_before_sleep.tv_sec, g_rtc_time_before_sleep.tv_nsec,
			rtc_time_after_sleep.tv_sec, rtc_time_after_sleep.tv_nsec,
			g_sleep_total_time.tv_sec, g_sleep_total_time.tv_nsec, oam_delta_time, g_sleep_oam_time, g_reflash_timer);

//END <><optimize the soft fule-gauge><20160429> panzaoyan
#if defined(SOC_BY_HW_FG)
#ifdef MTK_ENABLE_AGING_ALGORITHM
	if (bat_is_charger_exist() == KAL_FALSE)
		battery_aging_check();

#endif
#endif

	//if (_g_bat_sleep_total_time < g_spm_timer) {
	if ( (_g_bat_sleep_total_time < g_spm_timer) && (oam_delta_time < g_reflash_timer) ) {
		return 0;
	}
	d5_count = 60;
	bat_spm_timeout = true;
#if defined(SOC_BY_SW_FG)
	battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &hw_ocv_after_sleep);

//BEGIN <><optimize the soft fule-gauge><20160429> panzaoyan
	vol_bat1 = 15; 
	ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &vol_bat1);
	HW_OCV1 = vol_bat1 + mtk_imp_tracking(vol_bat1, 350, 5);  //35mA
	D0=fgauge_read_d_by_v(HW_OCV1);
	if (_g_bat_sleep_total_time > 60) {
		if ((hw_ocv_after_sleep <= g_hw_ocv_before_sleep) && (BMT_status.UI_SOC>100-D0))
		{   
		    if((D0 - oam_d0 >= 10 && oam_delta_time > 1800) 
                      || (D0 - oam_d0 >= 4 && D0-oam_d0 < 10 && oam_delta_time > 900) 
                      || (D0 - oam_d0 >= 2 && D0-oam_d0 < 4 && oam_delta_time > 300) 
                      || (D0 - oam_d0 == 1) || (sleep_reflash_oam > 2) ) {
			oam_d0 = D0;
			oam_v_ocv_2 = oam_v_ocv_1 = HW_OCV1;
			oam_car_1 = 0;
			oam_car_2 = 0;
			oam_d_5 = oam_d0;
			sleep_reflash_oam = 0;
		    } else {
			sleep_reflash_oam = sleep_reflash_oam + 1;
		        oam_car_1 = oam_car_1 + (40* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600);
		        oam_car_2 = oam_car_2 + (40* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600);
		    }
	       } else {
		    oam_car_1 = oam_car_1 + (40* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600);
		    oam_car_2 = oam_car_2 + (40* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600);
	       }
	} else {
		oam_car_1 = oam_car_1 + (40 * (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600); 
		oam_car_2 = oam_car_2 + (40 * (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600); 
		g_sleep_oam_time = 0;
	}
//END <><optimize the soft fule-gauge><20160429> panzaoyan

#endif
#endif

#if defined(FG_BAT_INT)
#if defined(CONFIG_POWER_EXT)
#elif defined(SOC_BY_HW_FG)
	battery_meter_set_columb_interrupt(0);
#endif
#endif				/* #if defined(FG_BAT_INT) */

	return 0;
}

/* ----------------------------------------------------- */

#ifdef CONFIG_OF
static const struct of_device_id mt_bat_meter_of_match[] = {
	{.compatible = "mediatek,bat_meter",},
	{},
};

MODULE_DEVICE_TABLE(of, mt_bat_meter_of_match);
#endif
struct platform_device battery_meter_device = {
	.name = "battery_meter",
	.id = -1,
};


static struct platform_driver battery_meter_driver = {
	.probe = battery_meter_probe,
	.remove = battery_meter_remove,
	.shutdown = battery_meter_shutdown,
	.suspend = battery_meter_suspend,
	.resume = battery_meter_resume,
	.driver = {
		   .name = "battery_meter",
		   },
};

static int battery_meter_dts_probe(struct platform_device *dev)
{
	int ret = 0;
	/* struct proc_dir_entry *entry = NULL; */

	battery_log(BAT_LOG_CRTI, "******** battery_meter_dts_probe!! ********\n");

	battery_meter_device.dev.of_node = dev->dev.of_node;
	ret = platform_device_register(&battery_meter_device);
	if (ret) {
		battery_log(BAT_LOG_CRTI,
			    "****[battery_meter_dts_probe] Unable to register device (%d)\n", ret);
		return ret;
	}
	return 0;

}

static struct platform_driver battery_meter_dts_driver = {
	.probe = battery_meter_dts_probe,
	.remove = NULL,
	.shutdown = NULL,
	.suspend = NULL,
	.resume = NULL,
	.driver = {
		   .name = "battery_meter_dts",
#ifdef CONFIG_OF
		   .of_match_table = mt_bat_meter_of_match,
#endif
		   },
};

static int __init battery_meter_init(void)
{
	int ret;

#ifdef CONFIG_OF
	/*  */
#else
	ret = platform_device_register(&battery_meter_device);
	if (ret) {
		return ret;
	}
#endif

	ret = platform_driver_register(&battery_meter_driver);
	if (ret) {
		return ret;
	}
#ifdef CONFIG_OF
	ret = platform_driver_register(&battery_meter_dts_driver);
#endif
	return 0;

}
#ifdef BATTERY_MODULE_INIT
/* #if 0 */
/* late_initcall(battery_meter_init); */
device_initcall(battery_meter_init);
#else
static void __exit battery_meter_exit(void)
{
}
module_init(battery_meter_init);
/* module_exit(battery_meter_exit); */
#endif

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("Battery Meter Device Driver");
MODULE_LICENSE("GPL");
