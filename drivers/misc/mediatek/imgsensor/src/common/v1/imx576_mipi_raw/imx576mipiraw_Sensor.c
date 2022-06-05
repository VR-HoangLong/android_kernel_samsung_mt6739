/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 IMX576mipi_Sensor.c
 *
 * Project:
 * --------
 *	 ALPS
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
/* #include <asm/system.h> */
/* #include <linux/xlog.h> */
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "imx576mipiraw_Sensor.h"

#undef VENDOR_EDIT

#define IMX576_VER_90CUT 0x01
#define IMX576_VER_91CUT 0x02
static int32_t gSensorVersion;

#define PFX "IMX576_camera_sensor"
#define LSC_DEBUG 0

static kal_uint32 streaming_control(kal_bool enable);

#define LOG_INF(format, args...) pr_debug(PFX "[%s] " format, __func__, ##args)
static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = IMX576_SENSOR_ID,

	.module_id = 0x01,  /* 0x01 Sunny,0x05 QTEK */

	.checksum_value = 0x4cb91a94,

	.pre = {
		/*setting for normal binning*/
		.pclk = 419900000,
		.linelength = 6320,
		.framelength = 2214,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2832,
		.grabwindow_height = 2124,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.pre_3HDR = {
		/*for 3hdr setting*/
		.pclk = 830400000,//820800000,
		.linelength = 6144,
		.framelength = 4505,//4453,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2880,
		.grabwindow_height = 2156,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 417200000,//444400000,
		.max_framerate = 300,
	},
	.cap = {
		/*setting for normal binning*/
		.pclk = 419900000,
		.linelength = 6320,
		.framelength = 2214,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2832,
		.grabwindow_height = 2124,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.normal_video = {
		/*setting for normal binning*/
		.pclk = 419900000,
		.linelength = 6320,
		.framelength = 2214,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2832,
		.grabwindow_height = 1592,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.hs_video = {
		/*setting for normal binning*/
		.pclk = 798200000,
		.linelength = 3160,
		.framelength = 2104,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2832,
		.grabwindow_height = 1592,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 1200,
	},
	.slim_video = {
		/*setting for remosaic*/
		.pclk = 808600000,
		.linelength = 6144,
		.framelength = 4386,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 5664,
		.grabwindow_height = 4248,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.custom1 = {
		/*setting for normal binning*/
		.pclk = 798200000,
		.linelength = 3160,
		.framelength = 2215,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2616,
		.grabwindow_height = 1960,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 1140,
	},
	.custom2 = {
		/*setting for normal binning*/
		.pclk = 419900000,
		.linelength = 6320,
		.framelength = 2214,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2336,
		.grabwindow_height = 1752,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.custom3 = {
		/*setting for normal binning*/
		.pclk = 419900000,
		.linelength = 6320,
		.framelength = 2214,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2336,
		.grabwindow_height = 1752,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.custom4 = {
		/*setting for normal binning*/
		.pclk = 419900000,
		.linelength = 6320,
		.framelength = 2214,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2336,
		.grabwindow_height = 1316,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.custom5 = {
		/*setting for fov68 remosaic*/
		.pclk = 808600000,
		.linelength = 6144,
		.framelength = 4386,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4672,
		.grabwindow_height = 3512,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 821600000,
		.max_framerate = 300,
	},
	.margin = 61,
	.min_shutter = 6,
	.min_gain = 64, /*1x gain*/
	.max_gain = 1024, /*16x gain*/
	.min_gain_iso = 100,
	.gain_step = 16,
	.gain_type = 0,

	.max_frame_length = 0xffff,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.ihdr_support = 0,	  /* 1, support; 0,not support */
	.ihdr_le_firstline = 0,  /* 1,le first ; 0, se first */
	.temperature_support = 0,/* 1, support; 0,not support */
	.sensor_mode_num = 10,	  /* support sensor mode num */

	.cap_delay_frame = 2,
	.pre_delay_frame = 2,
	.video_delay_frame = 2,
	.hs_video_delay_frame = 2,
	.slim_video_delay_frame = 2,
	.custom1_delay_frame = 2,
	.custom2_delay_frame = 2,
	.custom3_delay_frame = 2,
	.custom4_delay_frame = 2,
	.custom5_delay_frame = 2,
	.isp_driving_current = ISP_DRIVING_6MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2,
	/* 0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2 */
	.mipi_settle_delay_mode = 0,
	/* 0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL */
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_R,
	.mclk = 26,
	.mipi_lane_num = SENSOR_MIPI_4_LANE,
	.i2c_addr_table = {0x34, 0x20, 0xff},
	.i2c_speed = 320, /* i2c read/write speed */
};


static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL, /* mirrorflip information */
	/* IMGSENSOR_MODE enum value,record current sensor mode,such as:
	 *  INIT, Preview, Capture, Video,High Speed Video, Slim Video
	 */
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x3D0, /* current shutter */
	.gain = 0x100, /* current gain */
	.dummy_pixel = 0, /* current dummypixel */
	.dummy_line = 0, /* current dummyline */
	.current_fps = 0,
	.autoflicker_en = KAL_FALSE,
	.test_pattern = KAL_FALSE,
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	.ihdr_mode = 0, /* sensor need support LE, SE with HDR feature */
	.hdr_mode = 0, /* HDR mODE : 0: disable HDR, 1:IHDR, 2:HDR, 9:ZHDR */
	.i2c_write_id = 0x20,
	.AE_binning_type = BINNING_NONE,
};


/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[10] = {
	{ 5760, 4312,    0,   32, 5760, 4248, 2880, 2124,
	    24,    0, 2832, 2124,    0,    0, 2832, 2124 }, /* preview */
	{ 5760, 4312,    0,   32, 5760, 4248, 2880, 2124,
	    24,    0, 2832, 2124,    0,    0, 2832, 2124 }, /* capture (pre.) */
	{ 5760, 4312,    0,   32, 5760, 4248, 2880, 2124,
	    24,  266, 2832, 1592,    0,    0, 2832, 1592 }, /* video */
	{ 5760, 4312,    0,  564, 5760, 3184, 2880, 1592,
	    24,    0, 2832, 1592,    0,    0, 2832, 1592 }, /* hs video */
	{ 5760, 4312,    0,   32, 5760, 4248, 5760, 4248,
	    48,    0, 5664, 4248,    0,    0, 5664, 4248 }, /* slim (remo.) */
	{ 5760, 4312,    0,  196, 5760, 3920, 2880, 1960,
	   132,    0, 2616, 1960,    0,    0, 2616, 1960 }, /* custom1 */
	{ 5760, 4312,    0,   32, 5760, 4248, 2880, 2124,
	   272,  186, 2336, 1752,    0,    0, 2336, 1752 }, /* custom2 */
	{ 5760, 4312,    0,   32, 5760, 4248, 2880, 2124,
	   272,  186, 2336, 1752,    0,    0, 2336, 1752 }, /* custom3 */
	{ 5760, 4312,    0,   32, 5760, 4248, 2880, 2124,
	   272,  404, 2336, 1316,    0,    0, 2336, 1316 }, /* custom4 */
	{ 5760, 4312,    0,   32, 5760, 4248, 5760, 4248,
	   544,  368, 4672, 3512,    0,    0, 4672, 3512 }  /* custom5 */
};

static struct SENSOR_VC_INFO_STRUCT SENSOR_VC_INFO[3] = {
	/* Preview mode setting */
	{0x05, 0x0a, 0x00, 0x08, 0x40, 0x00,
	/*VC0:raw, VC1:Embedded data*/
	 0x00, 0x2b, 0x0B40, 0x086C, 0x00, 0x12, 0x0E10, 0x0002,
	/*VC2:Y HIST(3HDR), VC3:AE HIST(3HDR)*/
	 0x00, 0x31, 0x0E10, 0x0001, 0x00, 0x32, 0x0E10, 0x0001,
	/*VC4:Flicker(3HDR), VC5:no data*/
	 0x00, 0x33, 0x0E10, 0x0001, 0x00, 0x00, 0x0000, 0x0000},
	/* Capture mode setting */
	{0x03, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 0x00, 0x2b, 0x1680, 0x10D8, 0x00, 0x00, 0x0000, 0x0000,
	 0x00, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x0000, 0x0000},
	/* Video mode setting */
	{0x02, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 /*VC0:raw, VC1:Embedded data*/
	 0x00, 0x2b, 0x0B40, 0x086C, 0x00, 0x12, 0x0E10, 0x0002,
	/*VC2:Y HIST(3HDR), VC3:AE HIST(3HDR)*/
	 0x00, 0x31, 0x0E10, 0x0001, 0x00, 0x32, 0x0E10, 0x0001,
	/*VC4:Flicker(3HDR), VC5:no data*/
	 0x00, 0x33, 0x0E10, 0x0001, 0x00, 0x00, 0x0000, 0x0000}
};


/*add for sensor otp id*/
#define MODULE_ID_OFFSET 0x0000
#define SENSOR_OTP_ID_OFFSET 0x0006
#define EEPROM_READ_ID  0xA2
#define IMX576_OTP_ID        0x53

static kal_uint16 read_otp_sensor_id(void)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(SENSOR_OTP_ID_OFFSET >> 8),
				(char)(SENSOR_OTP_ID_OFFSET & 0xFF) };

	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, EEPROM_READ_ID);
	LOG_INF("read from otp, sensor id is 0x%x\n", get_byte);
	return get_byte;
}

/*modify for different module*/
static kal_uint16 read_module_id(void)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(MODULE_ID_OFFSET >> 8),
			     (char)(MODULE_ID_OFFSET & 0xFF) };

	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, 0xA2/*EEPROM_READ_ID*/);
	if (get_byte == 0) /*EEPROM_READ_ID*/
		iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, 0xA8);

	return get_byte;
}

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 2, imgsensor.i2c_write_id);
	return ((get_byte<<8)&0xff00)|((get_byte>>8)&0x00ff);
}


static void write_cmos_sensor(kal_uint16 addr, kal_uint16 para)
{
	char pusendcmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF),
			     (char)(para >> 8), (char)(para & 0xFF)};

	/*kdSetI2CSpeed(imgsensor_info.i2c_speed);*/
	/* Add this func to set i2c speed by each sensor */
	iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);
}

static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	 /* Add this func to set i2c speed by each sensor */
	/*kdSetI2CSpeed(imgsensor_info.i2c_speed);*/
	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);
	return get_byte;
}

static int write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
	char pusendcmd[4] = {(char)(addr >> 8),
			     (char)(addr & 0xFF),
			     (char)(para & 0xFF)};

	return iWriteRegI2C(pusendcmd, 3, imgsensor.i2c_write_id);
}

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d\n",
		imgsensor.dummy_line, imgsensor.dummy_pixel);
	/* return; //for test */
	write_cmos_sensor(0x0340, imgsensor.frame_length);
	write_cmos_sensor(0x0342, imgsensor.line_length);
}	/*	set_dummy  */


static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{

	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INF("framerate = %d, min framelength should enable %d\n",
		framerate, min_framelength_en);

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	if (frame_length >= imgsensor.min_frame_length)
		imgsensor.frame_length = frame_length;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	imgsensor.dummy_line =
		imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line =
			imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */

static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;


	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);

	if (shutter < imgsensor_info.min_shutter)
		shutter = imgsensor_info.min_shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps =
	  imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
			/* Extend frame length */
			write_cmos_sensor_8(0x0104, 0x01);
			write_cmos_sensor_8(0x0340,
					    imgsensor.frame_length >> 8);
			write_cmos_sensor_8(0x0341,
					    imgsensor.frame_length & 0xFF);
			write_cmos_sensor_8(0x0104, 0x00);
		}
	} else {
		/* Extend frame length */
		write_cmos_sensor_8(0x0104, 0x01);
		write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
		write_cmos_sensor_8(0x0104, 0x00);
	}

	/* Update Shutter */
	write_cmos_sensor_8(0x0104, 0x01);
	write_cmos_sensor_8(0x0202, (shutter >> 8) & 0xFF);
	write_cmos_sensor_8(0x0203, shutter  & 0xFF);
	write_cmos_sensor_8(0x0104, 0x00);
	LOG_INF("Exit! shutter =%d, framelength =%d\n",
		shutter, imgsensor.frame_length);

}	/*	write_shutter  */



/*************************************************************************
 * FUNCTION
 *	set_shutter
 *
 * DESCRIPTION
 *	This function set e-shutter of sensor to change exposure time.
 *
 * PARAMETERS
 *	iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	write_shutter(shutter);

}	/*	set_shutter */

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain;

	reg_gain = 1024 - (1024*64)/gain;

	LOG_INF("imx576 gain =%d, reg_gain =%d\n", gain, reg_gain);

	return reg_gain;
}

/*write AWB gain to sensor*/
static void feedback_awbgain(kal_uint32 r_gain, kal_uint32 b_gain)
{
	UINT32 r_gain_int = 0;
	UINT32 b_gain_int = 0;

	r_gain_int = r_gain / 512;
	b_gain_int = b_gain / 512;

	/*write r_gain*/
	write_cmos_sensor_8(0x0B90, r_gain_int);
	write_cmos_sensor_8(0x0B91,
		(((r_gain*100) / 512) - (r_gain_int * 100)) * 2);

	/*write _gain*/
	write_cmos_sensor_8(0x0B92, b_gain_int);
	write_cmos_sensor_8(0x0B93,
		(((b_gain * 100) / 512) - (b_gain_int * 100)) * 2);
}

/*************************************************************************
 * FUNCTION
 *	set_gain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *	iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;

	/* gain=1024;//for test */
	/* return; //for test */

	if (gain < imgsensor_info.min_gain || gain > imgsensor_info.max_gain) {
		LOG_INF("Error gain setting");

		if (gain < imgsensor_info.min_gain)
			gain = imgsensor_info.min_gain;
		else
			gain = imgsensor_info.max_gain;
	}


	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d, reg_gain = 0x%x\n ", gain, reg_gain);

	write_cmos_sensor_8(0x0104, 0x01);
	write_cmos_sensor_8(0x0204, (reg_gain>>8) & 0xFF);
	write_cmos_sensor_8(0x0205, reg_gain & 0xFF);
	write_cmos_sensor_8(0x0104, 0x00);

	return gain;
}	/*	set_gain  */

static void set_mirror_flip(kal_uint8 image_mirror)
{
	kal_uint8 itemp;

	LOG_INF("image_mirror = %d\n", image_mirror);
	itemp = read_cmos_sensor_8(0x0101);
	itemp &= ~0x03;

	switch (image_mirror) {

	case IMAGE_NORMAL:
	write_cmos_sensor_8(0x0101, itemp);
	break;

	case IMAGE_V_MIRROR:
	write_cmos_sensor_8(0x0101, itemp | 0x02);
	break;

	case IMAGE_H_MIRROR:
	write_cmos_sensor_8(0x0101, itemp | 0x01);
	break;

	case IMAGE_HV_MIRROR:
	write_cmos_sensor_8(0x0101, itemp | 0x03);
	break;
	}
}

/*************************************************************************
 * FUNCTION
 *	night_mode
 *
 * DESCRIPTION
 *	This function night mode of sensor.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
#if 0
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/
}	/*	night_mode	*/
#endif


#define MULTI_WRITE 1

#if MULTI_WRITE
#define I2C_BUFFER_LEN 765 /* trans# max is 255, each 3 bytes */
#else
#define I2C_BUFFER_LEN 3
#endif
static kal_uint16 imx576_table_write_cmos_sensor(kal_uint16 *para,
						 kal_uint32 len)
{
	char puSendCmd[I2C_BUFFER_LEN];
	kal_uint32 tosend, IDX;
	kal_uint16 addr = 0, addr_last = 0, data;

	tosend = 0;
	IDX = 0;

	while (len > IDX) {
		addr = para[IDX];

		{
			puSendCmd[tosend++] = (char)(addr >> 8);
			puSendCmd[tosend++] = (char)(addr & 0xFF);
			data = para[IDX + 1];
			puSendCmd[tosend++] = (char)(data & 0xFF);
			IDX += 2;
			addr_last = addr;

		}
#if MULTI_WRITE
		/* Write when remain buffer size is less than 3 bytes
		 * or reach end of data
		 */
		if ((I2C_BUFFER_LEN - tosend) < 3
			|| IDX == len || addr != addr_last) {
			iBurstWriteReg_multi(puSendCmd,
						tosend,
						imgsensor.i2c_write_id,
						3,
						imgsensor_info.i2c_speed);
			tosend = 0;
		}
#else
		iWriteRegI2C(puSendCmd, 3, imgsensor.i2c_write_id);
		tosend = 0;

#endif
	}
	return 0;
}

static kal_uint16 imx576_init_setting[] = {
	/* External Clock Setting */
	0x0136, 0x1A,
	0x0137, 0x00,

	/* Register version */
	0x3C7E, 0x05,
	0x3C7F, 0x09,

	/* Global Setting */
	0x380C, 0x00,
	0x3C00, 0x10,
	0x3C01, 0x10,
	0x3C02, 0x10,
	0x3C03, 0x10,
	0x3C04, 0x10,
	0x3C05, 0x01,
	0x3C08, 0xFF,
	0x3C09, 0xFF,
	0x3C0A, 0x01,
	0x3C0D, 0xFF,
	0x3C0E, 0xFF,
	0x3C0F, 0x20,
	0x3F89, 0x01,
	0x4B8E, 0x18,
	0x4B8F, 0x10,
	0x4BA8, 0x08,
	0x4BAA, 0x08,
	0x4BAB, 0x08,
	0x4BC9, 0x10,
	0x5511, 0x01,
	0x560B, 0x5B,
	0x56A7, 0x60,
	0x5B3B, 0x60,
	0x5BA7, 0x60,
	0x6002, 0x00,
	0x6014, 0x01,
	0x6118, 0x0A,
	0x6122, 0x0A,
	0x6128, 0x0A,
	0x6132, 0x0A,
	0x6138, 0x0A,
	0x6142, 0x0A,
	0x6148, 0x0A,
	0x6152, 0x0A,
	0x617B, 0x04,
	0x617E, 0x04,
	0x6187, 0x04,
	0x618A, 0x04,
	0x6193, 0x04,
	0x6196, 0x04,
	0x619F, 0x04,
	0x61A2, 0x04,
	0x61AB, 0x04,
	0x61AE, 0x04,
	0x61B7, 0x04,
	0x61BA, 0x04,
	0x61C3, 0x04,
	0x61C6, 0x04,
	0x61CF, 0x04,
	0x61D2, 0x04,
	0x61DB, 0x04,
	0x61DE, 0x04,
	0x61E7, 0x04,
	0x61EA, 0x04,
	0x61F3, 0x04,
	0x61F6, 0x04,
	0x61FF, 0x04,
	0x6202, 0x04,
	0x620B, 0x04,
	0x620E, 0x04,
	0x6217, 0x04,
	0x621A, 0x04,
	0x6223, 0x04,
	0x6226, 0x04,
	0x6B0B, 0x02,
	0x6B0C, 0x01,
	0x6B0D, 0x05,
	0x6B0F, 0x04,
	0x6B10, 0x02,
	0x6B11, 0x06,
	0x6B12, 0x03,
	0x6B13, 0x07,
	0x6B14, 0x0D,
	0x6B15, 0x09,
	0x6B16, 0x0C,
	0x6B17, 0x08,
	0x6B18, 0x0E,
	0x6B19, 0x0A,
	0x6B1A, 0x0F,
	0x6B1B, 0x0B,
	0x6B1C, 0x01,
	0x6B1D, 0x05,
	0x6B1F, 0x04,
	0x6B20, 0x02,
	0x6B21, 0x06,
	0x6B22, 0x03,
	0x6B23, 0x07,
	0x6B24, 0x0D,
	0x6B25, 0x09,
	0x6B26, 0x0C,
	0x6B27, 0x08,
	0x6B28, 0x0E,
	0x6B29, 0x0A,
	0x6B2A, 0x0F,
	0x6B2B, 0x0B,
	0x7948, 0x01,
	0x7949, 0x06,
	0x794B, 0x04,
	0x794C, 0x04,
	0x794D, 0x3A,
	0x7951, 0x00,
	0x7952, 0x01,
	0x7955, 0x00,
	0x9004, 0x10,
	0x9200, 0xA0,
	0x9201, 0xA7,
	0x9202, 0xA0,
	0x9203, 0xAA,
	0x9204, 0xA0,
	0x9205, 0xAD,
	0x9206, 0xA0,
	0x9207, 0xB0,
	0x9208, 0xA0,
	0x9209, 0xB3,
	0x920A, 0xB7,
	0x920B, 0x34,
	0x920C, 0xB7,
	0x920D, 0x36,
	0x920E, 0xB7,
	0x920F, 0x37,
	0x9210, 0xB7,
	0x9211, 0x38,
	0x9212, 0xB7,
	0x9213, 0x39,
	0x9214, 0xB7,
	0x9215, 0x3A,
	0x9216, 0xB7,
	0x9217, 0x3C,
	0x9218, 0xB7,
	0x9219, 0x3D,
	0x921A, 0xB7,
	0x921B, 0x3E,
	0x921C, 0xB7,
	0x921D, 0x3F,
	0x921E, 0x7F,
	0x921F, 0x77,
	0x99AF, 0x0F,
	0x99B0, 0x0F,
	0x99B1, 0x0F,
	0x99B2, 0x0F,
	0x99B3, 0x0F,
	0x99E1, 0x0F,
	0x99E2, 0x0F,
	0x99E3, 0x0F,
	0x99E4, 0x0F,
	0x99E5, 0x0F,
	0x99E6, 0x0F,
	0x99E7, 0x0F,
	0x99E8, 0x0F,
	0x99E9, 0x0F,
	0x99EA, 0x0F,
	0xE286, 0x31,
	0xE2A6, 0x32,
	0xE2C6, 0x33,

	/* Image Quality adjustment setting */
	0x4038, 0x00,
	0x9856, 0xA0,
	0x9857, 0x78,
	0x9858, 0x64,
	0x986E, 0x64,
	0x9870, 0x3C,
	0x993A, 0x0E,
	0x993B, 0x0E,
	0x9953, 0x08,
	0x9954, 0x08,
	0x996B, 0x0F,
	0x996D, 0x0F,
	0x996F, 0x0F,
	0x998E, 0x0F,
	0xA101, 0x01,
	0xA103, 0x01,
	0xA105, 0x01,
	0xA107, 0x01,
	0xA109, 0x01,
	0xA10B, 0x01,
	0xA10D, 0x01,
	0xA10F, 0x01,
	0xA111, 0x01,
	0xA113, 0x01,
	0xA115, 0x01,
	0xA117, 0x01,
	0xA119, 0x01,
	0xA11B, 0x01,
	0xA11D, 0x01,
	0xAA58, 0x00,
	0xAA59, 0x01,
	0xAB03, 0x10,
	0xAB04, 0x10,
	0xAB05, 0x10,
	0xAD6A, 0x03,
	0xAD6B, 0xFF,
	0xAD77, 0x00,
	0xAD82, 0x03,
	0xAD83, 0xFF,
	0xAE06, 0x04,
	0xAE07, 0x16,
	0xAE08, 0xFF,
	0xAE09, 0x04,
	0xAE0A, 0x16,
	0xAE0B, 0xFF,
	0xAF01, 0x04,
	0xAF03, 0x0A,
	0xAF05, 0x18,
	0xB048, 0x0A,
};

static kal_uint16 imx576_preview_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0xB0,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA6,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x00,
	0x0409, 0x18,
	0x040A, 0x00,
	0x040B, 0x00,
	0x040C, 0x0B,
	0x040D, 0x10,
	0x040E, 0x08,
	0x040F, 0x4C,

	/* Output Size Setting */
	0x034C, 0x0B,
	0x034D, 0x10,
	0x034E, 0x08,
	0x034F, 0x4C,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x04,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x43,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x02,
	0x3F81, 0x30,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_capture_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0xB0,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA6,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x00,
	0x0409, 0x18,
	0x040A, 0x00,
	0x040B, 0x00,
	0x040C, 0x0B,
	0x040D, 0x10,
	0x040E, 0x08,
	0x040F, 0x4C,

	/* Output Size Setting */
	0x034C, 0x0B,
	0x034D, 0x10,
	0x034E, 0x08,
	0x034F, 0x4C,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x04,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x43,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x02,
	0x3F81, 0x30,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_normal_video_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0xB0,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA6,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x00,
	0x0409, 0x18,
	0x040A, 0x01,
	0x040B, 0x0A,
	0x040C, 0x0B,
	0x040D, 0x10,
	0x040E, 0x06,
	0x040F, 0x38,

	/* Output Size Setting */
	0x034C, 0x0B,
	0x034D, 0x10,
	0x034E, 0x06,
	0x034F, 0x38,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x04,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x43,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x02,
	0x3F81, 0x30,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_hs_video_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x0C,
	0x0343, 0x58,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0x38,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x02,
	0x0347, 0x34,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x0E,
	0x034B, 0xA3,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x00,
	0x0409, 0x18,
	0x040A, 0x00,
	0x040B, 0x00,
	0x040C, 0x0B,
	0x040D, 0x10,
	0x040E, 0x06,
	0x040F, 0x38,

	/* Output Size Setting */
	0x034C, 0x0B,
	0x034D, 0x10,
	0x034E, 0x06,
	0x034F, 0x38,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x02,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x33,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x03,
	0x3F81, 0xE8,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_slim_video_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0x00,

	/* Frame Length Lines Setting */
	0x0340, 0x11,
	0x0341, 0x22,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x00,
	0x0901, 0x11,
	0x0902, 0x0A,
	0x3140, 0x00,
	0x3246, 0x01,
	0x3247, 0x01,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x00,
	0x0409, 0x30,
	0x040A, 0x00,
	0x040B, 0x00,
	0x040C, 0x16,
	0x040D, 0x20,
	0x040E, 0x10,
	0x040F, 0x98,

	/* Output Size Setting */
	0x034C, 0x16,
	0x034D, 0x20,
	0x034E, 0x10,
	0x034F, 0x98,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x02,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x37,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x01,
	0x3620, 0x01,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x01,
	0x3F81, 0x72,
	0x3FFC, 0x00,
	0x3FFD, 0x3C,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_custom1_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x0C,
	0x0343, 0x58,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA7,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0xC4,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0x13,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x00,
	0x0409, 0x84,
	0x040A, 0x00,
	0x040B, 0x00,
	0x040C, 0x0A,
	0x040D, 0x38,
	0x040E, 0x07,
	0x040F, 0xA8,

	/* Output Size Setting */
	0x034C, 0x0A,
	0x034D, 0x38,
	0x034E, 0x07,
	0x034F, 0xA8,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x02,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x33,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x03,
	0x3F81, 0xE8,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_custom2_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0xB0,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA6,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x01,
	0x0409, 0x10,
	0x040A, 0x00,
	0x040B, 0xBA,
	0x040C, 0x09,
	0x040D, 0x20,
	0x040E, 0x06,
	0x040F, 0xD8,

	/* Output Size Setting */
	0x034C, 0x09,
	0x034D, 0x20,
	0x034E, 0x06,
	0x034F, 0xD8,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x04,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x43,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x02,
	0x3F81, 0x30,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_custom3_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0xB0,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA6,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x01,
	0x0409, 0x10,
	0x040A, 0x00,
	0x040B, 0xBA,
	0x040C, 0x09,
	0x040D, 0x20,
	0x040E, 0x06,
	0x040F, 0xD8,

	/* Output Size Setting */
	0x034C, 0x09,
	0x034D, 0x20,
	0x034E, 0x06,
	0x034F, 0xD8,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x04,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x43,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x02,
	0x3F81, 0x30,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_custom4_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0xB0,

	/* Frame Length Lines Setting */
	0x0340, 0x08,
	0x0341, 0xA6,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x01,
	0x0901, 0x22,
	0x0902, 0x08,
	0x3140, 0x00,
	0x3246, 0x81,
	0x3247, 0x81,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x01,
	0x0409, 0x10,
	0x040A, 0x01,
	0x040B, 0x94,
	0x040C, 0x09,
	0x040D, 0x20,
	0x040E, 0x05,
	0x040F, 0x24,

	/* Output Size Setting */
	0x034C, 0x09,
	0x034D, 0x20,
	0x034E, 0x05,
	0x034F, 0x24,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x04,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x43,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x00,
	0x3620, 0x00,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x02,
	0x3F81, 0x30,
	0x3FFC, 0x00,
	0x3FFD, 0x26,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static kal_uint16 imx576_custom5_setting[] = {
	/* MIPI output setting */
	0x0112, 0x0A,
	0x0113, 0x0A,
	0x0114, 0x03,

	/* Line Length PCK Setting */
	0x0342, 0x18,
	0x0343, 0x00,

	/* Frame Length Lines Setting */
	0x0340, 0x11,
	0x0341, 0x22,

	/* ROI Setting */
	0x0344, 0x00,
	0x0345, 0x00,
	0x0346, 0x00,
	0x0347, 0x20,
	0x0348, 0x16,
	0x0349, 0x7F,
	0x034A, 0x10,
	0x034B, 0xB7,

	/* Mode Setting */
	0x0220, 0x62,
	0x0900, 0x00,
	0x0901, 0x11,
	0x0902, 0x0A,
	0x3140, 0x00,
	0x3246, 0x01,
	0x3247, 0x01,

	/* Digital Crop & Scaling */
	0x0401, 0x00,
	0x0404, 0x00,
	0x0405, 0x10,
	0x0408, 0x02,
	0x0409, 0x20,
	0x040A, 0x01,
	0x040B, 0x70,
	0x040C, 0x12,
	0x040D, 0x40,
	0x040E, 0x0D,
	0x040F, 0xB8,

	/* Output Size Setting */
	0x034C, 0x12,
	0x034D, 0x40,
	0x034E, 0x0D,
	0x034F, 0xB8,

	/* Clock Setting */
	0x0301, 0x05,
	0x0303, 0x02,
	0x0305, 0x04,
	0x0306, 0x01,
	0x0307, 0x37,
	0x030B, 0x01,
	0x030D, 0x04,
	0x030E, 0x01,
	0x030F, 0x3C,
	0x0310, 0x01,

	/* Other Setting */
	0x0B06, 0x01,
	0x3620, 0x01,
	0x3F0C, 0x00,
	0x3F14, 0x01,
	0x3F80, 0x01,
	0x3F81, 0x72,
	0x3FFC, 0x00,
	0x3FFD, 0x3C,

	/* Integration Setting */
	0x0202, 0x07,
	0x0203, 0xD0,
	0x0224, 0x01,
	0x0225, 0xF4,
	0x3FE0, 0x03,
	0x3FE1, 0xE8,

	/* Gain Setting */
	0x0204, 0x00,
	0x0205, 0x00,
	0x0216, 0x00,
	0x0217, 0x00,
	0x0218, 0x01,
	0x0219, 0x00,
	0x020E, 0x01,
	0x020F, 0x00,
	0x3FE2, 0x00,
	0x3FE3, 0x00,
	0x3FE4, 0x01,
	0x3FE5, 0x00,

	/* Y-Hist Setting */
	0x323A, 0x00,

	/* AE-Hist Setting */
	0x323B, 0x00,

	/* Flicker Setting */
	0x323C, 0x00,
};

static void sensor_init(void)
{
	LOG_INF("%s E\n", __func__);

	imx576_table_write_cmos_sensor(imx576_init_setting,
		sizeof(imx576_init_setting)/sizeof(kal_uint16));

	set_mirror_flip(imgsensor.mirror);
	LOG_INF("%s X\n", __func__);
}	/*	sensor_init  */

static void preview_setting(void)
{
	LOG_INF("preview_setting Start\n");

	imx576_table_write_cmos_sensor(imx576_preview_setting,
		sizeof(imx576_preview_setting)/sizeof(kal_uint16));

	LOG_INF("preview_setting End\n");
}	/* preview_setting  */

static void capture_setting(kal_uint16 currefps)
{
	LOG_INF("%s 30 fps E! currefps:%d\n", __func__, currefps);

	imx576_table_write_cmos_sensor(imx576_capture_setting,
		sizeof(imx576_capture_setting)/sizeof(kal_uint16));

	LOG_INF("%s 30 fpsX\n", __func__);
} /* capture setting */

static void normal_video_setting(kal_uint16 currefps)
{
	LOG_INF("%s E! currefps:%d\n", __func__, currefps);

	imx576_table_write_cmos_sensor(imx576_normal_video_setting,
		sizeof(imx576_normal_video_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void hs_video_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_hs_video_setting,
		sizeof(imx576_hs_video_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void slim_video_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_slim_video_setting,
		sizeof(imx576_slim_video_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void custom1_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_custom1_setting,
		sizeof(imx576_custom1_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void custom2_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_custom2_setting,
		sizeof(imx576_custom2_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void custom3_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_custom3_setting,
		sizeof(imx576_custom3_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void custom4_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_custom4_setting,
		sizeof(imx576_custom4_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

static void custom5_setting(void)
{
	LOG_INF("E\n");

	imx576_table_write_cmos_sensor(imx576_custom5_setting,
		sizeof(imx576_custom5_setting)/sizeof(kal_uint16));

	LOG_INF("X\n");
}

/*add for read real sensor chip id*/
static kal_uint32 return_lot_id_from_otp(void)
{
	kal_uint16 val = 0;
	int i = 0;
	kal_uint32 sensor_id = 0;
	kal_uint8 otp_sensor_id = 0;
	UINT32   sensor_chip_id = 0;

	if (write_cmos_sensor_8(0x0a02, 0x3f) < 0)
		return 0xFFFFFFFF;
	write_cmos_sensor_8(0x0a00, 0x01);

	for (i = 0; i < 3; i++) {
		val = read_cmos_sensor(0x0A01);
		if ((val & 0x01) == 0x01)
			break;
		mDELAY(3);
	}
	if (i == 3)
		LOG_INF("read otp fail Err !\n"); /* print log */

	LOG_INF("0x0A22 0x%x 0x0A23 0x%x\n",
		read_cmos_sensor_8(0x0A22), read_cmos_sensor_8(0x0A23));
	sensor_id =
	  (read_cmos_sensor_8(0x0A22) << 4) | (read_cmos_sensor_8(0x0A23) >> 4);
	if (sensor_id == IMX576_SENSOR_ID) {
		gSensorVersion = IMX576_VER_91CUT;
		LOG_INF("This is 0.91cut version, version = 0x%x\n",
			read_cmos_sensor_8(0x0018));
		return sensor_id;
	}

	/*For 0.90 cut sensor to read sensor id*/
	sensor_chip_id =
	  ((read_cmos_sensor_8(0x0016) << 8) | read_cmos_sensor_8(0x0017));
	if (sensor_chip_id == 0x550) {
		otp_sensor_id = read_otp_sensor_id();
		if (otp_sensor_id == IMX576_OTP_ID) {
			gSensorVersion = IMX576_VER_90CUT;
			LOG_INF("This is 0.90cut version\n");
			return IMX576_SENSOR_ID;
		}
	}
	return 0;
}

/*************************************************************************
 * FUNCTION
 *	get_imgsensor_id
 *
 * DESCRIPTION
 *	This function get the sensor ID
 *
 * PARAMETERS
 *	*sensorID : return the sensor ID
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;

	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20, */
	/* we should detect the module used i2c address */
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_lot_id_from_otp();
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, *sensor_id);

				/*modify for different module*/
				imgsensor_info.module_id = read_module_id();
				LOG_INF("IMX576_module_id=%d\n",
					imgsensor_info.module_id);
				return ERROR_NONE;
			}
			LOG_INF(
				"Read sensor id fail, i2c_write_id: 0x%x  sensor_id: 0x%x\n",
				imgsensor.i2c_write_id, *sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		/* if Sensor ID is not correct, */
		/* Must set *sensor_id to 0xFFFFFFFF */
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}

/*************************************************************************
 * FUNCTION
 *	open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint16 sensor_id = 0;

	LOG_INF("PLATFORM:MT6595,MIPI 2LANE\n");
	LOG_INF(
	"preview 1280*960@30fps,864Mbps/lane; video 1280*960@30fps,864Mbps/lane;\n");
	LOG_INF("capture 5M@30fps,864Mbps/lane\n");

	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20, */
	/* we should detect the module used i2c address */
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_lot_id_from_otp();
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF(
				"Read sensor id fail, id: 0x%x sensor_id=0x%x\n",
				imgsensor.i2c_write_id, sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;
	/* initail sequence write in  */

	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.shutter = 0x3D0;
	imgsensor.gain = 0x100;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_mode = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
} /* open */



/*************************************************************************
 * FUNCTION
 *	close
 *
 * DESCRIPTION
 *
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");
	/*No Need to implement this function*/
	streaming_control(KAL_FALSE);
	return ERROR_NONE;
} /* close */


/*************************************************************************
 * FUNCTION
 * preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *	*image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E normal\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	preview_setting();

	return ERROR_NONE;
}

/*************************************************************************
 * FUNCTION
 *	capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	capture_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
} /* normal_video */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	/* imgsensor.video_mode = KAL_TRUE; */
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	/* imgsensor.current_fps = 300; */
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			     MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	/* imgsensor.video_mode = KAL_TRUE; */
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	/* imgsensor.current_fps = 300; */
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
} /* slim_video */

static kal_uint32 custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom1_setting();

	return ERROR_NONE;
}	/* custom1 */

static kal_uint32 custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
	imgsensor.pclk = imgsensor_info.custom2.pclk;
	imgsensor.line_length = imgsensor_info.custom2.linelength;
	imgsensor.frame_length = imgsensor_info.custom2.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom2_setting();

	return ERROR_NONE;
}	/* custom2 */

static kal_uint32 custom3(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM3;
	imgsensor.pclk = imgsensor_info.custom3.pclk;
	imgsensor.line_length = imgsensor_info.custom3.linelength;
	imgsensor.frame_length = imgsensor_info.custom3.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom3.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom3_setting();

	return ERROR_NONE;
}	/* custom3 */

static kal_uint32 custom4(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM4;
	imgsensor.pclk = imgsensor_info.custom4.pclk;
	imgsensor.line_length = imgsensor_info.custom4.linelength;
	imgsensor.frame_length = imgsensor_info.custom4.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom4.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom4_setting();

	return ERROR_NONE;
}	/* custom4 */

static kal_uint32 custom5(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("%s.\n", __func__);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM5;
	imgsensor.pclk = imgsensor_info.custom5.pclk;
	imgsensor.line_length = imgsensor_info.custom5.linelength;
	imgsensor.frame_length = imgsensor_info.custom5.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom5.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom5_setting();

	return ERROR_NONE;
}	/* custom5 */

static kal_uint32 get_resolution(
		MSDK_SENSOR_RESOLUTION_INFO_STRUCT(*sensor_resolution))
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth =
		imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight =
		imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth =
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth =
		imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight =
		imgsensor_info.normal_video.grabwindow_height;

	sensor_resolution->SensorHighSpeedVideoWidth =
		imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight =
		imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth =
		imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight =
		imgsensor_info.slim_video.grabwindow_height;

	sensor_resolution->SensorCustom1Width =
		imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height =
		imgsensor_info.custom1.grabwindow_height;

	sensor_resolution->SensorCustom2Width =
		imgsensor_info.custom2.grabwindow_width;
	sensor_resolution->SensorCustom2Height =
		imgsensor_info.custom2.grabwindow_height;

	sensor_resolution->SensorCustom3Width =
		imgsensor_info.custom3.grabwindow_width;
	sensor_resolution->SensorCustom3Height =
		imgsensor_info.custom3.grabwindow_height;

	sensor_resolution->SensorCustom4Width =
		imgsensor_info.custom4.grabwindow_width;
	sensor_resolution->SensorCustom4Height =
		imgsensor_info.custom4.grabwindow_height;

	sensor_resolution->SensorCustom5Width =
			imgsensor_info.custom5.grabwindow_width;
	sensor_resolution->SensorCustom5Height =
			imgsensor_info.custom5.grabwindow_height;

	return ERROR_NONE;
} /* get_resolution */

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			   MSDK_SENSOR_INFO_STRUCT *sensor_info,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat =
			imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame =
			imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame =
			imgsensor_info.slim_video_delay_frame;
	sensor_info->Custom1DelayFrame =
			imgsensor_info.custom1_delay_frame;
	sensor_info->Custom2DelayFrame =
			imgsensor_info.custom2_delay_frame;
	sensor_info->Custom3DelayFrame =
			imgsensor_info.custom3_delay_frame;
	sensor_info->Custom4DelayFrame =
			imgsensor_info.custom4_delay_frame;
	sensor_info->Custom5DelayFrame =
			imgsensor_info.custom5_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	/* The frame of setting shutter default 0 for TG int */
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	/* The frame of setting sensor gain */
	sensor_info->AESensorGainDelayFrame =
			imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame =
			imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	sensor_info->PDAF_Support = 0;

	if (gSensorVersion == IMX576_VER_90CUT) {
		/*0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR, 4:four-cell mVHDR*/
		sensor_info->HDR_Support = 0;
	} else {
		sensor_info->HDR_Support = 4;
	}

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->TEMPERATURE_SUPPORT = imgsensor_info.temperature_support;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  /* 0 is default 1x */
	sensor_info->SensorHightSampling = 0;	/* 0 is default 1x */
	sensor_info->SensorPacketECCOrder = 1;

		switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX =
					imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.pre.starty;
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
				imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX =
					imgsensor_info.cap.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.cap.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
				imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

			sensor_info->SensorGrabStartX =
					imgsensor_info.normal_video.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.normal_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			  imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			sensor_info->SensorGrabStartX =
					imgsensor_info.hs_video.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.hs_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			    imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX =
					imgsensor_info.slim_video.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.slim_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			    imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			sensor_info->SensorGrabStartX =
					imgsensor_info.custom1.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.custom1.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			  imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			sensor_info->SensorGrabStartX =
					imgsensor_info.custom2.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.custom2.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			  imgsensor_info.custom2.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			sensor_info->SensorGrabStartX =
					imgsensor_info.custom3.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.custom3.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			  imgsensor_info.custom3.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			sensor_info->SensorGrabStartX =
					imgsensor_info.custom4.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.custom4.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			  imgsensor_info.custom4.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			sensor_info->SensorGrabStartX =
					imgsensor_info.custom5.startx;
			sensor_info->SensorGrabStartY =
					imgsensor_info.custom5.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			  imgsensor_info.custom5.mipi_data_lp2hs_settle_dc;

			break;
		default:
			sensor_info->SensorGrabStartX =
						imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY =
						imgsensor_info.pre.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
				imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}

	return ERROR_NONE;
}	/*	get_info  */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			  MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		preview(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		capture(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		normal_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		hs_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		slim_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		custom1(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		custom2(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM3:
		custom3(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM4:
		custom4(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM5:
		custom5(image_window, sensor_config_data);
		break;
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}

	if (read_cmos_sensor_8(0x0900) & 0x1) {
		/*0x0902 =  0:averaged, 1:summed, 2:bayer weighting*/
		if (read_cmos_sensor_8(0x0902) & 0x1)
			imgsensor.AE_binning_type = BINNING_SUMMED;
		else
			imgsensor.AE_binning_type = BINNING_AVERAGED;
	} else
		imgsensor.AE_binning_type = BINNING_NONE;
	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	/* SetVideoMode Function should fix framerate */
	if (framerate == 0)
		/* Dynamic frame rate */
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d\n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) /* enable auto flicker */
		imgsensor.autoflicker_en = KAL_TRUE;
	else /* Cancel Auto flick */
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(
		enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

		switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			if (imgsensor.hdr_mode) {
				frame_length =
				imgsensor_info.pre_3HDR.pclk / framerate * 10 /
				imgsensor_info.pre_3HDR.linelength;
				spin_lock(&imgsensor_drv_lock);
				imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre_3HDR.framelength)
			? (frame_length - imgsensor_info.pre_3HDR.framelength)
			: 0;
				imgsensor.frame_length =
					imgsensor_info.pre_3HDR.framelength +
					imgsensor.dummy_line;
				imgsensor.min_frame_length =
					imgsensor.frame_length;
				spin_unlock(&imgsensor_drv_lock);
			} else {
				frame_length =
				    imgsensor_info.pre.pclk / framerate * 10 /
				    imgsensor_info.pre.linelength;
				spin_lock(&imgsensor_drv_lock);
				imgsensor.dummy_line =
				(frame_length > imgsensor_info.pre.framelength)
				?
				(frame_length - imgsensor_info.pre.framelength)
				: 0;
				imgsensor.frame_length =
					imgsensor_info.pre.framelength +
					imgsensor.dummy_line;
				imgsensor.min_frame_length =
					imgsensor.frame_length;
				spin_unlock(&imgsensor_drv_lock);
			}
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if (framerate == 0)
				return ERROR_NONE;
			if (imgsensor.hdr_mode) {
				frame_length =
				 imgsensor_info.pre_3HDR.pclk / framerate * 10 /
				 imgsensor_info.pre_3HDR.linelength;
				spin_lock(&imgsensor_drv_lock);
				imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre_3HDR.framelength) ?
		       (frame_length - imgsensor_info.pre_3HDR.framelength) : 0;
				imgsensor.frame_length =
					imgsensor_info.pre_3HDR.framelength +
					imgsensor.dummy_line;
				imgsensor.min_frame_length =
					imgsensor.frame_length;
				spin_unlock(&imgsensor_drv_lock);
			} else {
				frame_length =
					imgsensor_info.normal_video.pclk /
					framerate * 10 /
					imgsensor_info.normal_video.linelength;
				spin_lock(&imgsensor_drv_lock);
				imgsensor.dummy_line =
		   (frame_length > imgsensor_info.normal_video.framelength) ?
		   (frame_length - imgsensor_info.normal_video.framelength) : 0;
				imgsensor.frame_length =
				    imgsensor_info.normal_video.framelength +
				    imgsensor.dummy_line;
				imgsensor.min_frame_length =
					imgsensor.frame_length;
				spin_unlock(&imgsensor_drv_lock);
			}

			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		if (imgsensor.current_fps ==
				   imgsensor_info.cap1.max_framerate) {
			frame_length =
				imgsensor_info.cap1.pclk / framerate * 10 /
				imgsensor_info.cap1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			   (frame_length > imgsensor_info.cap1.framelength) ?
			   (frame_length - imgsensor_info.cap1.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.cap1.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		} else if (imgsensor.current_fps ==
				imgsensor_info.cap2.max_framerate) {
			frame_length =
				imgsensor_info.cap2.pclk / framerate * 10 /
				imgsensor_info.cap2.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.cap2.framelength) ?
			(frame_length - imgsensor_info.cap2.framelength) : 0;
			imgsensor.frame_length =
			 imgsensor_info.cap2.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		} else {
			if (imgsensor.current_fps !=
					imgsensor_info.cap.max_framerate)
				LOG_INF(
					"Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",
				framerate, imgsensor_info.cap.max_framerate/10);
				frame_length =
					imgsensor_info.cap.pclk /
					framerate * 10 /
					imgsensor_info.cap.linelength;
				spin_lock(&imgsensor_drv_lock);
				imgsensor.dummy_line =
			(frame_length > imgsensor_info.cap.framelength) ?
			(frame_length - imgsensor_info.cap.framelength) : 0;
				imgsensor.frame_length =
					imgsensor_info.cap.framelength +
					imgsensor.dummy_line;
				imgsensor.min_frame_length =
					imgsensor.frame_length;
				spin_unlock(&imgsensor_drv_lock);
		}
		set_dummy();
		break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length =
				imgsensor_info.hs_video.pclk / framerate * 10 /
				imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
		      (frame_length > imgsensor_info.hs_video.framelength) ?
		      (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.hs_video.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length =
				imgsensor_info.slim_video.pclk /
				framerate * 10 /
				imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
		    (frame_length > imgsensor_info.slim_video.framelength) ?
		    (frame_length - imgsensor_info.slim_video.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.slim_video.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			frame_length =
				imgsensor_info.custom1.pclk /
				framerate * 10 /
				imgsensor_info.custom1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom1.framelength) ?
			(frame_length - imgsensor_info.custom1.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.custom1.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			frame_length =
				imgsensor_info.custom2.pclk /
				framerate * 10 /
				imgsensor_info.custom2.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom2.framelength) ?
			(frame_length - imgsensor_info.custom2.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.custom2.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			frame_length =
				imgsensor_info.custom3.pclk /
				framerate * 10 /
				imgsensor_info.custom3.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom3.framelength) ?
			(frame_length - imgsensor_info.custom3.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.custom3.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			frame_length =
				imgsensor_info.custom4.pclk /
				framerate * 10 /
				imgsensor_info.custom4.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom4.framelength) ?
			(frame_length - imgsensor_info.custom4.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.custom4.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			frame_length =
				imgsensor_info.custom5.pclk /
				framerate * 10 /
				imgsensor_info.custom5.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom5.framelength) ?
			(frame_length - imgsensor_info.custom5.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.custom5.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
		default:  /* coding with  preview scenario by default */
			frame_length =
				imgsensor_info.pre.pclk / framerate * 10 /
				imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			    (frame_length > imgsensor_info.pre.framelength) ?
			    (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.pre.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			LOG_INF(
				"error scenario_id = %d, we use preview scenario\n",
				scenario_id);
			break;
	}
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(
	enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		*framerate = imgsensor_info.pre.max_framerate;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		*framerate = imgsensor_info.normal_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		*framerate = imgsensor_info.cap.max_framerate;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		*framerate = imgsensor_info.hs_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		*framerate = imgsensor_info.slim_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		*framerate = imgsensor_info.custom1.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		*framerate = imgsensor_info.custom2.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM3:
		*framerate = imgsensor_info.custom3.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM4:
		*framerate = imgsensor_info.custom4.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM5:
		*framerate = imgsensor_info.custom5.max_framerate;
		break;
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
	/* 0x5E00[8]: 1 enable,  0 disable */
	/* 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK */
		write_cmos_sensor_8(0x0601, 0x0002);
	} else {
	/* 0x5E00[8]: 1 enable,  0 disable */
	/* 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK */
		write_cmos_sensor_8(0x0601, 0x0000);
	}
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static void hdr_write_tri_shutter(kal_uint16 le, kal_uint16 me, kal_uint16 se)
{
	kal_uint16 realtime_fps = 0;

	LOG_INF("E! le:0x%x, me:0x%x, se:0x%x\n", le, me, se);
	spin_lock(&imgsensor_drv_lock);
	if (le > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = le + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	if (le < imgsensor_info.min_shutter)
		le = imgsensor_info.min_shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps =
			imgsensor.pclk / imgsensor.line_length * 10 /
			imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
			write_cmos_sensor_8(0x0104, 0x01);
			write_cmos_sensor_8(0x0340,
			imgsensor.frame_length >> 8); /*FRM_LENGTH_LINES[15:8]*/
			write_cmos_sensor_8(0x0341,
			imgsensor.frame_length & 0xFF);/*FRM_LENGTH_LINES[7:0]*/
			write_cmos_sensor_8(0x0104, 0x00);
		}
	} else {
		write_cmos_sensor_8(0x0104, 0x01);
		write_cmos_sensor_8(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor_8(0x0341, imgsensor.frame_length & 0xFF);
		write_cmos_sensor_8(0x0104, 0x00);
	}

	write_cmos_sensor_8(0x0104, 0x01);
	/* Long exposure */
	write_cmos_sensor_8(0x0202, (le >> 8) & 0xFF);
	write_cmos_sensor_8(0x0203, le & 0xFF);
	/* Muddle exposure */
	/*MID_COARSE_INTEG_TIME[15:8]*/
	write_cmos_sensor_8(0x3FE0, (me >> 8) & 0xFF);
	/*MID_COARSE_INTEG_TIME[7:0]*/
	write_cmos_sensor_8(0x3FE1, me & 0xFF);
	/* Short exposure */
	write_cmos_sensor_8(0x0224, (se >> 8) & 0xFF);
	write_cmos_sensor_8(0x0225, se & 0xFF);
	write_cmos_sensor_8(0x0104, 0x00);

	LOG_INF("L! le:0x%x, me:0x%x, se:0x%x\n", le, me, se);

}

static void hdr_write_tri_gain(kal_uint16 lgain, kal_uint16 mg, kal_uint16 sg)
{
	kal_uint16 reg_lg, reg_mg, reg_sg;

	if (lgain < BASEGAIN || lgain > 16 * BASEGAIN) {
		LOG_INF("Error gain setting");

		if (lgain < BASEGAIN)
			lgain = BASEGAIN;
		else if (lgain > 16 * BASEGAIN)
			lgain = 16 * BASEGAIN;
	}

	reg_lg = gain2reg(lgain);
	reg_mg = gain2reg(mg);
	reg_sg = gain2reg(sg);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_lg;
	spin_unlock(&imgsensor_drv_lock);
	write_cmos_sensor_8(0x0104, 0x01);
	/* Long Gian */
	write_cmos_sensor_8(0x0204, (reg_lg>>8) & 0xFF);
	write_cmos_sensor_8(0x0205, reg_lg & 0xFF);
	/* Middle Gian */
	write_cmos_sensor_8(0x3FE2, (reg_mg>>8) & 0xFF);
	write_cmos_sensor_8(0x3FE3, reg_mg & 0xFF);
	/* Short Gian */
	write_cmos_sensor_8(0x0216, (reg_sg>>8) & 0xFF);
	write_cmos_sensor_8(0x0217, reg_sg & 0xFF);

	if (lgain > mg) {
		LOG_INF("long gain > medium gain\n");
		write_cmos_sensor_8(0xEB06, 0x00);
		write_cmos_sensor_8(0xEB08, 0x00);
		write_cmos_sensor_8(0xEB0A, 0x00);
		write_cmos_sensor_8(0xEB12, 0x00);
		write_cmos_sensor_8(0xEB14, 0x00);
		write_cmos_sensor_8(0xEB16, 0x00);

		write_cmos_sensor_8(0xEB07, 0x08);
		write_cmos_sensor_8(0xEB09, 0x08);
		write_cmos_sensor_8(0xEB0B, 0x08);
		write_cmos_sensor_8(0xEB13, 0x10);
		write_cmos_sensor_8(0xEB15, 0x10);
		write_cmos_sensor_8(0xEB17, 0x10);
	} else {
		LOG_INF("long gain <= medium gain\n");
		write_cmos_sensor_8(0xEB06, 0x00);
		write_cmos_sensor_8(0xEB08, 0x00);
		write_cmos_sensor_8(0xEB0A, 0x00);
		write_cmos_sensor_8(0xEB12, 0x01);
		write_cmos_sensor_8(0xEB14, 0x01);
		write_cmos_sensor_8(0xEB16, 0x01);

		write_cmos_sensor_8(0xEB07, 0xC8);
		write_cmos_sensor_8(0xEB09, 0xC8);
		write_cmos_sensor_8(0xEB0B, 0xC8);
		write_cmos_sensor_8(0xEB13, 0x2C);
		write_cmos_sensor_8(0xEB15, 0x2C);
		write_cmos_sensor_8(0xEB17, 0x2C);
	}
	write_cmos_sensor_8(0x0104, 0x00);

	LOG_INF(
		"lgain:0x%x, reg_lg:0x%x, sg:0x%x, reg_mg:0x%x, mg:0x%x, reg_sg:0x%x\n",
		lgain, reg_lg, mg, reg_mg, sg, reg_sg);

}

static void imx576_set_lsc_reg_setting(
		kal_uint8 index, kal_uint16 *regDa, MUINT32 regNum)
{
	int i;
	int startAddr[4] = {0x9D88, 0x9CB0, 0x9BD8, 0x9B00};
	/*0:B,1:Gb,2:Gr,3:R*/

#if 0
	int R_startAddr  = 0x9B00; /*0x9B00-0x9BD7*/
	int GR_startAddr = 0x9BD8; /*0x9BD8-0x9CAF*/
	int GB_startAddr = 0x9CB0; /*0x9CB0-0x9D87*/
	int B_startAddr  = 0x9D88; /*0x9D88-0x9E5F*/
#endif
	LOG_INF("E! index:%d, regNum:%d\n", index, regNum);

	write_cmos_sensor_8(0x0B00, 0x01); /*lsc enable*/
	write_cmos_sensor_8(0x9014, 0x01);
	write_cmos_sensor_8(0x4439, 0x01);
	mdelay(1);
	LOG_INF("Addr 0xB870, 0x380D Value:0x%x %x\n",
		read_cmos_sensor_8(0xB870), read_cmos_sensor_8(0x380D));
	/*define Knot point, 2'b01:u3.7*/
	write_cmos_sensor_8(0x9750, 0x01);
	write_cmos_sensor_8(0x9751, 0x01);
	write_cmos_sensor_8(0x9752, 0x01);
	write_cmos_sensor_8(0x9753, 0x01);

	for (i = 0; i < regNum; i++)
		write_cmos_sensor(startAddr[index] + 2*i, regDa[i]);

	#if LSC_DEBUG
	for (i = 0; i < regNum; i++) {
		LOG_INF("l Addr:0x%x Value:0x%x %x Table:0x%x\n",
			(startAddr[index] + 2*i),
			read_cmos_sensor_8(startAddr[index] + 2*i),
			read_cmos_sensor_8(startAddr[index] + 2*i + 1),
			regDa[i]);
	}
	#endif
	write_cmos_sensor_8(0x0B00, 0x00); /*lsc disable*/


}

static void set_imx576_ATR(
		kal_uint16 LimitGain, kal_uint16 LtcRate, kal_uint16 PostGain)
{
	LOG_INF("Limit Gain:0x%x LTC Rate:0x%x Post Gain:0x%x\n",
		LimitGain, LtcRate, PostGain);

	write_cmos_sensor_8(0x7F77, PostGain); /*RG_TC_VE_POST_GAIN*/

	write_cmos_sensor_8(0x3C00, LtcRate & 0xFF); /*TC_LTC_RATIO_1*/
	write_cmos_sensor_8(0x3C01, LtcRate & 0xFF); /*TC_LTC_RATIO_2*/
	write_cmos_sensor_8(0x3C02, LtcRate & 0xFF); /*TC_LTC_RATIO_3*/
	write_cmos_sensor_8(0x3C03, LtcRate & 0xFF); /*TC_LTC_RATIO_4*/
	write_cmos_sensor_8(0x3C04, LtcRate & 0xFF); /*TC_LTC_RATIO_5*/

	write_cmos_sensor_8(0xA141, LimitGain & 0xFF); /*TC_LIMIT_GAIN_1*/
	write_cmos_sensor_8(0xA140, (LimitGain >> 8) & 0xFF);
	write_cmos_sensor_8(0xA147, LimitGain & 0xFF); /*TC_LIMIT_GAIN_2*/
	write_cmos_sensor_8(0xA146, (LimitGain >> 8) & 0xFF);
	write_cmos_sensor_8(0xA14D, LimitGain & 0xFF); /*TC_LIMIT_GAIN_3*/
	write_cmos_sensor_8(0xA14C, (LimitGain >> 8) & 0xFF);
	write_cmos_sensor_8(0xA153, LimitGain & 0xFF); /*TC_LIMIT_GAIN_4*/
	write_cmos_sensor_8(0xA152, (LimitGain >> 8) & 0xFF);
	write_cmos_sensor_8(0xA159, LimitGain & 0xFF); /*TC_LIMIT_GAIN_5*/
	write_cmos_sensor_8(0xA158, (LimitGain >> 8) & 0xFF);

}

static kal_uint32 imx576_awb_gain(struct SET_SENSOR_AWB_GAIN *pSetSensorAWB)
{
	UINT32 rgain_32, grgain_32, gbgain_32, bgain_32;

	LOG_INF("E\n");

	grgain_32 = (pSetSensorAWB->ABS_GAIN_GR + 1) >> 1;
	rgain_32 = (pSetSensorAWB->ABS_GAIN_R + 1) >> 1;
	bgain_32 = (pSetSensorAWB->ABS_GAIN_B + 1) >> 1;
	gbgain_32 = (pSetSensorAWB->ABS_GAIN_GB + 1) >> 1;

	LOG_INF("[%s] ABS_GAIN_GR:%d, grgain_32:%d\n",
		__func__,
		pSetSensorAWB->ABS_GAIN_GR, grgain_32);
	LOG_INF("[%s] ABS_GAIN_R:%d, rgain_32:%d\n",
		__func__,
		pSetSensorAWB->ABS_GAIN_R, rgain_32);
	LOG_INF("[%s] ABS_GAIN_B:%d, bgain_32:%d\n",
		__func__,
		pSetSensorAWB->ABS_GAIN_B, bgain_32);
	LOG_INF("[%s] ABS_GAIN_GB:%d, gbgain_32:%d\n",
		__func__,
		pSetSensorAWB->ABS_GAIN_GB, gbgain_32);

	write_cmos_sensor_8(0x0b8e, (grgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b8f, grgain_32 & 0xFF);
	write_cmos_sensor_8(0x0b90, (rgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b91, rgain_32 & 0xFF);
	write_cmos_sensor_8(0x0b92, (bgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b93, bgain_32 & 0xFF);
	write_cmos_sensor_8(0x0b94, (gbgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b95, gbgain_32 & 0xFF);

	return ERROR_NONE;
}

static kal_uint32 streaming_control(kal_bool enable)
{
	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);
	if (enable)
		write_cmos_sensor_8(0x0100, 0x01);
	else
		write_cmos_sensor_8(0x0100, 0x00);

	mdelay(10);
	return ERROR_NONE;
}

static kal_uint32 get_sensor_temperature(void)
{
	UINT8 temperature;
	INT32 temperature_convert;

	temperature = read_cmos_sensor_8(0x013a);

	if (temperature >= 0x0 && temperature <= 0x4F)
		temperature_convert = temperature;
	else if (temperature >= 0x50 && temperature <= 0x7F)
		temperature_convert = 80;
	else if (temperature >= 0x80 && temperature <= 0xEC)
		temperature_convert = -20;
	else
		temperature_convert = (INT8) temperature;

	/*
	 * LOG_INF("temp_c(%d), read_reg(%d)\n",
	 * temperature_convert, temperature);
	 */

	return temperature_convert;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				  UINT8 *feature_para,
				  UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *) feature_para;

	struct SET_PD_BLOCK_INFO_T *PDAFinfo;
	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	struct SENSOR_VC_INFO_STRUCT *pvcinfo;

	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data =
			(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.normal_video.pclk;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom1.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom2.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom3.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom4.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom5.pclk;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			if (imgsensor.hdr_mode)
				*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre_3HDR.pclk;
			else
				*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre.pclk;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre.pclk;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.cap.framelength << 16)
				+ imgsensor_info.cap.linelength;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.normal_video.framelength << 16)
				+ imgsensor_info.normal_video.linelength;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.hs_video.framelength << 16)
				+ imgsensor_info.hs_video.linelength;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.slim_video.framelength << 16)
				+ imgsensor_info.slim_video.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom1.framelength << 16)
				 + imgsensor_info.custom1.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom2.framelength << 16)
				 + imgsensor_info.custom2.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom3.framelength << 16)
				 + imgsensor_info.custom3.linelength;
		break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom4.framelength << 16)
				 + imgsensor_info.custom4.linelength;
		break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom5.framelength << 16)
				 + imgsensor_info.custom5.linelength;
		break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			if (imgsensor.hdr_mode)
				*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= (imgsensor_info.pre_3HDR.framelength << 16)
				+ imgsensor_info.pre_3HDR.linelength;
			else
				*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= (imgsensor_info.pre.framelength << 16)
				+ imgsensor_info.pre.linelength;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.pre.framelength << 16)
				+ imgsensor_info.pre.linelength;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		LOG_INF(
			"feature_Control imgsensor.pclk = %d,imgsensor.current_fps = %d\n",
			imgsensor.pclk, imgsensor.current_fps);
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter(*feature_data);
		break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
	     /* night_mode((BOOL) *feature_data); */
		break;
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16) *feature_data);
		break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
	case SENSOR_FEATURE_SET_REGISTER:
		write_cmos_sensor_8(sensor_reg_data->RegAddr,
				    sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData =
			read_cmos_sensor_8(sensor_reg_data->RegAddr);
		break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		/* get the lens driver ID from EEPROM */
		/* or just return LENS_DRIVER_ID_DO_NOT_CARE */
		/* if EEPROM does not exist in camera module. */
		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode(*feature_data);
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		get_imgsensor_id(feature_return_para_32);
		break;
	#ifdef VENDOR_EDIT
	/*modify for different module*/
	case SENSOR_FEATURE_CHECK_MODULE_ID:
		*feature_return_para_32 = imgsensor_info.module_id;
		break;
	#endif
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode((BOOL)*feature_data_16,
				      *(feature_data_16+1));
		break;
	case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
		*feature_return_para_32 = get_sensor_temperature();
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM)*feature_data,
			*(feature_data+1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM)*(feature_data),
			(MUINT32 *)(uintptr_t)(*(feature_data+1)));
		break;
	case SENSOR_FEATURE_GET_PDAF_DATA:
		LOG_INF("SENSOR_FEATURE_GET_PDAF_DATA. No support\n");
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL)*feature_data);
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		/* for factory mode auto testing */
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
LOG_INF("current fps :%d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
imgsensor.current_fps = (UINT16)*feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
	#if 0
		LOG_INF(
		"SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n",
		(UINT32)*feature_data);
	#endif
		wininfo =
			(struct SENSOR_WINSIZE_INFO_STRUCT *)
			(uintptr_t)(*(feature_data+1));
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo,
			    (void *)&imgsensor_winsize_info[1],
			    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo,
			    (void *)&imgsensor_winsize_info[2],
			    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo,
			    (void *)&imgsensor_winsize_info[3],
			    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo,
			    (void *)&imgsensor_winsize_info[4],
			    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[5],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[6],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[7],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[8],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[9],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo,
			    (void *)&imgsensor_winsize_info[0],
			    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	/*HDR CMD */
	case SENSOR_FEATURE_SET_HDR_ATR:
		LOG_INF(
			"SENSOR_FEATURE_SET_HDR_ATR Limit_Gain=%d, LTC Rate=%d, Post_Gain=%d\n",
			(UINT16)*feature_data,
			(UINT16)*(feature_data + 1),
			(UINT16)*(feature_data + 2));
		set_imx576_ATR((UINT16)*feature_data,
			(UINT16)*(feature_data + 1),
			(UINT16)*(feature_data + 2));
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INF("hdr enable :%d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.hdr_mode = (UINT8)*feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
		LOG_INF(
			"SENSOR_FEATURE_SET_HDR_SHUTTER LE=%d, SE=%d, no support\n",
			(UINT16) *feature_data,	(UINT16) *(feature_data + 1));
		#if 0
		hdr_write_shutter((UINT16) *feature_data,
				  (UINT16) *(feature_data + 1),
				  (UINT16) *(feature_data + 2));
		#endif
		break;
	case SENSOR_FEATURE_SET_HDR_TRI_SHUTTER:
		LOG_INF(
			"SENSOR_FEATURE_SET_HDR_TRI_SHUTTER LE=%d, SE=%d, ME=%d\n",
			(UINT16) *feature_data,
			(UINT16) *(feature_data + 1),
			(UINT16) *(feature_data + 2));
		hdr_write_tri_shutter((UINT16)*feature_data,
					(UINT16)*(feature_data+1),
					(UINT16)*(feature_data+2));
		break;
	case SENSOR_FEATURE_SET_HDR_TRI_GAIN:
		LOG_INF(
			"SENSOR_FEATURE_SET_HDR_TRI_GAIN LGain=%d, SGain=%d, MGain=%d\n",
			(UINT16) *feature_data,
			(UINT16) *(feature_data + 1),
			(UINT16) *(feature_data + 2));
		hdr_write_tri_gain((UINT16)*feature_data,
				   (UINT16)*(feature_data+1),
				   (UINT16)*(feature_data+2));
		break;
	case SENSOR_FEATURE_GET_VC_INFO:
		LOG_INF("SENSOR_FEATURE_GET_VC_INFO %d\n",
			(UINT16) *feature_data);
		pvcinfo =
	     (struct SENSOR_VC_INFO_STRUCT *) (uintptr_t) (*(feature_data + 1));
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[1],
				   sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[2],
				   sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[0],
				   sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_AWB_GAIN:
		/* modify to separate 3hdr and remosaic */
		if (imgsensor.sensor_mode == IMGSENSOR_MODE_CAPTURE) {
			/*write AWB gain to sensor*/
			feedback_awbgain((UINT32)*(feature_data_32 + 1),
					(UINT32)*(feature_data_32 + 2));
		} else {
			imx576_awb_gain(
				(struct SET_SENSOR_AWB_GAIN *) feature_para);
		}
		break;
	case SENSOR_FEATURE_SET_LSC_TBL:
		{
		kal_uint8 index =
			*(((kal_uint8 *)feature_para) + (*feature_para_len));

		imx576_set_lsc_reg_setting(index, feature_data_16,
					  (*feature_para_len)/sizeof(UINT16));
		}
		break;
	case SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY:
		/*
		 * SENSOR_VHDR_MODE_NONE  = 0x0,
		 * SENSOR_VHDR_MODE_IVHDR = 0x01,
		 * SENSOR_VHDR_MODE_MVHDR = 0x02,
		 * SENSOR_VHDR_MODE_ZVHDR = 0x09
		 * SENSOR_VHDR_MODE_4CELL_MVHDR = 0x0A
		 */
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0x2;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		default:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0x0;
			break;
		}
		LOG_INF(
			"SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY scenarioId:%llu, HDR:%llu\n",
			*feature_data, *(feature_data+1));
		break;
		/*END OF HDR CMD */
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
	{
		kal_uint32 rate;

		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			rate = imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			rate = imgsensor_info.normal_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			rate = imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			rate = imgsensor_info.slim_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			rate = imgsensor_info.custom1.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			rate = imgsensor_info.custom2.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			rate = imgsensor_info.custom3.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			rate = imgsensor_info.custom4.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			rate = imgsensor_info.custom5.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			rate = imgsensor_info.pre.mipi_pixel_rate;
			break;
		default:
			rate = 0;
			break;
		}
		*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = rate;
	}
	break;
	case SENSOR_FEATURE_GET_PDAF_INFO:
		#if 0
		LOG_INF(
			"SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%d\n",
			*feature_data);
		#endif
		PDAFinfo =
		(struct SET_PD_BLOCK_INFO_T *)(uintptr_t)(*(feature_data+1));
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			/*
			 * memcpy((void *)PDAFinfo,(void *)&imgsensor_pd_info,
			 * sizeof(SET_PD_BLOCK_INFO_T));
			 */
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			break;
		}
		break;
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
		/* LOG_INF("PDAF_CAPACITY scenarioId:%d\n", *feature_data); */
		/* PDAF capacity enable or not*/
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			/* video & capture use same setting */
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n",
			*feature_data);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_BINNING_TYPE:
		switch (*(feature_data + 1)) {
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CUSTOM5:
			*feature_return_para_32 = 1;
		break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_CUSTOM1:
		case MSDK_SCENARIO_ID_CUSTOM2:
		case MSDK_SCENARIO_ID_CUSTOM3:
		case MSDK_SCENARIO_ID_CUSTOM4:
		default:
			*feature_return_para_32 = 2;
		break;
		}

		pr_debug(
		"SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
		*feature_return_para_32);
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_AWB_REQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CUSTOM5:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 0;
			break;
		}
		break;
	default:
		break;
	}

	return ERROR_NONE;
}	/*	feature_control()  */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

/* kin0603 */
UINT32 IMX576_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc =  &sensor_func;
	return ERROR_NONE;
}	/*	OV5693_MIPI_RAW_SensorInit	*/