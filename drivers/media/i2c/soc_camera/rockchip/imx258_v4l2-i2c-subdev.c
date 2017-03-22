/*
 * IMX258 sensor driver
 *
 * Copyright (C) 2016 Fuzhou Rockchip Electronics Co., Ltd.
 *
 * Copyright (C) 2012-2014 Intel Mobile Communications GmbH
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 * Note:
 *
 *v0.1.0:
 *1. Initialize version;
 */

#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf-core.h>
#include <linux/slab.h>
#include "imx_camera_module.h"

#define IMX258_DRIVER_NAME "imx258"

#define IMX258_AEC_PK_GAIN_HIGH_REG 0x0204
#define IMX258_AEC_PK_GAIN_LOW_REG 0x0205

#define IMX258_AEC_PK_EXPO_HIGH_REG 0x0202
#define IMX258_AEC_PK_EXPO_LOW_REG 0x0203

#define IMX258_FETCH_HIGH_BYTE_EXP(VAL) ((VAL >> 8) & 0xFF)
#define IMX258_FETCH_LOW_BYTE_EXP(VAL) (VAL & 0xFF)

#define IMX258_PIDH_ADDR 0x0016
#define IMX258_PIDL_ADDR 0x0017
#define IMX258_PIDH_MAGIC 0x02
#define IMX258_PIDL_MAGIC 0x58

#define IMX258_TIMING_VTS_HIGH_REG 0x0340
#define IMX258_TIMING_VTS_LOW_REG 0x0341
#define IMX258_TIMING_HTS_HIGH_REG 0x0342
#define IMX258_TIMING_HTS_LOW_REG 0x0343
#define IMX258_TIMING_X_INC 0x0383
#define IMX258_TIMING_Y_INC 0x0387
#define IMX258_HORIZONTAL_START_HIGH_REG 0x0344
#define IMX258_HORIZONTAL_START_LOW_REG 0x0345
#define IMX258_VERTICAL_START_HIGH_REG 0x0346
#define IMX258_VERTICAL_START_LOW_REG 0x0347
#define IMX258_HORIZONTAL_END_HIGH_REG 0x0348
#define IMX258_HORIZONTAL_END_LOW_REG 0x0349
#define IMX258_VERTICAL_END_HIGH_REG 0x034a
#define IMX258_VERTICAL_END_LOW_REG 0x034b
#define IMX258_HORIZONTAL_OUTPUT_SIZE_HIGH_REG 0x034c
#define IMX258_HORIZONTAL_OUTPUT_SIZE_LOW_REG 0x034d
#define IMX258_VERTICAL_OUTPUT_SIZE_HIGH_REG 0x034e
#define IMX258_VERTICAL_OUTPUT_SIZE_LOW_REG 0x034f

#define IMX258_INTEGRATION_TIME_MARGIN 8
#define IMX258_FINE_INTG_TIME_MIN 0
#define IMX258_FINE_INTG_TIME_MAX_MARGIN 0
#define IMX258_COARSE_INTG_TIME_MIN 16
#define IMX258_COARSE_INTG_TIME_MAX_MARGIN 4

#define IMX258_ORIENTATION_REG 0x0101
#define IMX258_ORIENTATION_H 0x1
#define IMX258_ORIENTATION_V 0x2

#define IMX258_EXT_CLK 24000000

static struct imx_camera_module imx258;
static struct imx_camera_module_custom_config imx258_custom_config;

/* ======================================================================== */
/* Base sensor configs */
/* ======================================================================== */

/* MCLK:24MHz  3264x1836  30fps  4Lane   798Mbps/lane */
static struct imx_camera_module_reg imx258_init_tab_3264_1836_30fps[] = {
	/* External Clock Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0136, 0x18},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0137, 0x00},
	/* Global Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3051, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b11, 0xcf},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff0, 0x08},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff1, 0x0f},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff2, 0x08},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff3, 0x1b},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff4, 0x23},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff5, 0x60},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff6, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff7, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff8, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff9, 0x78},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffa, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffb, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffc, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffd, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffe, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fff, 0x03},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7f76, 0x03},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7f77, 0xfe},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fa8, 0x03},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fa9, 0xfe},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7b24, 0x81},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7b25, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6564, 0x07},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b0d, 0x41},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x653d, 0x04},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b05, 0x8c},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b06, 0xf9},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b08, 0x65},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b09, 0xfc},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b0a, 0xcf},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b0b, 0xd2},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6700, 0x0e},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6707, 0x0e},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9104, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7421, 0x1c},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7423, 0xd7},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x5f04, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x5f05, 0xed},
	/* Output Format Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0101, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0112, 0x0a},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0113, 0x0a},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0114, 0x03},
	/* Clock Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0301, 0x05},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0303, 0x02},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0305, 0x04},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0306, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0307, 0x82},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0309, 0x0a},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030b, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030d, 0x02},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030e, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030f, 0xd8},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0310, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0820, 0x0c},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0821, 0x30},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0822, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0823, 0x00},
	/* Line Length Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0342, 0x14},//HTS[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0343, 0xe8},//HTS[7:0]
	/* Frame Length Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0340, 0x07},//VTS[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0341, 0x98},//VTS[7:0]
	/* ROI Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0344, 0x00},//X_ADD_STA[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0345, 0x00},//X_ADD_STA[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0346, 0x02},//Y_ADD_STA[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0347, 0x82},//Y_ADD_STA[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0348, 0x10},//X_ADD_END[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0349, 0x6f},//X_ADD_END[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034a, 0x09},//Y_ADD_END[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034b, 0xad},//Y_ADD_END[7:0]
	/* Analog Image Size Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0381, 0x01},//X_EVN_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0383, 0x01},//X_ODD_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0385, 0x01},//Y_EVN_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0387, 0x01},//Y_ODD_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0900, 0x00},//BINNING_MODE
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0901, 0x11},//BINNING_TYPE_V
	/* Digital Image Size Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0401, 0x00},//SCALE_MODE[1:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0404, 0x00},//SCALE_M[8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0405, 0x10},//SCALE_M[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0408, 0x01},//DIG_CROP_X_OFFSET[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0409, 0xd8},//DIG_CROP_X_OFFSET[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040a, 0x00},//DIG_CROP_Y_OFFSET[11:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040b, 0x00},//DIG_CROP_Y_OFFSET[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040c, 0x0c},//DIG_CROP_IMAGE_WIDTH[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040d, 0xc0},//DIG_CROP_IMAGE_WIDTH[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040e, 0x07},//DIG_CROP_IMAGE_HEIGHT[11:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040f, 0x2c},//DIG_CROP_IMAGE_HEIGHT[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3038, 0x00},//SCALE_MODE_EXT
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x303a, 0x00},//SCALE_M_EXT[8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x303b, 0x10},//SCALE_M_EXT[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x300d, 0x00},//FORCE_FD_SUM
	/* Output Size Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034c, 0x0c},//X_OUT_SIZE[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034d, 0xc0},//X_OUT_SIZE[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034e, 0x07},//Y_OUT_SIZE[11:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034f, 0x2c},//Y_OUT_SIZE[7:0]
	/* Integration Time Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0202, 0x07},//COARSE_INTEG_TIME[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0203, 0x8e},//COARSE_INTEG_TIME[7:0]
	/* Gain Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0204, 0x01},//ANA_GAIN_GLOBAL[8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0205, 0x00},//ANA_GAIN_GLOBAL[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x020e, 0x01},//DIG_GAIN_GR[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x020f, 0x00},//DIG_GAIN_GR[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0210, 0x01},//DIG_GAIN_R[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0211, 0x00},//DIG_GAIN_R[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0212, 0x01},//DIG_GAIN_B[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0213, 0x00},//DIG_GAIN_B[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0214, 0x01},//DIG_GAIN_GB[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0215, 0x00},//DIG_GAIN_GB[7:0]
	/* Added Setting AF */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7bcd, 0x00},
	/* Added Setting IQ */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x94dc, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x94dd, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x94de, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x95dc, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x95dd, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x95de, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fb0, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9010, 0x3e},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9419, 0x50},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x941b, 0x50},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9519, 0x50},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x951b, 0x50},
	/* Added Setting mode */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3030, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3032, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0220, 0x00}
};

/* MCLK:24MHz  3264x2448  24fps  4Lane   798Mbps/lane */
static struct imx_camera_module_reg imx258_init_tab_3264_2448_24fps[] = {
	/* External Clock Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0136, 0x18},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0137, 0x00},
	/* Global Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3051, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b11, 0xcf},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff0, 0x08},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff1, 0x0f},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff2, 0x08},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff3, 0x1b},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff4, 0x23},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff5, 0x60},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff6, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff7, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff8, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ff9, 0x78},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffa, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffb, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffc, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffd, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7ffe, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fff, 0x03},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7f76, 0x03},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7f77, 0xfe},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fa8, 0x03},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fa9, 0xfe},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7b24, 0x81},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7b25, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6564, 0x07},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b0d, 0x41},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x653d, 0x04},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b05, 0x8c},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b06, 0xf9},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b08, 0x65},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b09, 0xfc},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b0a, 0xcf},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6b0b, 0xd2},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6700, 0x0e},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x6707, 0x0e},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9104, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7421, 0x1c},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7423, 0xd7},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x5f04, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x5f05, 0xed},
	/* Output Format Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0101, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0112, 0x0a},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0113, 0x0a},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0114, 0x03},
	/* Clock Setting*/
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0301, 0x05},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0303, 0x02},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0305, 0x04},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0306, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0307, 0x82},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0309, 0x0a},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030b, 0x01},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030d, 0x02},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030e, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x030f, 0xd8},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0310, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0820, 0x0c},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0821, 0x30},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0822, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0823, 0x00},
	/* Line Length Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0342, 0x14},//HTS[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0343, 0xe8},//HTS[7:0]
	/* Frame Length Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0340, 0x09},//VTS[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0341, 0xb0},//VTS[7:0]
	/* ROI Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0344, 0x00},//X_ADD_STA[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0345, 0x00},//X_ADD_STA[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0346, 0x01},//Y_ADD_STA[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0347, 0x50},//Y_ADD_STA[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0348, 0x10},//X_ADD_END[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0349, 0x6f},//X_ADD_END[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034a, 0x0a},//Y_ADD_END[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034b, 0xdf},//Y_ADD_END[7:0]
	/* Analog Image Size Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0381, 0x01},//X_EVN_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0383, 0x01},//X_ODD_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0385, 0x01},//Y_EVN_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0387, 0x01},//Y_ODD_INC
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0900, 0x00},//BINNING_MODE
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0901, 0x11},//BINNING_TYPE_V
	/* Digital Image Size Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0401, 0x00},//SCALE_MODE[1:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0404, 0x00},//SCALE_M[8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0405, 0x10},//SCALE_M[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0408, 0x01},//DIG_CROP_X_OFFSET[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0409, 0xd8},//DIG_CROP_X_OFFSET[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040a, 0x00},//DIG_CROP_Y_OFFSET[11:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040b, 0x00},//DIG_CROP_Y_OFFSET[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040c, 0x0c},//DIG_CROP_IMAGE_WIDTH[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040d, 0xc0},//DIG_CROP_IMAGE_WIDTH[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040e, 0x09},//DIG_CROP_IMAGE_HEIGHT[11:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x040f, 0x90},//DIG_CROP_IMAGE_HEIGHT[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3038, 0x00},//SCALE_MODE_EXT
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x303a, 0x00},//SCALE_M_EXT[8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x303b, 0x10},//SCALE_M_EXT[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x300d, 0x00},//FORCE_FD_SUM
	/* Output Size Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034c, 0x0c},//X_OUT_SIZE[12:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034d, 0xc0},//X_OUT_SIZE[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034e, 0x09},//Y_OUT_SIZE[11:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x034f, 0x90},//Y_OUT_SIZE[7:0]
	/* Integration Time Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0202, 0x09},//COARSE_INTEG_TIME[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0203, 0xa6},//COARSE_INTEG_TIME[7:0]
	/* Gain Setting */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0204, 0x01},//ANA_GAIN_GLOBAL[8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0205, 0x00},//ANA_GAIN_GLOBAL[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x020e, 0x01},//DIG_GAIN_GR[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x020f, 0x00},//DIG_GAIN_GR[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0210, 0x01},//DIG_GAIN_R[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0211, 0x00},//DIG_GAIN_R[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0212, 0x01},//DIG_GAIN_B[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0213, 0x00},//DIG_GAIN_B[7:0]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0214, 0x01},//DIG_GAIN_GB[15:8]
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0215, 0x00},//DIG_GAIN_GB[7:0]
	/* Added Setting AF */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7bcd, 0x00},
	/* Added Setting IQ */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x94dc, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x94dd, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x94de, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x95dc, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x95dd, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x95de, 0x20},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x7fb0, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9010, 0x3e},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9419, 0x50},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x941b, 0x50},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x9519, 0x50},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x951b, 0x50},
	/* Added Setting mode */
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3030, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x3032, 0x00},
	{IMX_CAMERA_MODULE_REG_TYPE_DATA, 0x0220, 0x00}
};
/* ======================================================================== */

static struct imx_camera_module_config imx258_configs[] = {
	{
		.name = "3264x1836_30fps",
		.frm_fmt = {
			.width = 3264,
			.height = 1836,
			.code = V4L2_MBUS_FMT_SRGGB10_1X10
		},
		.frm_intrvl = {
			.interval = {
				.numerator = 1,
				.denominator = 30
			}
		},
		.auto_exp_enabled = false,
		.auto_gain_enabled = false,
		.auto_wb_enabled = false,
		.reg_table = (void *)imx258_init_tab_3264_1836_30fps,
		.reg_table_num_entries =
			sizeof(imx258_init_tab_3264_1836_30fps)
			/
			sizeof(imx258_init_tab_3264_1836_30fps[0]),
		.v_blanking_time_us = 5000,
		PLTFRM_CAM_ITF_MIPI_CFG(0, 4, 798, IMX258_EXT_CLK)
	},
	{
		.name = "3264x2448_24fps",
		.frm_fmt = {
			.width = 3264,
			.height = 2448,
			.code = V4L2_MBUS_FMT_SRGGB10_1X10
		},
		.frm_intrvl = {
			.interval = {
				.numerator = 1,
				.denominator = 24
			}
		},
		.auto_exp_enabled = false,
		.auto_gain_enabled = false,
		.auto_wb_enabled = false,
		.reg_table = (void *)imx258_init_tab_3264_2448_24fps,
		.reg_table_num_entries =
			sizeof(imx258_init_tab_3264_2448_24fps)
			/
			sizeof(imx258_init_tab_3264_2448_24fps[0]),
		.v_blanking_time_us = 5000,
		PLTFRM_CAM_ITF_MIPI_CFG(0, 4, 798, IMX258_EXT_CLK)
	}
};

/*--------------------------------------------------------------------------*/

static int imx258_g_VTS(struct imx_camera_module *cam_mod, u32 *vts)
{
	u32 msb, lsb;
	int ret;

	ret = imx_camera_module_read_reg_table(
		cam_mod,
		IMX258_TIMING_VTS_HIGH_REG,
		&msb);
	if (IS_ERR_VALUE(ret))
		goto err;

	ret = imx_camera_module_read_reg_table(
		cam_mod,
		IMX258_TIMING_VTS_LOW_REG,
		&lsb);
	if (IS_ERR_VALUE(ret))
		goto err;

	*vts = (msb << 8) | lsb;

	return 0;
err:
	imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_auto_adjust_fps(struct imx_camera_module *cam_mod,
	u32 exp_time)
{
	int ret;
	u32 vts;

	if ((exp_time + IMX258_COARSE_INTG_TIME_MAX_MARGIN)
		> cam_mod->vts_min)
		vts =exp_time + IMX258_COARSE_INTG_TIME_MAX_MARGIN;
	else
		vts = cam_mod->vts_min;
	ret = imx_camera_module_write_reg(cam_mod, IMX258_TIMING_VTS_LOW_REG, vts & 0xFF);
	ret |= imx_camera_module_write_reg(cam_mod, IMX258_TIMING_VTS_HIGH_REG, (vts >> 8) & 0xFF);

	if (IS_ERR_VALUE(ret)) {
		imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	} else {
		imx_camera_module_pr_debug(cam_mod, "updated vts = %d,vts_min=%d\n", vts, cam_mod->vts_min);
		cam_mod->vts_cur = vts;
	}

	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_write_aec(struct imx_camera_module *cam_mod)
{
	int ret = 0;

	imx_camera_module_pr_debug(cam_mod,
				  "exp_time = %d, gain = %d, flash_mode = %d\n",
				  cam_mod->exp_config.exp_time,
				  cam_mod->exp_config.gain,
				  cam_mod->exp_config.flash_mode);

	/* if the sensor is already streaming, write to shadow registers,
	   if the sensor is in SW standby, write to active registers,
	   if the sensor is off/registers are not writeable, do nothing */
	if ((cam_mod->state == IMX_CAMERA_MODULE_SW_STANDBY) || (cam_mod->state == IMX_CAMERA_MODULE_STREAMING)) {
		u32 a_gain = cam_mod->exp_config.gain;
		u32 exp_time = cam_mod->exp_config.exp_time;
		a_gain = a_gain * cam_mod->exp_config.gain_percent / 100;
		if (a_gain > 480)
			a_gain = 480;

		if (!IS_ERR_VALUE(ret) && cam_mod->auto_adjust_fps)
			ret = imx258_auto_adjust_fps(cam_mod, cam_mod->exp_config.exp_time);

		// Gain
		ret = imx_camera_module_write_reg(cam_mod, IMX258_AEC_PK_GAIN_HIGH_REG, ((a_gain & 0x100) >> 8));
		ret = imx_camera_module_write_reg(cam_mod, IMX258_AEC_PK_GAIN_LOW_REG, (a_gain & 0xff));

		// Integration Time
		ret = imx_camera_module_write_reg(cam_mod, IMX258_AEC_PK_EXPO_HIGH_REG, IMX258_FETCH_HIGH_BYTE_EXP(exp_time));
		ret |= imx_camera_module_write_reg(cam_mod, IMX258_AEC_PK_EXPO_LOW_REG, IMX258_FETCH_LOW_BYTE_EXP(exp_time));

	}

	if (IS_ERR_VALUE(ret))
		imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_g_ctrl(struct imx_camera_module *cam_mod, u32 ctrl_id)
{
	int ret = 0;

	imx_camera_module_pr_debug(cam_mod, "\n");

	switch (ctrl_id) {
	case V4L2_CID_GAIN:
	case V4L2_CID_EXPOSURE:
	case V4L2_CID_FLASH_LED_MODE:
		/* nothing to be done here */
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (IS_ERR_VALUE(ret))
		imx_camera_module_pr_debug(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_filltimings(struct imx_camera_module_custom_config *custom)
{
	int i, j;
	struct imx_camera_module_config *config;
	struct imx_camera_module_timings *timings;
	struct imx_camera_module_reg *reg_table;
	int reg_table_num_entries;

	for (i = 0; i < custom->num_configs; i++) {
		config = &custom->configs[i];
		reg_table = config->reg_table;
		reg_table_num_entries = config->reg_table_num_entries;
		timings = &config->timings;

		for (j = 0; j < reg_table_num_entries; j++) {
			switch (reg_table[j].reg) {
			case IMX258_TIMING_VTS_HIGH_REG:
				timings->frame_length_lines = reg_table[j].val << 8;
				break;
			case IMX258_TIMING_VTS_LOW_REG:
				timings->frame_length_lines |= reg_table[j].val;
				break;
			case IMX258_TIMING_HTS_HIGH_REG:
				timings->line_length_pck = (reg_table[j].val << 8);
				break;
			case IMX258_TIMING_HTS_LOW_REG:
				timings->line_length_pck |= reg_table[j].val;
				break;
			case IMX258_TIMING_X_INC:
				timings->binning_factor_x = ((reg_table[j].val >> 4) + 1) / 2;
				if (timings->binning_factor_x == 0)
					timings->binning_factor_x = 1;
				break;
			case IMX258_TIMING_Y_INC:
				timings->binning_factor_y = ((reg_table[j].val >> 4) + 1) / 2;
				if (timings->binning_factor_y == 0)
					timings->binning_factor_y = 1;
				break;
			case IMX258_HORIZONTAL_START_HIGH_REG:
				timings->crop_horizontal_start = reg_table[j].val << 8;
				break;
			case IMX258_HORIZONTAL_START_LOW_REG:
				timings->crop_horizontal_start |= reg_table[j].val;
				break;
			case IMX258_VERTICAL_START_HIGH_REG:
				timings->crop_vertical_start = reg_table[j].val << 8;
				break;
			case IMX258_VERTICAL_START_LOW_REG:
				timings->crop_vertical_start |= reg_table[j].val;
				break;
			case IMX258_HORIZONTAL_END_HIGH_REG:
				timings->crop_horizontal_end = reg_table[j].val << 8;
				break;
			case IMX258_HORIZONTAL_END_LOW_REG:
				timings->crop_horizontal_end |= reg_table[j].val;
				break;
			case IMX258_VERTICAL_END_HIGH_REG:
				timings->crop_vertical_end = reg_table[j].val << 8;
				break;
			case IMX258_VERTICAL_END_LOW_REG:
				timings->crop_vertical_end |= reg_table[j].val;
				break;
			case IMX258_HORIZONTAL_OUTPUT_SIZE_HIGH_REG:
				timings->sensor_output_width = reg_table[j].val << 8;
				break;
			case IMX258_HORIZONTAL_OUTPUT_SIZE_LOW_REG:
				timings->sensor_output_width |= reg_table[j].val;
				break;
			case IMX258_VERTICAL_OUTPUT_SIZE_HIGH_REG:
				timings->sensor_output_height = reg_table[j].val << 8;
				break;
			case IMX258_VERTICAL_OUTPUT_SIZE_LOW_REG:
				timings->sensor_output_height |= reg_table[j].val;
				break;
			}
		}

		timings->vt_pix_clk_freq_hz = config->frm_intrvl.interval.denominator
					* timings->frame_length_lines
					* timings->line_length_pck;

		timings->coarse_integration_time_min = IMX258_COARSE_INTG_TIME_MIN;
		timings->coarse_integration_time_max_margin = IMX258_COARSE_INTG_TIME_MAX_MARGIN;

		/* IMX Sensor do not use fine integration time. */
		timings->fine_integration_time_min = IMX258_FINE_INTG_TIME_MIN;
		timings->fine_integration_time_max_margin = IMX258_FINE_INTG_TIME_MAX_MARGIN;
	}

	return 0;
}
static int imx258_g_timings(struct imx_camera_module *cam_mod,
	struct imx_camera_module_timings *timings)
{
	int ret = 0;
	unsigned int vts;

	if (IS_ERR_OR_NULL(cam_mod->active_config))
		goto err;

	*timings = cam_mod->active_config->timings;

	vts = (!cam_mod->vts_cur) ?
		timings->frame_length_lines :
		cam_mod->vts_cur;

	if (cam_mod->frm_intrvl_valid)
		timings->vt_pix_clk_freq_hz =
			cam_mod->frm_intrvl.interval.denominator
			* vts
			* timings->line_length_pck;
	else
		timings->vt_pix_clk_freq_hz =
			cam_mod->active_config->frm_intrvl.interval.denominator
			* vts
			* timings->line_length_pck;

	return ret;
err:
	imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_set_flip(
	struct imx_camera_module *cam_mod,
	struct pltfrm_camera_module_reg reglist[],
	int len)
{
	int i, mode = 0;
	u16 orientation = 0;

	mode = imx_camera_module_get_flip_mirror(cam_mod);
	if (mode == -1) {
		imx_camera_module_pr_info(cam_mod, "dts don't set flip, return!\n");
		return 0;
	}

	if (!IS_ERR_OR_NULL(cam_mod->active_config)) {
		if (mode == IMX_MIRROR_BIT_MASK)
			orientation = IMX258_ORIENTATION_H;
		else if (mode == IMX_FLIP_BIT_MASK)
			orientation = IMX258_ORIENTATION_V;
		else if (mode == (IMX_FLIP_BIT_MASK | IMX_FLIP_BIT_MASK))
			orientation = IMX258_ORIENTATION_H + IMX258_ORIENTATION_V;
		for (i = 0; i < len; i++) {
			if (reglist[i].reg == IMX258_ORIENTATION_REG)
				reglist[i].val = orientation;
		}
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int imx258_s_ctrl(struct imx_camera_module *cam_mod, u32 ctrl_id)
{
	int ret = 0;

	imx_camera_module_pr_debug(cam_mod, "\n");

	switch (ctrl_id) {
	case V4L2_CID_GAIN:
	case V4L2_CID_EXPOSURE:
		ret = imx258_write_aec(cam_mod);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (IS_ERR_VALUE(ret))
		imx_camera_module_pr_debug(cam_mod, "failed with error (%d) 0x%x\n", ret, ctrl_id);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_s_ext_ctrls(struct imx_camera_module *cam_mod,
	struct imx_camera_module_ext_ctrls *ctrls)
{
	int ret = 0;

	/* Handles only exposure and gain together special case. */
	if ((ctrls->ctrls[0].id == V4L2_CID_GAIN ||
		ctrls->ctrls[0].id == V4L2_CID_EXPOSURE))
		ret = imx258_write_aec(cam_mod);
	else
		ret = -EINVAL;

	if (IS_ERR_VALUE(ret))
		imx_camera_module_pr_debug(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_start_streaming(struct imx_camera_module *cam_mod)
{
	int ret = 0;

	imx_camera_module_pr_info(cam_mod, "active config=%s\n", cam_mod->active_config->name);

	ret = imx258_g_VTS(cam_mod, &cam_mod->vts_min);
	if (IS_ERR_VALUE(ret))
		goto err;

	if (IS_ERR_VALUE(imx_camera_module_write_reg(cam_mod, 0x0100, 1)))
		goto err;

	msleep(25);

	return 0;
err:
	imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n",
		ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int imx258_stop_streaming(struct imx_camera_module *cam_mod)
{
	int ret = 0;

	imx_camera_module_pr_info(cam_mod, "\n");

	ret = imx_camera_module_write_reg(cam_mod, 0x0100, 0);
	if (IS_ERR_VALUE(ret))
		goto err;

	msleep(25);

	return 0;
err:
	imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}
/*--------------------------------------------------------------------------*/
static int imx258_check_camera_id(struct imx_camera_module *cam_mod)
{
	u32 pidh, pidl;
	int ret = 0;

	imx_camera_module_pr_debug(cam_mod, "\n");

	ret |= imx_camera_module_read_reg(cam_mod, 1, IMX258_PIDH_ADDR, &pidh);
	ret |= imx_camera_module_read_reg(cam_mod, 1, IMX258_PIDL_ADDR, &pidl);
	if (IS_ERR_VALUE(ret)) {
		imx_camera_module_pr_err(cam_mod, "register read failed, camera module powered off?\n");
		goto err;
	}

	if ((pidh == IMX258_PIDH_MAGIC) && (pidl == IMX258_PIDL_MAGIC))
		imx_camera_module_pr_err(cam_mod, "successfully detected camera ID 0x%02x%02x\n", pidh, pidl);
	else {
		imx_camera_module_pr_err(cam_mod, "wrong camera ID, expected 0x%02x%02x, detected 0x%02x%02x\n",
			IMX258_PIDH_MAGIC, IMX258_PIDL_MAGIC, pidh, pidl);
		ret = -EINVAL;
		goto err;
	}

	return 0;
err:
	imx_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/* ======================================================================== */
/* This part is platform dependent */
/* ======================================================================== */

static struct v4l2_subdev_core_ops imx258_camera_module_core_ops = {
	.g_ctrl = imx_camera_module_g_ctrl,
	.s_ctrl = imx_camera_module_s_ctrl,
	.s_ext_ctrls = imx_camera_module_s_ext_ctrls,
	.s_power = imx_camera_module_s_power,
	.ioctl = imx_camera_module_ioctl
};

static struct v4l2_subdev_video_ops imx258_camera_module_video_ops = {
	.enum_frameintervals = imx_camera_module_enum_frameintervals,
	.s_mbus_fmt = imx_camera_module_s_fmt,
	.g_mbus_fmt = imx_camera_module_g_fmt,
	.try_mbus_fmt = imx_camera_module_try_fmt,
	.s_frame_interval = imx_camera_module_s_frame_interval,
	.g_frame_interval = imx_camera_module_g_frame_interval,
	.s_stream = imx_camera_module_s_stream
};

static struct v4l2_subdev_ops imx258_camera_module_ops = {
	.core = &imx258_camera_module_core_ops,
	.video = &imx258_camera_module_video_ops
};

static struct imx_camera_module_custom_config imx258_custom_config = {
	.start_streaming = imx258_start_streaming,
	.stop_streaming = imx258_stop_streaming,
	.s_ctrl = imx258_s_ctrl,
	.s_ext_ctrls = imx258_s_ext_ctrls,
	.g_ctrl = imx258_g_ctrl,
	.g_timings = imx258_g_timings,
	.check_camera_id = imx258_check_camera_id,
	.set_flip = imx258_set_flip,
	.s_vts = imx258_auto_adjust_fps,
	.configs = imx258_configs,
	.num_configs = sizeof(imx258_configs) / sizeof(imx258_configs[0]),
	.power_up_delays_ms = {5, 20, 0},
	/*
	*0: Exposure time valid fileds;
	*1: Exposure gain valid fileds;
	*(2 fileds == 1 frames)
	*/
	.exposure_valid_frame = {4, 4}
};

static int imx258_probe(
	struct i2c_client *client,
	const struct i2c_device_id *id)
{
	dev_info(&client->dev, "probing...\n");

	imx258_filltimings(&imx258_custom_config);
	v4l2_i2c_subdev_init(&imx258.sd, client, &imx258_camera_module_ops);
	imx258.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	imx258.custom = imx258_custom_config;

	dev_info(&client->dev, "probing successful\n");
	return 0;
}

static int imx258_remove(struct i2c_client *client)
{
	struct imx_camera_module *cam_mod = i2c_get_clientdata(client);

	dev_info(&client->dev, "removing device...\n");

	if (!client->adapter)
		return -ENODEV;	/* our client isn't attached */

	imx_camera_module_release(cam_mod);

	dev_info(&client->dev, "removed\n");
	return 0;
}

static const struct i2c_device_id imx258_id[] = {
	{ IMX258_DRIVER_NAME, 0 },
	{ }
};

static struct of_device_id imx258_of_match[] = {
	{.compatible = "sony,imx258-v4l2-i2c-subdev"},
	{},
};

MODULE_DEVICE_TABLE(i2c, imx258_id);

static struct i2c_driver imx258_i2c_driver = {
	.driver = {
		.name = IMX258_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = imx258_of_match
	},
	.probe = imx258_probe,
	.remove = imx258_remove,
	.id_table = imx258_id,
};

module_i2c_driver(imx258_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for IMX258");
MODULE_AUTHOR("George");
MODULE_LICENSE("GPL");
