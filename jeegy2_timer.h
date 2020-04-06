#ifndef _JEEGY2_TIMER_
#define _JEEGY2_TIMER_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// function prototype 
void create_app_timer(void);

void oneSec_timer_handler(void * p_context);
void create_one_second_timer(void);
void start_one_second_timer(void);
void stop_one_second_timer(void);

void ms100_LED_off_timer_handler(void * p_context);
void create_ms100_LED_off_timer(void);
void start_ms100_LED_off_timer(void);
void stop_ms100_LED_off_timer(void);

void threeSec_Power_on_timer_handler(void * p_context);
void create_threeSec_Power_on_timer(void);
void start_threeSec_Power_on_timer(void);
void stop_threeSec_Power_on_timer(void);

void long_push_power_off_timer_handler(void * p_context);
void create_long_push_power_off_timer(void);
void start_long_push_power_off_timer(void);
void stop_long_push_power_off_timer(void);

void long_push_power_on_timer_handler(void * p_context);
void create_long_push_power_on_timer(void);
void start_long_push_power_on_timer(void);
void stop_long_push_power_on_timer(void);

void real_power_off_timer_handler(void * p_context);
void create_real_power_off_timer(void);
void start_real_power_off_timer(void);
void stop_real_power_off_timer(void);

#endif