#include <stdint.h>
#include "jeegy2_device.h"
#include "bsp_btn_ble.h"
#include "nrf_delay.h"

#define     SENSE_HIGH      0x01
#define     SENSE_MIDDLE    0x02
#define     SENSE_LOW       0x03

#define     SPEAKER_OFF 	28
#define     BUZZER			3

bool    f_alarm_func_enable = false;
bool    f_mute_speaker = false;
uint8_t u8_accel_sense = SENSE_MIDDLE;

extern uint8_t           percentage_batt_lvl;
extern bool f_ble_connected;

/**
 *	brief  Function for stopping alarm function ( smart phone -> device )
 */
void stop_alarm_function(void)
{
    Uart_Printf("Stop alarm function\r\n");
    f_alarm_func_enable = false;
    f_mute_speaker = false;
}

/**
 *	brief  Function for starting alarm function ( smart phone -> device )
 */
void start_alarm_function(void)
{
    Uart_Printf("Start alarm function\r\n");
    f_alarm_func_enable = true;
}

/**
 *	brief  Function for setting touch sensitivity as high  ( smart phone -> device )
 */
void set_sense_high(void)
{
    Uart_Printf("Set aceel sens HIGH\r\n");
    u8_accel_sense = SENSE_HIGH; 
    // write accel code 
}

/**
 *	brief  Function for setting touch sensitivity as middle ( smart phone -> device )
 */
void set_sense_middle(void)
{
    Uart_Printf("Set aceel sens MIDDLE\r\n");
    u8_accel_sense = SENSE_MIDDLE;
    // write accel code 
}

/**
 *	brief  Function for setting touch sensitivity as low ( smart phone -> device )
 */
void set_sense_low(void)
{
    Uart_Printf("Set aceel sens LOW\r\n");
    u8_accel_sense = SENSE_LOW;
    // write accel code
}

/**
 *	brief  Function for mutting device alarm  ( smart phone -> device ) 
 */
void mute_alarm(void)
{
    Uart_Printf("Mute alarm\r\n");
    f_mute_speaker = false;
    stop_speaker();

}

/**
 *	brief  Function for testing device LED  ( smart phone -> device )
 */
void test_LED(void)
{
    Uart_Printf("Test LED\r\n");
    for(uint8_t i = 0;i<5;i++){
        bsp_board_led_on(0);
	    bsp_board_led_on(1);
        nrf_delay_ms(100);
        bsp_board_led_off(0);
	    bsp_board_led_on(1);
        nrf_delay_ms(100);
    }   
}


/**
 *	brief  Function for testing device speaker ( smart phone -> device ) 
 */
void test_speaker(void)
{
    Uart_Printf("Test speaker\r\n");
    start_speaker();
}

/**
 *	brief  Function for testing accelerometer  ( smart phone -> device )
 */
void test_accel(void)
{
    Uart_Printf("Test accelerometer\r\n");
    uint8_t uID = u8LIS2_TestRead();
    if(uID == 0x33){
        for(int j = 0;j<3;j++){
            bsp_board_led_on(0);
            bsp_board_led_on(1);
            nrf_delay_ms(100);
            bsp_board_led_off(0);
            bsp_board_led_off(1);
            nrf_delay_ms(100);
            
        }
    }else{
        for( ; ; ){
            bsp_board_led_on(0);
            bsp_board_led_on(1);
            nrf_delay_ms(100);
            bsp_board_led_off(0);
            bsp_board_led_off(1);
            nrf_delay_ms(100);
            
        }
    }
}

/**
 *	brief  Function for disconnecting the device  ( smart phone -> device )
 */
void test_discon_device(void)
{
    Uart_Printf("Test disconnect device\r\n");
}

/**
 *	brief  Function for send signal when touch is detected  ( device  -> smart phone) 
 */
uint32_t touch_detected(void)
{
    uint8_t buff[10] = {0,};
    buff[0] = 't';
    buff[1] = SENSE_MIDDLE;
    buff[2] = 0x00;
    buff[3] = 0x00;
    buff[4] = 0x00;
    if(f_alarm_func_enable == true){
        Uart_Printf("Touch detected!!!\r\n");
        send_nus_data(buff);
        start_speaker();
    }
}
				

/**
 *	brief  Function for send a ping ( device  -> smart phone) 
 *         Every 1 sec, this ping signal is sent.
 */
uint32_t ping_periodic(void)
{
    uint8_t buff[10] = {0,};

    if(f_ble_connected == false){
        Uart_Printf("Not connected\r\n");
        return;
    }
   
    if(f_alarm_func_enable == true){

        Uart_Printf("Ping alarm on\r\n");
        buff[0] = 'b';
        buff[1] = percentage_batt_lvl;
        buff[2] = u8_accel_sense;
        buff[3] = 0x00;
        buff[4] = 0x00;
        send_nus_data(buff);
    }else{
        Uart_Printf("Ping alarm off\r\n");
        buff[0] = 'c';
        buff[1] = percentage_batt_lvl;
        buff[2] = u8_accel_sense;
        buff[3] = 0x00;
        buff[4] = 0x00;
        send_nus_data(buff);

    }
}



/**
 *	brief  Function for send responce signal of "test LED" ( device  -> smart phone) 
 */
uint32_t test_resp_LED(void)
{
    Uart_Printf("Response of LED test\r\n");
}

/**
 *	brief  Function for send responce signal of "test accel" ( device  -> smart phone) 
 */
uint32_t test_resp_accel(void)
{
    Uart_Printf("Response of LED test\r\n");
}

/**
 *	brief  Function for send responce signal of "test speaker" ( device  -> smart phone) 
 */
uint32_t test_resp_speaker(void)
{
    Uart_Printf("Response of speaker test\r\n");
}

/**
 *	brief  Function for stopping speaker 
 */
void stop_speaker(void)
{
    nrf_gpio_pin_clear(BUZZER);
    nrf_gpio_pin_clear(SPEAKER_OFF);    
    stop_speaker_toggle_timer();
    stop_speaker_finish_timer();
    stop_speaker_interval_timer();
}

/**
 *	brief  Function for mute speaker 
 */
void mute_speaker(void)
{
    nrf_gpio_pin_clear(BUZZER);
}

/**
 *	brief  Function for start speaker 
 */
void start_speaker(void)
{
    nrf_gpio_pin_set(SPEAKER_OFF);    
    start_speaker_toggle_timer();
    start_speaker_finish_timer();
    start_speaker_interval_timer();
}

/**
 *	brief  Function for init speaker 
 */
void init_speaker(void)
{
    nrf_gpio_cfg_output(SPEAKER_OFF);
	nrf_gpio_cfg_output(BUZZER);
	nrf_gpio_pin_write(SPEAKER_OFF, 0);
}

/**
 *	brief  Function for init speaker 
 */
void detect_touch(void)
{
    if(f_alarm_func_enable == true){
        start_speaker();
    }
}

/**
 *	brief  Function for generating toggle 
 */
void generate_toggle(void)
{
    nrf_gpio_pin_toggle(BUZZER);	
}