/*------------------------------------------------------------------------------------------------*/
/*                                                                                                */
/*           Copyright (C) 2022 NeuronBasic Co., Ltd. All rights reserved.                        */
/*                                                                                                */
/*------------------------------------------------------------------------------------------------*/
#include "nbsdk.h"
#include "bsp.h"
#include "nb_cmd_api.h"
#include "nbsdk_api.h"
#include "include/sensor_aeg.h"
#include "include/Basic.h"
#include "Token.h"

#include "User_Config.h"

#define FRAME_FINSH 0
#define TASK_DELAY_TIME 2

#define HOST_SECTION __attribute__((section(".ver_host")))
static const char HOST_SECTION host_ver[] = "HOST-"__DATE__
											"-"__TIME__;
static TaskHandle_t xHandle_Win = NULL;
static TaskHandle_t xHandle_User = NULL;
unsigned char detect_enable = TRUE;
u_int32_t Sys_Tick = 0;

static void feature_init(int frame_count)
{
    scaler_init(0);
    scaler_start();
    /*config md*/
    int r = md_init(MD_AREA_FIXED);
    if (r)
    {
        printf("md init fail\n");
    }
    /*start md*/
    md_start();

    int mode = 0;                   //Fe mode. 0: fe mode 1: quat mode 2:scaler mode 3:mixed mode
    int dma_mode = 1 ;              //0: FIFO mode 1: dma mode //dma is faster
    static int ret = fe_init(mode, frame_count, dma_mode);
    if (ret)
    {
        printf("fe init fail\n");
    }
    return ret;
}


static void Win_Task(void *parameters)
{
#define PLAN_X0 80
#define PLAN_Y0 80
#define PLAN_Xe PLAN_X0+160
#define PLAN_Ye PLAN_Y0+160
#define HOG_BIN_SIZE 80
#define HOG_SIZE_0 10*10*18
#define HOG_SIZE_1 10*9*18
#define HOG_SIZE_2 9*10*18
#define HOG_SIZE_3 9*9*18
#define TOTAL_HOG_SIZE HOG_SIZE_0+HOG_SIZE_1+HOG_SIZE_2+HOG_SIZE_3


	debug_enable_box(0, 1, PLAN_X0, PLAN_Y0, PLAN_Xe, PLAN_Ye);
	//do not use sensor save to memory
	uint8_t *fe_bufcopy = (uint8_t*)0x2002C000;
	//make sure the memory is null
	memset(fe_bufcopy,NULL,TOTAL_HOG_SIZE);
	struct fe_output_frame_t *frame;
	int ret = 0;
	
	// fe init & start
	feature_init(2);

	struct scaler_wins_t wins = {0};
    memset(&wins,0,sizeof(struct scaler_wins_t));
    wins.win[0].sx = PLAN_X0;
	wins.win[0].sy = PLAN_Y0;
	wins.win[0].ex = PLAN_Xe;
	wins.win[0].ey = PLAN_Ye;
	wins.win[0].hsize = HOG_BIN_SIZE;
	wins.win[0].vsize = HOG_BIN_SIZE;
	wins.win[0].valid = 1;
	wins.win[1].sx = PLAN_X0+8;
	wins.win[1].sy = PLAN_Y0;
	wins.win[1].ex = PLAN_X0+8+144;
	wins.win[1].ey = PLAN_Ye;
	wins.win[1].hsize = HOG_BIN_SIZE-8;
	wins.win[1].vsize = HOG_BIN_SIZE;
	wins.win[1].valid = 1;
	scaler_change_wins(&wins); 
    fe_change_win(&wins);
    fe_start();
	printf("feature start\n");
	// osMemInfoDump(0);

	for (;;)
	{
		// do fe every frame
		frame = fe_get_buffer();
		printf("frame: %d\n",frame->frame_id);
		if (frame->frame_id == 100)
		{			
			//parse and check fe data
			//do not delete!!
			ret = fe_check_result(frame);
			if (ret)
			{
				printf("feature hog is error! \n");
			}
			fe_reorder_result(frame, 0, fe_bufcopy, HOG_SIZE_0);			
			fe_reorder_result(frame, 1, fe_bufcopy+HOG_SIZE_0, HOG_SIZE_1);

			// change win before next frame
			wins.win[0].sx = PLAN_X0;
			wins.win[0].sy = PLAN_Y0+8;
			wins.win[0].ex = PLAN_Xe;
			wins.win[0].ey = PLAN_Y0+8+144;
			wins.win[0].hsize = HOG_BIN_SIZE;
			wins.win[0].vsize = HOG_BIN_SIZE-8;
			wins.win[0].valid = 1;
			wins.win[1].sx = PLAN_X0+8;
			wins.win[1].sy = PLAN_Y0+8;
			wins.win[1].ex = PLAN_X0+8+144;
			wins.win[1].ey = PLAN_Y0+8+144;
			wins.win[1].hsize = HOG_BIN_SIZE-8;
			wins.win[1].vsize = HOG_BIN_SIZE-8;
			wins.win[1].valid = 1;
			fe_change_win_force(&wins);
		}

		if (frame->frame_id == 101)
		{
			
			if (frame)
			{
				printf("detect set feature start 101 \n");		
				ret = fe_check_result(frame);
				if (ret)
				{
					printf("feature hog is error! \n");
				}
				fe_reorder_result(frame, 0, fe_bufcopy+HOG_SIZE_0+HOG_SIZE_1,HOG_SIZE_2);
				fe_reorder_result(frame, 1, fe_bufcopy+HOG_SIZE_0+HOG_SIZE_1+HOG_SIZE_2, HOG_SIZE_3);
			}
		}

		if (frame->frame_id > 110)
		{
			// combine 2 u8
			u16* fe_bufcopy16 = fe_bufcopy;
			printf("\n start*********************\n ");
			for (int i = 0; i < 900+810+810+729; i++)
			{
				if (i==900||i==900+810||i==900+810+810)
					printf("\n end*********************\n ");
				printf("%d ", fe_bufcopy16[i]);
			}
			printf("\n end*********************\n ");
			break;
		}

		// must put back every time u get buffer
		fe_put_buffer(frame);
	}
	vTaskDelete(NULL);
}

static void winTaskInit(void)
{
	BaseType_t xTask_Win;
	xTask_Win = xTaskCreate(
		Win_Task,
		"Win_Task",
		configMINIMAL_STACK_SIZE * 8,
		NULL,
		tskIDLE_PRIORITY + 5,
		&xHandle_Win);
	if (xTask_Win != pdPASS)
	{
		DEBUG(DEBUG_ERROR, "Win_Task is NULL!\n");
		exit(FALSE);
	}
}

int main(void)
{
	SetDebugLevel(DEBUG_LEVEL, DETAIL_INFO_SUPPORT);
	struct Init_Config *BasicInitConfig = get_init_config();
	BasicInitConfig->BaudRate = UART_DEFAULT_BAUD_RATE_INDEX;
	BasicInitConfig->SPISpeed = FLASH_SPI_SPEED;
	BasicInitConfig->CISWorkFreq = SYS_WORK_FREQ;
	BasicInitConfig->SystemInfoDump = SYS_INFO_DUMP_SW;
	BasicInitConfig->IOPortingMode = IO_PORTING_MODE;
	BasicInitConfig->DebugMode = DEBUG_MODE;
	BasicInitConfig->BLCEnable = BLC_SW;
	InitializeBase(BasicInitConfig);

#if (AEG_MODE == MACR_AEG_APPLICATION_PROJECT_CUSTOM)
	sensor_aeg_custom_app_info_structure app_info_str = {0};
	app_info_str.adjust_strategy = AEG_LIGHT_DETECT_TYPE;
	app_info_str.frm_rate_max = AEG_FRAME_RATE_MODE_MAX;
	app_info_str.frm_rate_min = AEG_FRAME_RATE_MODE_MIN;
	app_info_str.lpm_opt = AEG_POWER_MANAGEMENT_MODE;
	app_info_str.light_level_max = AEG_LED_LIGHT_CTRL_MAX;
	app_info_str.light_level_min = AEG_LED_LIGHT_CTRL_MIN;
	app_info_str.ana_gain_max = AEG_ANALOG_GAIN_MAX;
	app_info_str.md_info_filter = AEG_MOTION_LOCATION_FILTER;
	app_info_str.dete_info_filter = AEG_DETECT_LOCATION_FILTER;
	app_info_str.test_mode_valid_flag = AEG_TEST_MODE_VALID_SUPPORT;
	app_info_str.valid_flag = AEG_CUSTOM_VALID_SUPPORT;
	sensor_aeg_update_custom_application_info(app_info_str);
#endif // #if (AEG_MODE == MACR_AEG_APPLICATION_PROJECT_CUSTOM )
	sensor_aeg_init(AEG_MODE, AEG_AC_POWER_FREQ, AEG_GAMMA_MODE);
	sensor_aeg_video_debug_label_enable(DISABLE);
	sensor_set_exposure_row(396);
	sensor_set_analog_gain(2);

 	winTaskInit();
	return 0;
}
