#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include "kd_camera_hw.h"
/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, args...)   do {} while (0)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG(fmt, args...) 		do {} while (0)
#define PK_ERR(fmt, arg...)         pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...)  do {} while (0)
#else
#define PK_DBG(a, ...)
#define PK_ERR(a, ...)
#define PK_XLOG_INFO(fmt, args...)
#endif

#if !defined(CONFIG_MTK_LEGACY)
/* GPIO Pin control*/
struct platform_device *cam_plt_dev = NULL;
struct pinctrl *camctrl = NULL;
struct pinctrl_state *cam0_pnd_h = NULL;
struct pinctrl_state *cam0_pnd_l = NULL;
struct pinctrl_state *cam0_rst_h = NULL;
struct pinctrl_state *cam0_rst_l = NULL;
struct pinctrl_state *cam1_pnd_h = NULL;
struct pinctrl_state *cam1_pnd_l = NULL;
struct pinctrl_state *cam1_rst_h = NULL;
struct pinctrl_state *cam1_rst_l = NULL;
struct pinctrl_state *cam_ldo0_h = NULL;
struct pinctrl_state *cam_ldo0_l = NULL;
struct pinctrl_state *sub_cam_avdd_h = NULL;
struct pinctrl_state *sub_cam_avdd_l = NULL;
struct pinctrl_state *sub_cam_dvdd_h = NULL;
struct pinctrl_state *sub_cam_dvdd_l = NULL;

int mtkcam_gpio_init(struct platform_device *pdev)
{
    int ret = 0;

    camctrl = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR(camctrl))
    {
        dev_err(&pdev->dev, "Cannot find camera pinctrl!");
        ret = PTR_ERR(camctrl);
    }
    /*Cam0 Power/Rst Ping initialization*/
    cam0_pnd_h = pinctrl_lookup_state(camctrl, "cam0_pnd1");
    if (IS_ERR(cam0_pnd_h))
    {
        ret = PTR_ERR(cam0_pnd_h);
        pr_debug("%s : pinctrl err, cam0_pnd_h\n", __func__);
    }

    cam0_pnd_l = pinctrl_lookup_state(camctrl, "cam0_pnd0");
    if (IS_ERR(cam0_pnd_l))
    {
        ret = PTR_ERR(cam0_pnd_l);
        pr_debug("%s : pinctrl err, cam0_pnd_l\n", __func__);
    }


    cam0_rst_h = pinctrl_lookup_state(camctrl, "cam0_rst1");
    if (IS_ERR(cam0_rst_h))
    {
        ret = PTR_ERR(cam0_rst_h);
        pr_debug("%s : pinctrl err, cam0_rst_h\n", __func__);
    }

    cam0_rst_l = pinctrl_lookup_state(camctrl, "cam0_rst0");
    if (IS_ERR(cam0_rst_l))
    {
        ret = PTR_ERR(cam0_rst_l);
        pr_debug("%s : pinctrl err, cam0_rst_l\n", __func__);
    }

    /*Cam1 Power/Rst Ping initialization*/
    cam1_pnd_h = pinctrl_lookup_state(camctrl, "cam1_pnd1");
    if (IS_ERR(cam1_pnd_h))
    {
        ret = PTR_ERR(cam1_pnd_h);
        pr_debug("%s : pinctrl err, cam1_pnd_h\n", __func__);
    }

    cam1_pnd_l = pinctrl_lookup_state(camctrl, "cam1_pnd0");
    if (IS_ERR(cam1_pnd_l ))
    {
        ret = PTR_ERR(cam1_pnd_l );
        pr_debug("%s : pinctrl err, cam1_pnd_l\n", __func__);
    }


    cam1_rst_h = pinctrl_lookup_state(camctrl, "cam1_rst1");
    if (IS_ERR(cam1_rst_h))
    {
        ret = PTR_ERR(cam1_rst_h);
        pr_debug("%s : pinctrl err, cam1_rst_h\n", __func__);
    }


    cam1_rst_l = pinctrl_lookup_state(camctrl, "cam1_rst0");
    if (IS_ERR(cam1_rst_l))
    {
        ret = PTR_ERR(cam1_rst_l);
        pr_debug("%s : pinctrl err, cam1_rst_l\n", __func__);
    }
    /*externel LDO enable */
    cam_ldo0_h = pinctrl_lookup_state(camctrl, "cam_ldo0_1");
    if (IS_ERR(cam_ldo0_h))
    {
        ret = PTR_ERR(cam_ldo0_h);
        pr_debug("%s : pinctrl err, cam_ldo0_h\n", __func__);
    }


    cam_ldo0_l = pinctrl_lookup_state(camctrl, "cam_ldo0_0");
    if (IS_ERR(cam_ldo0_l))
    {
        ret = PTR_ERR(cam_ldo0_l);
        pr_debug("%s : pinctrl err, cam_ldo0_l\n", __func__);
    }

    sub_cam_avdd_h = pinctrl_lookup_state(camctrl, "sub_cam_avdd_1");
    if (IS_ERR(sub_cam_avdd_h))
    {
        ret = PTR_ERR(sub_cam_avdd_h);
        pr_debug("%s : pinctrl err, sub_cam_avdd_h\n", __func__);
    }

    sub_cam_avdd_l = pinctrl_lookup_state(camctrl, "sub_cam_avdd_0");
    if (IS_ERR(sub_cam_avdd_l))
    {
        ret = PTR_ERR(sub_cam_avdd_l);
        pr_debug("%s : pinctrl err, sub_cam_avdd_h\n", __func__);
    }

    sub_cam_dvdd_h = pinctrl_lookup_state(camctrl, "sub_cam_dvdd_1");
    if (IS_ERR(sub_cam_dvdd_h))
    {
        ret = PTR_ERR(sub_cam_dvdd_h);
        pr_debug("%s : pinctrl err, sub_cam_dvdd_h\n", __func__);
    }

    sub_cam_dvdd_l = pinctrl_lookup_state(camctrl, "sub_cam_dvdd_0");
    if (IS_ERR(sub_cam_dvdd_l))
    {
        ret = PTR_ERR(sub_cam_dvdd_l);
        pr_debug("%s : pinctrl err, sub_cam_dvdd_h\n", __func__);
    }


    return ret;
}

int mtkcam_gpio_set(int PinIdx, int PwrType, int Val)
{
    int ret = 0;
    switch (PwrType)
    {
        case CAMRST:
            if (PinIdx == 0)
            {
                if (Val == 0)
                    pinctrl_select_state(camctrl, cam0_rst_l);
                else
                    pinctrl_select_state(camctrl, cam0_rst_h);
            }
            else
            {
                if (Val == 0)
                    pinctrl_select_state(camctrl, cam1_rst_l);
                else
                    pinctrl_select_state(camctrl, cam1_rst_h);
            }
            break;
        case CAMPDN:
            if (PinIdx == 0)
            {
                if (Val == 0)
                    pinctrl_select_state(camctrl, cam0_pnd_l);
                else
                    pinctrl_select_state(camctrl, cam0_pnd_h);
            }
            else
            {
                if (Val == 0)
                    pinctrl_select_state(camctrl, cam1_pnd_l);
                else
                    pinctrl_select_state(camctrl, cam1_pnd_h);
            }

            break;
        case CAMLDO:
            if (Val == 0)
                pinctrl_select_state(camctrl, cam_ldo0_l);
            else
                pinctrl_select_state(camctrl, cam_ldo0_h);
            break;
        case SUBCAMAVDD:
            if (Val == 0)
                pinctrl_select_state(camctrl, sub_cam_avdd_l);
            else
                pinctrl_select_state(camctrl, sub_cam_avdd_h);
            break;

        case SUBCAMDVDD:
            if (Val == 0)
                pinctrl_select_state(camctrl, sub_cam_dvdd_l);
            else
                pinctrl_select_state(camctrl, sub_cam_dvdd_h);

            break;
        default:
            PK_DBG("YC PwrType(%d) is invalid !!\n", PwrType);
            break;
    };

    PK_DBG("PinIdx(%d) PwrType(%d) val(%d)\n", PinIdx, PwrType, Val);

    return ret;
}
#define MainCamera_AVDD_PowerOn()  mtkcam_gpio_set(0, CAMLDO, 1);

#define MainCamera_AVDD_PowerDown() mtkcam_gpio_set(0, CAMLDO, 0);

#define SubCamera_AVDD_PowerOn() mtkcam_gpio_set(1, SUBCAMAVDD, 1);

#define SubCamera_AVDD_PowerDown() mtkcam_gpio_set(1, SUBCAMAVDD, 0);

#define SubCamera_DVDD_PowerOn() mtkcam_gpio_set(1, SUBCAMDVDD, 1);

#define SubCamera_DVDD_PowerDown() mtkcam_gpio_set(1, SUBCAMDVDD, 0);


int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, bool On, char *mode_name)
{

    u32 pinSetIdx = 0;/* default main sensor */

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3
#define VOL_2800 2800000
#define VOL_1800 1800000
#define VOL_1500 1500000
#define VOL_1200 1200000
#define VOL_1000 1000000


    u32 pinSet[2][8] =
    {
        /* for main sensor */
        {/* The reset pin of main sensor uses GPIO10 of mt6306, please call mt6306 API to set */
            CAMERA_CMRST_PIN,
            CAMERA_CMRST_PIN_M_GPIO,   /* mode */
            GPIO_OUT_ONE,              /* ON state */
            GPIO_OUT_ZERO,             /* OFF state */
            CAMERA_CMPDN_PIN,
            CAMERA_CMPDN_PIN_M_GPIO,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
        },
        /* for sub sensor */
        {
            CAMERA_CMRST1_PIN,
            CAMERA_CMRST1_PIN_M_GPIO,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
            CAMERA_CMPDN1_PIN,
            CAMERA_CMPDN1_PIN_M_GPIO,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
        },
    };

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
        pinSetIdx = 0;
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx)
        pinSetIdx = 1;

    /* power ON */
    if (On)
    {

#if 0
        ISP_MCLK1_EN(1);
        ISP_MCLK2_EN(1);
        ISP_MCLK3_EN(1);
#else
        if (pinSetIdx == 0)
            ISP_MCLK1_EN(1);
        else if (pinSetIdx == 1)
            ISP_MCLK2_EN(1);
#endif

        PK_DBG("[PowerON]pinSetIdx:%d, currSensorName: %s\n", pinSetIdx, currSensorName);


        if (currSensorName && (0 == strcmp("s5k3m2mipiraw", currSensorName)))
        {

            //printk("YC	currSensorName:  %s 	 power up	!!!!!!!!!!!!!!!!!!\n", currSensorName);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN, pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST, pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);
			mdelay(10);

            if (TRUE != _hwPowerOn(VCAMD, VOL_1000))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D), power id = %d\n", VCAMD);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);

            MainCamera_AVDD_PowerOn();
            mdelay(10);

            /* VCAM_IO */
            if (TRUE != _hwPowerOn(VCAMIO, VOL_1800))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable IO power (VCAM_IO), power id = %d\n", VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }

            /* AF_VCC */
            if (TRUE != _hwPowerOn(VCAMAF, VOL_2800))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable AF power (VCAM_AF), power id = %d\n", VCAMAF);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]);

            //mdelay(7);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST, pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]);
            //mdelay(40);
            //printk("[PowerON] s5k3m2mipiraw yixuhong \n");

#if 0
            //disable inactive sensor
            if(pinSetIdx == 0 || pinSetIdx == 2)
            {
                //disable sub
                if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST])
                {
                    if(mtkcam_gpio_set(1, CAMRST, pinSet[1][IDX_PS_CMRST + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");   //low == reset sensor
                    }
                    if(mtkcam_gpio_set(1, CAMPDN, pinSet[1][IDX_PS_CMPDN + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA LENS] set gpio failed!! \n");   //high == power down lens module
                    }
                }
            }
            else if(pinSetIdx == 1)
            {
                if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST])
                {
                    if(mtkcam_gpio_set(0, CAMRST, pinSet[0][IDX_PS_CMRST + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");   //low == reset sensor
                    }
                    if(mtkcam_gpio_set(0, CAMPDN, pinSet[0][IDX_PS_CMPDN + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA LENS] set gpio failed!! \n");   //high == power down lens module
                    }
                }
            }
#endif
        }

        else if (currSensorName && ((0 == strcmp("ov5648mipi_darling", currSensorName)) || (0 == strcmp("ov5648mipi_sunwin", currSensorName))))
        {
            //printk("YC  currSensorName:	%s		power up Start	!!!!!!!!!!!!!!!!!!\n", currSensorName);

            //First Power Pin low and Reset Pin Low
            //if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN, pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST, pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);
			mdelay(20);

            /* VCAM_IO */
            if (TRUE != _hwPowerOn(VCAMIO, VOL_1800))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable IO power (VCAM_IO), power id = %d\n", VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);

            //VCAM_A
            SubCamera_AVDD_PowerOn();

            mdelay(5);
            ////VCAM_D
            SubCamera_DVDD_PowerOn();
            mdelay(10);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN, pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]);

            //mdelay(2);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST, pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]);
            mdelay(20);
#if 0
            //disable inactive sensor
            if(pinSetIdx == 0 || pinSetIdx == 2)
            {
                //disable sub
                if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST])
                {
                    if(mtkcam_gpio_set(1, CAMRST, pinSet[1][IDX_PS_CMRST + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");   //low == reset sensor
                    }
                    if(mtkcam_gpio_set(1, CAMPDN, pinSet[1][IDX_PS_CMPDN + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA LENS] set gpio failed!! \n");   //high == power down lens module
                    }
                }
            }
            else if(pinSetIdx == 1)
            {
                if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST])
                {
                    if(mtkcam_gpio_set(0, CAMRST, pinSet[0][IDX_PS_CMRST + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");   //low == reset sensor
                    }
                    if(mtkcam_gpio_set(0, CAMPDN, pinSet[0][IDX_PS_CMPDN + IDX_PS_OFF]))
                    {
                        PK_DBG("[CAMERA LENS] set gpio failed!! \n");   //high == power down lens module
                    }
                }
            }
#endif
            //printk("YC	currSensorName:  %s 	 power up  EnD !!!!!!!!!!!!!!!!!!\n", currSensorName);

        }
        else
        {
            PK_DBG("kdCISModulePowerOn get in---  No support,Please add your sensor power up code \n");
        }
    }
    else     /* power OFF */
    {

        PK_DBG("[PowerOFF]pinSetIdx:%d, currSensorName: %s\n", pinSetIdx, currSensorName);
        if (pinSetIdx == 0)
            ISP_MCLK1_EN(0);
        else if (pinSetIdx == 1)
            ISP_MCLK2_EN(0);

        if (currSensorName && (0 == strcmp("s5k3m2mipiraw", currSensorName)))
        {
            //printk("YC  currSensorName:  %s	   power down	!!!!!!!!!!!!!!!!!!\n", currSensorName);
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN, pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);

            /* Set Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST, pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);

            MainCamera_AVDD_PowerDown();
            mdelay(1);

            if (TRUE != _hwPowerDown(VCAMD))
            {
                PK_DBG("[CAMERA SENSOR] Fail to disenable digital power (VCAM_D), power id = %d\n", VCAMD);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(1);

            /* VCAM_IO */
            if (TRUE != _hwPowerDown(VCAMIO))
            {
                PK_DBG("[CAMERA SENSOR] Fail to disenable IO power (VCAM_IO), power id = %d\n", VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);

            /* AF_VCC */
            if (TRUE != _hwPowerDown(VCAMAF))
            {
                PK_DBG("[CAMERA SENSOR] Fail to disenable AF power (VCAM_AF), power id = %d\n", VCAMAF);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(1);
        }
        else if (currSensorName && ((0 == strcmp("ov5648mipi_darling", currSensorName)) || (0 == strcmp("ov5648mipi_sunwin", currSensorName))))
        {
            //printk("YC  currSensorName:  %s	   power down	!!!!!!!!!!!!!!!!!!\n", currSensorName);

            //First Power Pin low and Reset Pin Low
            /* Set Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN, pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST, pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);

            //VCAM_D
            SubCamera_DVDD_PowerDown();

            //VCAM_A
            SubCamera_AVDD_PowerDown();
            /* VCAM_IO */
            if (TRUE != _hwPowerDown(VCAMIO))
            {
                PK_DBG("[CAMERA SENSOR] Fail to disenable IO power (VCAM_IO), power id = %d\n", VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }
			mdelay(20);


        }
        else
        {
            PK_DBG("kdCISModulePower--off get in---no support \n");
        }

    }

    return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;

}
#else
/*Legacy define*/
int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char *mode_name)
{

    u32 pinSetIdx = 0;/*default main sensor*/

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3


    u32 pinSet[3][8] =
    {
        /* for main sensor */
        {
            /* The reset pin of main sensor uses GPIO10 of mt6306, please call mt6306 API to set */
            CAMERA_CMRST_PIN,
            CAMERA_CMRST_PIN_M_GPIO,   /* mode */
            GPIO_OUT_ONE,              /* ON state */
            GPIO_OUT_ZERO,             /* OFF state */
            CAMERA_CMPDN_PIN,
            CAMERA_CMPDN_PIN_M_GPIO,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
        },
        /* for sub sensor */
        {
            CAMERA_CMRST1_PIN,
            CAMERA_CMRST1_PIN_M_GPIO,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
            CAMERA_CMPDN1_PIN,
            CAMERA_CMPDN1_PIN_M_GPIO,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
        },
        /* for main_2 sensor */
        {
            GPIO_CAMERA_INVALID,
            GPIO_CAMERA_INVALID,   /* mode */
            GPIO_OUT_ONE,               /* ON state */
            GPIO_OUT_ZERO,              /* OFF state */
            GPIO_CAMERA_INVALID,
            GPIO_CAMERA_INVALID,
            GPIO_OUT_ONE,
            GPIO_OUT_ZERO,
        }
    };



    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
        pinSetIdx = 0;
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx)
        pinSetIdx = 1;
    else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx)
        pinSetIdx = 2;


    /* power ON */
    if (On)
    {

#if 0
        ISP_MCLK1_EN(1);
        ISP_MCLK2_EN(1);
        ISP_MCLK3_EN(1);
#else
        if (pinSetIdx == 0)
            ISP_MCLK1_EN(1);
        else if (pinSetIdx == 1)
            ISP_MCLK2_EN(1);
#endif

        PK_DBG("[PowerON]pinSetIdx:%d, currSensorName: %s\n", pinSetIdx, currSensorName);

        if ((currSensorName && (0 == strcmp(currSensorName, "imx135mipiraw"))) ||
            (currSensorName && (0 == strcmp(currSensorName, "imx220mipiraw"))))
        {
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            /* AF_VCC */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            /* VCAM_A */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A),power id = %d\n",
                       CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1000, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D),power id = %d\n",
                       CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            /* VCAM_IO */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(2);


            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

        }
        else if (currSensorName && (0 == strcmp("ov5648mipiraw",
                                                currSensorName)))
        {
            mt_set_gpio_mode(GPIO_SPI_MOSI_PIN, GPIO_MODE_00);
            mt_set_gpio_dir(GPIO_SPI_MOSI_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN, GPIO_OUT_ONE);
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }


            /* VCAM_IO */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO),power id = %d\n", CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            /* VCAM_A */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A),power id = %d\n",
                       CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            if (TRUE != hwPowerOn(SUB_CAMERA_POWER_VCAM_D, VOL_1500, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D),power id = %d\n",
                       CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);

            /* AF_VCC */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }


            mdelay(1);


            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }


            mdelay(2);


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            mdelay(20);
        }
        else  if (currSensorName && (0 == strcmp("gc2355mipiraw", currSensorName)))
        {
            mt_set_gpio_mode(GPIO_SPI_MOSI_PIN, GPIO_MODE_00);
            mt_set_gpio_dir(GPIO_SPI_MOSI_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN, GPIO_OUT_ONE);
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            mdelay(50);

            /* VCAM_A */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A),power id = %d\n",
                       CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            /* VCAM_IO */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            if (TRUE != hwPowerOn(SUB_CAMERA_POWER_VCAM_D, VOL_1500, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D),power id = %d\n",
                       CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            /* AF_VCC */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }


            mdelay(50);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
                mdelay(5);
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");

            }
            mdelay(5);
            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
            }
            if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
            if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]))
                PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            mdelay(5);
            if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
        }
        else
        {
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            /* VCAM_IO */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_A */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A),power id = %d\n",
                       CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_D */
            if (currSensorName && (0 == strcmp("s5k2p8mipiraw", currSensorName)))
            {
                if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200, mode_name))
                {
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                    goto _kdCISModulePowerOn_exit_;
                }
            }
            else if (currSensorName && (0 == strcmp("imx219mipiraw", currSensorName)))
            {
                if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200, mode_name))
                {
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                    goto _kdCISModulePowerOn_exit_;
                }
            }
            else     /* Main VCAMD max 1.5V */
            {
                if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500, mode_name))
                {
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                    goto _kdCISModulePowerOn_exit_;
                }

            }


            /* AF_VCC */
            if (TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);

            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            mdelay(1);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }
        }
    }
    else     /* power OFF */
    {

        PK_DBG("[PowerOFF]pinSetIdx:%d\n", pinSetIdx);
        if (pinSetIdx == 0)
            ISP_MCLK1_EN(0);
        else if (pinSetIdx == 1)
            ISP_MCLK2_EN(0);

        if ((currSensorName && (0 == strcmp(currSensorName, "imx135mipiraw"))) ||
            (currSensorName && (0 == strcmp(currSensorName, "imx220mipiraw"))))
        {
            /* Set Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            /* Set Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            /* AF_VCC */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_IO */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D),power id = %d\n",
                       CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_A */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_A, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                       CAMERA_POWER_VCAM_A);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

        }
        else if (currSensorName && (0 == strcmp("ov5648mipiraw", currSensorName)))
        {
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN, GPIO_OUT_ZERO);
            /* Set Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }

            if (TRUE != hwPowerDown(SUB_CAMERA_POWER_VCAM_D, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D),power id = %d\n",
                       SUB_CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_A */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_A, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                       CAMERA_POWER_VCAM_A);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_IO */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* AF_VCC */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

        }
        else if (currSensorName && (0 == strcmp("gc2355mipiraw", currSensorName)))
        {
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN, GPIO_OUT_ZERO);
            /* Set Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }


            if (TRUE != hwPowerDown(SUB_CAMERA_POWER_VCAM_D, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D),power id = %d\n",
                       SUB_CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_A */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_A, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                       CAMERA_POWER_VCAM_A);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_IO */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* AF_VCC */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

        }
        else
        {
            /* Set Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                     pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_MODE]))
                    PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT))
                    PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],
                                    pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]))
                    PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");
            }


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
            {
                if (mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],
                                     pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_MODE]))
                    PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");
                if (mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    GPIO_DIR_OUT))
                    PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");
                if (mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],
                                    pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]))
                    PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");
            }


            if (TRUE != hwPowerDown(SUB_CAMERA_POWER_VCAM_D, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D),power id = %d\n",
                       SUB_CAMERA_POWER_VCAM_D);
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_A */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_A, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                       CAMERA_POWER_VCAM_A);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* VCAM_IO */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO),power id = %d\n",
                       CAMERA_POWER_VCAM_IO);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

            /* AF_VCC */
            if (TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n",
                       CAMERA_POWER_VCAM_AF);
                /* return -EIO; */
                goto _kdCISModulePowerOn_exit_;
            }

        }

    }

    return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

#endif
EXPORT_SYMBOL(kdCISModulePowerOn);

/* !-- */
/*  */

