#include <stdint.h>
#include "sdk_errors.h"
#include "jeegy2_timer.h"
#include "ble_advdata.h"
#include "app_timer.h"
#include "boards.h"
#include "jeegy2_device.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_saadc.h"


extern bool f_ble_connected;
bool f_speaker_interval = false;

#define     SENSE_HIGH      0x01
#define     SENSE_MIDDLE    0x02
#define     SENSE_LOW       0x03

extern bool b_send_3_time_sense;
extern uint8_t ub1SecDelay;

APP_TIMER_DEF(m_OneSec_timer_id);
APP_TIMER_DEF(m_100ms_LED_off_timer_id);
APP_TIMER_DEF(m_3Sec_Power_on_timer_id);
APP_TIMER_DEF(m_Long_push_power_off_timer_id);
APP_TIMER_DEF(m_Long_push_power_on_timer_id);
APP_TIMER_DEF(m_Real_power_off_timer_id);
APP_TIMER_DEF(m_Speaker_toggle_timer_id);
APP_TIMER_DEF(m_Speaker_interval_timer_id);
APP_TIMER_DEF(m_Speaker_finish_timer_id);
APP_TIMER_DEF(m_Send_3_accel_sense_timer_id);
APP_TIMER_DEF(m_Battery_sense_timer_id);


/**
 *	brief  Function for create all timers 
 */
void create_app_timer(void)
{
    // Create periodic 1 second timer 
    create_one_second_timer();
    // Create LED off 100ms timer 
    create_ms100_LED_off_timer();
    // Create 3 second power on timer 
    create_threeSec_Power_on_timer();
	// Create 100ms long push power off timer 
	create_long_push_power_off_timer();
	// Create 100ms long push power on timer 
	create_long_push_power_on_timer();
	// Create real power off timer 
	create_real_power_off_timer();
	// Create speaker toggle timer 
	create_speaker_toggle_timer();
	// Create speaker interval timer 
	create_speaker_interval_timer();
	// Create speaker finish timer 
	create_speaker_finish_timer();
	// Create sending 3 times accel sense timer 
	create_send_3_accel_sense_timer();
	// Create battery sense timer
	create_battery_sense_timer();
}

/*----------------- 1 second main periodic timer -------------------*/
/**
 *	brief  Function for handker for one sec timer 
 */
void oneSec_timer_handler(void * p_context)
{
	static uint16_t count =0;
	static uint8_t led_toggle = 0;
	static uint8_t battery_check = 0;
	ret_code_t err_code = NRF_SUCCESS;

	battery_check++;
	if(battery_check > 10){
		battery_check = 0;
		err_code = nrf_drv_saadc_sample();
		APP_ERROR_CHECK(err_code);
	}
    
	
    bsp_board_led_on(0);
    start_ms100_LED_off_timer();
	uint8_t buff[20] = {0,};
	uint16_t length = 0;
	if(ub1SecDelay < 2){
		Uart_Printf("NOT ready to send packet!\r\n");
		ub1SecDelay++;
	}else{
		//sprintf((char *)buff, "Count:%d\r\n", count++);
		//Uart_Printf("1 second timer %d\r\n", count);
		//send_nus_data(buff);
		bsp_board_led_on(0);
		ping_periodic();
	}
}

/**@brief Function for 1 second periodic timer 
 */
void create_one_second_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_OneSec_timer_id, APP_TIMER_MODE_REPEATED, oneSec_timer_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting one second timer
 */
void start_one_second_timer(void)
{
	app_timer_start(m_OneSec_timer_id, APP_TIMER_TICKS(1000), NULL);
}

/**
 *	brief  Function for stop one second timer
 */
void stop_one_second_timer(void)
{
	app_timer_stop(m_OneSec_timer_id);
}

/*-------------------- 100ms LED off timer ---------------------*/
/**
 *	brief  Function for handler for 100ms LED off timer 
 */
void ms100_LED_off_timer_handler(void * p_context)
{
    bsp_board_led_off(0);
}

/**@brief Function for creating 100ms LED off timer
 */
void create_ms100_LED_off_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_100ms_LED_off_timer_id, APP_TIMER_MODE_SINGLE_SHOT, ms100_LED_off_timer_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting 100ms LED off timer
 */
void start_ms100_LED_off_timer(void)
{
	app_timer_start(m_100ms_LED_off_timer_id, APP_TIMER_TICKS(100), NULL);
}

/**
 *	brief  Function for stop 100ms LED off timer
 */
void stop_ms100_LED_off_timer(void)
{
	app_timer_stop(m_100ms_LED_off_timer_id);
}

/*----------------- 3 second power on timer -------------------*/
/**
 *	brief  Function for handler for 3 second power on timer 
 */
void threeSec_Power_on_timer_handler(void * p_context)
{
    //bool key_pressed =  bsp_board_button_state_get(BUTTON_1);   
	if(nrf_gpio_pin_read(BUTTON_1) == true){
        //sleep_mode_enter();
		// for debugging 
		stop_long_push_power_on_timer();
		stop_threeSec_Power_on_timer();
    }else{
		stop_long_push_power_on_timer();
		stop_threeSec_Power_on_timer();
	}
}

/**@brief Function for creating 3 second power on timer
 */
void create_threeSec_Power_on_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_3Sec_Power_on_timer_id, APP_TIMER_MODE_SINGLE_SHOT, threeSec_Power_on_timer_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting 3 second power ontimer
 */
void start_threeSec_Power_on_timer(void)
{
	app_timer_start(m_3Sec_Power_on_timer_id, APP_TIMER_TICKS(1000), NULL);
}

/**
 *	brief  Function for stop 3 second power on timer
 */
void stop_threeSec_Power_on_timer(void)
{
	app_timer_stop(m_3Sec_Power_on_timer_id);
}

/*----------------- long push power off timer -------------------*/
/**
 *	brief  Function for handler for long push power off timer 
 */
void long_push_power_off_timer_handler(void * p_context)
{
	static int power_off_led_cnt = 0;
	static bool power_off_led_toggle = true;
	Uart_Printf("long push off test\r\n");
	if(power_off_led_toggle == true){
		power_off_led_toggle = false;
		bsp_board_led_off(0);
		bsp_board_led_off(1);
	}else{
		power_off_led_toggle = true;
		bsp_board_led_on(0);
		bsp_board_led_on(1);
	}
	power_off_led_cnt++;
	if(power_off_led_cnt > 10 ){
		bsp_board_led_off(0);
		bsp_board_led_off(1);
		stop_long_push_power_off_timer();
		stop_one_second_timer();
		start_real_power_off_timer();
	}
	
}

/**@brief Function for creating long push power off timer
 */
void create_long_push_power_off_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Long_push_power_off_timer_id, APP_TIMER_MODE_REPEATED, long_push_power_off_timer_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**t
 *	brief  Function for starting 3 second power ontimer
 */
void start_long_push_power_off_timer(void)
{
	app_timer_start(m_Long_push_power_off_timer_id, APP_TIMER_TICKS(100), NULL);
}

/**
 *	brief  Function for stop 3 second power on timer
 */
void stop_long_push_power_off_timer(void)
{
	app_timer_stop(m_Long_push_power_off_timer_id);
}

/*----------------- long push power on timer -------------------*/
/**
 *	brief  Function for handler for long push power on timer 
 */
void long_push_power_on_timer_handler(void * p_context)
{
	static int power_on_led_cnt = 0;
	static bool power_on_led_toggle = true;
	Uart_Printf("long push on test\r\n");
	if(power_on_led_toggle == true){
		power_on_led_toggle = false;
		bsp_board_led_off(0);
		bsp_board_led_off(1);
	}else{
		power_on_led_toggle = true;
		bsp_board_led_on(0);
		bsp_board_led_on(1);
	}
}

/**@brief Function for creating long push power on timer
 */
void create_long_push_power_on_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Long_push_power_on_timer_id, APP_TIMER_MODE_REPEATED, long_push_power_on_timer_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting long push power on timer
 */
void start_long_push_power_on_timer(void)
{
	app_timer_start(m_Long_push_power_on_timer_id, APP_TIMER_TICKS(100), NULL);
}

/**
 *	brief  Function for stop long push power on timer
 */
void stop_long_push_power_on_timer(void)
{
	app_timer_stop(m_Long_push_power_on_timer_id);
}

/*----------------- real power off timer -------------------*/
/**
 *	brief  Function for handler for real power off timer 
 */
void real_power_off_timer_handler(void * p_context)
{
	sleep_mode_enter();	
}

/**@brief Function for creating real power off timer
 */
void create_real_power_off_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Real_power_off_timer_id, APP_TIMER_MODE_SINGLE_SHOT, real_power_off_timer_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting real power off timer
 */
void start_real_power_off_timer(void)
{
	app_timer_start(m_Real_power_off_timer_id, APP_TIMER_TICKS(4000), NULL);
}

/**
 *	brief  Function for stop 3 second power on timer
 */
void stop_real_power_off_timer(void)
{
	app_timer_stop(m_Real_power_off_timer_id);
}

/*----------------- speaker timer -------------------*/
/**
 *	brief  Function for handler for speaker timer 
 */
void speaker_toggle_handler(void * p_context)
{
	if(f_speaker_interval == true){
		generate_toggle();
	}else{
		mute_speaker();
	}
}

/**@brief Function for creating speaker timer
 */
void create_speaker_toggle_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Speaker_toggle_timer_id, APP_TIMER_MODE_REPEATED, speaker_toggle_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting speaker timer
 */
void start_speaker_toggle_timer(void)
{
	app_timer_start(m_Speaker_toggle_timer_id, APP_TIMER_TICKS(1), NULL);
}

/**
 *	brief  Function for stop speaker timer
 */
void stop_speaker_toggle_timer(void)
{
	app_timer_stop(m_Speaker_toggle_timer_id);
}

/*----------------- speaker interval timer -------------------*/
/**
 *	brief  Function for handler for speaker interval timer 
 */
void speaker_interval_handler(void * p_context)
{
	if(f_speaker_interval == false){
		f_speaker_interval = true;	
	}else{
		f_speaker_interval = false;
	}
}

/**@brief Function for creating speaker timer
 */
void create_speaker_interval_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Speaker_interval_timer_id, APP_TIMER_MODE_REPEATED, speaker_interval_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting speaker timer
 */
void start_speaker_interval_timer(void)
{
	app_timer_start(m_Speaker_interval_timer_id, APP_TIMER_TICKS(50), NULL);
}

/**
 *	brief  Function for stop speaker timer
 */
void stop_speaker_interval_timer(void)
{
	app_timer_stop(m_Speaker_interval_timer_id);
}

/*----------------- speaker finish timer -------------------*/
/**
 *	brief  Function for handler for speaker finish timer 
 */
void speaker_finish_handler(void * p_context)
{
	stop_speaker();	
}

/**@brief Function for creating speaker finish timer
 */
void create_speaker_finish_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Speaker_finish_timer_id, APP_TIMER_MODE_SINGLE_SHOT, speaker_finish_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting speaker finish timer
 */
void start_speaker_finish_timer(void)
{
	app_timer_start(m_Speaker_finish_timer_id, APP_TIMER_TICKS(30000), NULL);
}

/**
 *	brief  Function for stop speaker timer
 */
void stop_speaker_finish_timer(void)
{
	app_timer_stop(m_Speaker_finish_timer_id);
}

/*----------------- send 3 times accel sense timer  -------------------*/
/**
 *	brief  Function for handler for sending 3 times accel sense timer 
 */
void send_3_accel_sense_handler(void * p_context)
{
	static uint8_t u8Send3Time = 0;
	uint8_t buff[10] = {0,};
	uint8_t length = 5;
	u8Send3Time++;
	Uart_Printf("send 3 accel handler\r\n");
	if(u8Send3Time > 3){
		u8Send3Time = 0;
		stop_send_3_accel_sense_timer();
		b_send_3_time_sense = false;
	}else{
		b_send_3_time_sense = true;
	}
}

/**@brief Function for creating sending 3 times accel sense timer 
 */
void create_send_3_accel_sense_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Send_3_accel_sense_timer_id, APP_TIMER_MODE_REPEATED, send_3_accel_sense_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting sending 3 times accel sense timer 
 */
void start_send_3_accel_sense_timer(void)
{
	Uart_Printf("start send 3 accel timer\r\n");
	app_timer_start(m_Send_3_accel_sense_timer_id, APP_TIMER_TICKS(1000), NULL);
}

/**
 *	brief  Function for stop ssending 3 times accel sense timer 
 */
void stop_send_3_accel_sense_timer(void)
{
	Uart_Printf("stop send 3 accel timer\r\n");
	app_timer_stop(m_Send_3_accel_sense_timer_id);
}

/*----------------- battery sense timer  -------------------*/
/**
 *	brief  Function for handler for battery sense timer 
 */
void battery_sense_handler(void * p_context)
{
	Uart_Printf("Battery sense handler\r\n");
	
	ret_code_t err_code;
    //err_code = nrf_drv_saadc_sample();
    //if (err_code != NRF_SUCCESS)
	//{
	//	Uart_Printf("SAADC failed!\r\n");
	//}
}

/**@brief Function for creating battery sense timer 
 */
void create_battery_sense_timer(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = app_timer_create(&m_Battery_sense_timer_id, APP_TIMER_MODE_REPEATED, battery_sense_handler);
	if (err_code != NRF_SUCCESS)
	{
		
	}
}

/**
 *	brief  Function for starting battery sense timer 
 */
void start_battery_sense_timer(void)
{
	Uart_Printf("start battery sense timer\r\n");
	app_timer_start(m_Battery_sense_timer_id, APP_TIMER_TICKS(5000), NULL);
}

/**
 *	brief  Function for stop ssending 3 times accel sense timer 
 */
void stop_battery_sense_timer(void)
{
	Uart_Printf("stop battery sense timer\r\n");
	app_timer_stop(m_Battery_sense_timer_id);
}