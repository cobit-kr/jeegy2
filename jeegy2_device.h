#ifndef _JEEGY2_DEVICE_
#define _JEEGY2_DEVICE_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t u8_accel_sense;

// function prototype 
// smart phone -> device 
void stop_alarm_function(void);
void start_alarm_function(void);
void set_sense_high(void);
void set_sense_middle(void);
void set_sense_low(void);
void mute_alarm(void);
void test_LED(void);
void test_accel(void);
void test_speaker(void);

// device -> smart phone 
uint32_t touch_detected(void);
uint32_t ping_periodic(void);
uint32_t find_phone(void);
uint32_t test_resp_LED(void);
uint32_t test_resp_accel(void);
uint32_t test_resp_speaker(void);

#endif