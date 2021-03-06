/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */


#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_drv_saadc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_drv_wdt.h"
#include "app_timer.h"
#include "app_util.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "LIS2DH12registers.h"
#include "LIS2DH12.h"
#include "nrf_drv_gpiote.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"


#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
#include "nrf_sdm.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "jeegy2_timer.h"
#include "jeegy2_device.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "jeegy2"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

#define JEEGYSHOW_KEY0					0
#define JEEGYSHOW_KEY1					1

#define     SENSE_HIGH      0x01
#define     SENSE_MIDDLE    0x02
#define     SENSE_LOW       0x03

#define	DEBUG_UART_ON		1
#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
#define MAX_DEBUG_MSG			50
#define MCU_PROMPT			"JeegyShow>"

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   600                                     /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION    6                                       /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS  270                                     /**< Typical forward voltage drop of the diode . */
#define ADC_RES_10BIT                   1024                                    /**< Maximum digital value for 10-bit ADC conversion. */

#define LESC_DEBUG_MODE                     0  

#define SEC_PARAM_BOND                      1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      0                                       /**< LE Secure Connections enabled. */
#define SEC_PARAM_KEYPRESS                  0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                      /**< Maximum encryption key size. */


#define TX_POWER_LEVEL                  (8)

/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 *
 * @retval     Result converted to millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

void vLIS2_Init (void);


/* Temporal TWI and LIS2D setting */
#define	LIS2DH_ADD			0x19

#define	TWI_SCL	            11
#define	TWI_SDA	            12

#define TWI_INSTANCE_ID		0

// Watch dog 
nrf_drv_wdt_channel_id m_channel_id;

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
// Semaphore: true if TWI transfer operation has completed
static volatile bool boTransferDone = false;
static ret_code_t twi_err_code;

static volatile bool boInterruptEvent = false;

static uint8_t u8SignExtend;

bool b_send_3_time_sense = false;

static nrf_saadc_value_t adc_buf[2];

enum AccelRES
{
	mode_8bit,
	mode_10bit,
	mode_12bit
};

/**
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    //__disable_irq();
    NRF_LOG_FINAL_FLUSH();

#ifndef DEBUG
    NRF_LOG_ERROR("Fatal error");
#else
    switch (id)
    {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
        case NRF_FAULT_ID_SD_ASSERT:
            NRF_LOG_ERROR("SOFTDEVICE: ASSERTION FAILED");
            break;
        case NRF_FAULT_ID_APP_MEMACC:
            NRF_LOG_ERROR("SOFTDEVICE: INVALID MEMORY ACCESS");
            break;
#endif
        case NRF_FAULT_ID_SDK_ASSERT:
        {
            assert_info_t * p_info = (assert_info_t *)info;
            NRF_LOG_ERROR("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
            break;
        }
        case NRF_FAULT_ID_SDK_ERROR:
        {
            error_info_t * p_info = (error_info_t *)info;
            NRF_LOG_ERROR("ERROR %u [%s] at %s:%u\r\nPC at: 0x%08x",
                          p_info->err_code,
                          nrf_strerror_get(p_info->err_code),
                          p_info->p_file_name,
                          p_info->line_num,
                          pc);
             NRF_LOG_ERROR("End of error report");
            break;
        }
        default:
            NRF_LOG_ERROR("UNKNOWN FAULT at 0x%08X", pc);
            break;
    }
#endif

    NRF_BREAKPOINT_COND;
    // On assert, the system can only recover with a reset.

#ifndef DEBUG
    NRF_LOG_WARNING("System reset");
    for(int i=0;i<10;i++){
        bsp_board_led_on(0);
	    bsp_board_led_on(1);
        nrf_delay_ms(100);
        bsp_board_led_off(0);
	    bsp_board_led_off(1);
        nrf_delay_ms(100);
    }
    NVIC_SystemReset();
#else
    //app_error_save_and_stop(id, pc, info);
    assert_info_t * p_info = (assert_info_t *)info;
    Uart_Printf("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
    for(int j = 0;j<10;j++){
        bsp_board_led_on(0);
	    bsp_board_led_on(1);
        nrf_delay_ms(100);
        bsp_board_led_off(0);
	    bsp_board_led_off(1);
        nrf_delay_ms(100);
        
    }
    NVIC_SystemReset();
#endif // DEBUG
}


// Currently selected resolution
enum AccelRES eRsolution;


static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

bool  f_ble_connected = false;

unsigned char ubMsgCnt=0;
unsigned char ubGotDbgMsg = 0;
unsigned char ubDebugOld[MAX_DEBUG_MSG];
unsigned char ubDebugMsg[MAX_DEBUG_MSG];

uint8_t ubJeegyShowUartEnabled =0;

uint8_t ub1SecDelay = 0;


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for changing the tx power.
 */
static void tx_power_set(void)
{
    ret_code_t err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_advertising.adv_handle, TX_POWER_LEVEL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        /*for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
        {
            while (app_uart_put('\n') == NRF_ERROR_BUSY);
        }*/
        switch(p_evt->params.rx_data.p_data[0]){
            case 'S':
            {
                stop_alarm_function();
            }
            break;
            case 'P':
            {
                start_alarm_function();
            }
            break;
            case 'H':
            {
                set_sense_high();
            }
            break;
            case 'M':
            {
                set_sense_middle();
            }
            break;
            case 'L':
            {
                set_sense_low();
            }
            break;
            case 'Z':
            {
                mute_alarm();
            }
            break;
            case 'A':
            {
                test_LED();
            }
            break;
            case 'B':
            {
                test_accel();
                touch_detected();
            }
            break;
            case 'C':
            {
                test_speaker();
            }
            break;
        }
    }
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            //APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            /*
                After booting device, (may be 30 second) device is scheduled to sleep.
                So comment below code. Jeegy 2 should continue to work. 
            */
            Uart_Printf("BLE_ADV_EVT_IDLE\r\n");
            err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
            //sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            Uart_Printf("connected! \r\n");
            ub1SecDelay = 0;
            //start_send_3_accel_sense_timer();
            //err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            //APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            //err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            //APP_ERROR_CHECK(err_code);
            
            f_ble_connected = true;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            Uart_Printf("disconnected! \r\n");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            f_ble_connected = false;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            //err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            //APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            //err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            //APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for assigning button and its event
 */
uint32_t init_ButtonEvent(void)
{
	uint32_t err_code=0;
	
	err_code = bsp_event_to_button_action_assign(JEEGYSHOW_KEY0, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_0_SHORT);
    APP_ERROR_CHECK(err_code);

	err_code = bsp_event_to_button_action_assign(JEEGYSHOW_KEY0, BSP_BUTTON_ACTION_LONG_PUSH, BSP_EVENT_KEY_0_LONG);
	APP_ERROR_CHECK(err_code);

	err_code = bsp_event_to_button_action_assign(JEEGYSHOW_KEY1, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_1_SHORT);
	APP_ERROR_CHECK(err_code);

	err_code = bsp_event_to_button_action_assign(JEEGYSHOW_KEY1, BSP_BUTTON_ACTION_LONG_PUSH, BSP_EVENT_KEY_1_LONG);
	APP_ERROR_CHECK(err_code);
	return err_code;
}



/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        case BSP_EVENT_KEY_0_SHORT:		
			Uart_Printf("key:KEY_0_SHORT\r\n");	
			break;

		case BSP_EVENT_KEY_0_LONG:
			Uart_Printf("key:KEY_0_LONG\r\n");	
            //stop_one_second_timer();
            start_long_push_power_off_timer();
            stop_ms100_LED_off_timer();
			break;
			
		case BSP_EVENT_KEY_1_SHORT:	
			Uart_Printf("key:KEY_1_SHORT\r\n");						
			break;

		case BSP_EVENT_KEY_1_LONG:	
			Uart_Printf("key:KEY_1_LONG\r\n");								
			break;

        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    unsigned char ubRxData;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            app_uart_get(&ubRxData);
			JeegyUart_Handler(ubRxData);
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

void JeegyUart_Handler(unsigned char ubRxData)
{
	int idx;
	char bDisCmd[64];
	if (ubMsgCnt == MAX_DEBUG_MSG)
	{
		ubMsgCnt = 0;
		memset(ubDebugMsg, 0, MAX_DEBUG_MSG);
		Uart_Printf("MSG_ERR: Too Many Msg..!\r\n");
	}

	ubDebugMsg[ubMsgCnt] = ubRxData;
	
	app_uart_put(ubRxData);
	
	if(ubRxData == 0x08)
	{
		for(idx=0;idx<(ubMsgCnt+8);idx++)
			app_uart_put(0x08);
				
		ubDebugMsg[ubMsgCnt] = 0;

		if(ubMsgCnt>0)
			ubMsgCnt--;

		ubDebugMsg[ubMsgCnt] = 0;

		sprintf(bDisCmd, "%s%s", MCU_PROMPT, ubDebugMsg);
		print_JeegyString(bDisCmd);
		
		return;			
	}

	if(ubMsgCnt>=2)
	{
		if((ubDebugMsg[ubMsgCnt-2]==0x1b) && (ubDebugMsg[ubMsgCnt-1]==0x5b) && (ubDebugMsg[ubMsgCnt]==0x41))
		{
			memcpy(ubDebugMsg, ubDebugOld, MAX_DEBUG_MSG);	
			ubMsgCnt = strlen((char const*)ubDebugMsg);		
			
			sprintf(bDisCmd, "\r\n%s%s", MCU_PROMPT, ubDebugMsg);
			print_JeegyString(bDisCmd);
			//Uart_Printf("\r\n%s%s", MCU_PROMPT, ubDebugMsg);
			return;
		}
	}
		
	if ((ubRxData == '\r') || (ubRxData == '\n'))
	{	
		ubDebugMsg[ubMsgCnt] = 0;

		if(ubMsgCnt != 0)
			memcpy(ubDebugOld, ubDebugMsg, MAX_DEBUG_MSG);	
			
			
		ubMsgCnt = 0;				
		ubGotDbgMsg = 1;
	}
	else
		ubMsgCnt++;		
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
#if defined (UART_PRESENT)
        .baud_rate    = NRF_UART_BAUDRATE_115200
#else
        .baud_rate    = NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);

    ubJeegyShowUartEnabled = 1;
}

void init_JeegyUart(void)
{
	uart_init();
	ubMsgCnt = 0;				
	ubGotDbgMsg = 0;
	memset(ubDebugMsg, 0, MAX_DEBUG_MSG);
}

void print_JeegyString(char *pStr)
{
	while(*pStr != 0)
		app_uart_put(*pStr++);
}

void Uart_Printf(const char* fmt,...)
{  
	va_list ap;
	char string[256];

	if(ubJeegyShowUartEnabled == 0)
		return;
	
	va_start(ap,fmt);
	vsprintf(string,(const char*)fmt,ap);
	print_JeegyString((char*)string);
	va_end(ap);
}


/**@snippet [UART Initialization] */

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    //switch (p_evt->evt_id)
    //{
        //case PM_EVT_PEERS_DELETE_SUCCEEDED:
            //advertising_start();
            //break;

        //default:
            //break;
    //}
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            Uart_Printf("PM_EVT_BONDED_PEER_CONNECTED\r\n");
            //NRF_LOG_INFO("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            Uart_Printf("PPM_EVT_CONN_SEC_SUCCEEDED\r\n");
            /*NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);*/
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            Uart_Printf("PM_EVT_CONN_SEC_FAILED\r\n");
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            Uart_Printf("PM_EVT_CONN_SEC_CONFIG_REQ\r\n");
            // Reject pairing request from an already bonded peer.
            //pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
            //pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            Uart_Printf("PM_EVT_STORAGE_FULL\r\n");
            // Run garbage collection on the flash.
            //err_code = fds_gc();
            //if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            //{
                // Retry.
            //}
            //else
            //{
            //    APP_ERROR_CHECK(err_code);
            //}
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            Uart_Printf("PM_EVT_PEERS_DELETE_SUCCEEDED\r\n");
            uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
            //advertising_start(true);
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            Uart_Printf("PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED\r\n");
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            Uart_Printf("PM_EVT_PEER_DATA_UPDATE_FAILED\r\n");
            //APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            Uart_Printf("PM_EVT_PEER_DELETE_FAILED\r\n");
            //APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            Uart_Printf("PM_EVT_PEERS_DELETE_FAILED\r\n");
            //APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            Uart_Printf("PM_EVT_ERROR_UNEXPECTED\r\n");
            //APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            Uart_Printf("default...\r\n");
            break;
    }
    
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

void send_nus_data(uint8_t *buff)
{
    ret_code_t err_code = NRF_SUCCESS;
	uint16_t length = 0;
	length = strlen((char *)buff);
	do
	{
            if(f_ble_connected == true){
                err_code = ble_nus_data_send(&m_nus, (char *)buff, &length, m_conn_handle);
                if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
                {
                        APP_ERROR_CHECK(err_code);
                }
            }
	} while (err_code == NRF_ERROR_RESOURCES);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

bool erase_bonds = true;
/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    if(erase_bonds == true){
        delete_bonds();
    }else{
        uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
    
}

static void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
	switch (p_event->type)
	{
		case NRF_DRV_TWI_EVT_DONE:
		{
			switch (p_event->xfer_desc.type)
			{
				case NRF_DRV_TWI_XFER_TX:
				case NRF_DRV_TWI_XFER_RX:
				{
					boTransferDone = true;
					break;
				}
				default:
					//Uart_Printf("unknown xfer_desc.type: %x\n", p_event->xfer_desc.type);
					break;
			}
			break;
		}
		default:
			//Uart_Printf("Unknown event type: %x\n", p_event->type);
			break;
	}
}

void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lis2dh_config = {
       .scl                = TWI_SCL,
       .sda                = TWI_SDA,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lis2dh_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

static void vWaitForEvent(void)
{
	do
	{
		__WFE();
	}
	while (! boTransferDone);
}

static void vSetSubAdd(uint8_t u8subAdd)
{
	boTransferDone = false;
	twi_err_code = nrf_drv_twi_tx(&m_twi, LIS2DH_ADD, &u8subAdd, 1, false);
	APP_ERROR_CHECK(twi_err_code);

	vWaitForEvent();
}

void vTWI_Read (uint8_t u8subAdd, uint8_t *pu8readData)
{
	vSetSubAdd(u8subAdd);
    
	boTransferDone = false; 
	twi_err_code = nrf_drv_twi_rx(&m_twi, LIS2DH_ADD, pu8readData, 1);
	APP_ERROR_CHECK(twi_err_code);

	vWaitForEvent();
}

void vTWI_Write (uint8_t u8address, uint8_t u8data)
{
	uint8_t au8addData[2];
	au8addData[0] = u8address;
	au8addData[1] = u8data;

	boTransferDone = false;
	twi_err_code = nrf_drv_twi_tx(&m_twi, LIS2DH_ADD, au8addData, sizeof(au8addData), false);
	APP_ERROR_CHECK(twi_err_code);

	vWaitForEvent();
}


uint8_t u8LIS2_TestRead(void)
{
	uint8_t u8whoAmI;
    vTWI_Read (WHO_AM_I, &u8whoAmI);
	return u8whoAmI;
}

static void vThresholdConfigure (uint8_t u8Level, uint8_t u8Duration, uint8_t u8Mode)
{
  // Enable X,Y,Z sensors and set a default sample rate
	vTWI_Write(CTRL_REG1, (ODR_10Hz | X_EN | Y_EN | Z_EN));
	
	// Enable high-pass filtering with highest cut-off frequency
	// and unfiltered samples to the data registers
	vTWI_Write(CTRL_REG2, (HPF_CUTOFF_F0 | HP_IA1));
	
	// Enable INT1 interrupts
	vTWI_Write(CTRL_REG3, I1_IA1);
	
	// Select measurement range to +/- 2g, 12-bit resolution
	vTWI_Write(CTRL_REG4, HIRES_MODE | FS_2G);
	//vTWI_Write(CTRL_REG4, FS_2G);
	eRsolution = mode_12bit;
	u8SignExtend = 4;

	// Set wake-up threshold level
	vTWI_Write(INT1_THS, u8Level);
	
	// Set duration that threshold needs to be held
	vTWI_Write(INT1_DURATION, u8Duration);
	
	// Enable interrupt on INT1 pin
	boInterruptEvent = false;
	vTWI_Write(CTRL_REG5, LIR_INT1);
	
	// Read reference register to force HP filters to current
	// acceleration/tilt value
	uint8_t u8dummy;
	vTWI_Read(REFERENCE_REG, &u8dummy);
	
	// Enable threshold event
	vTWI_Write(INT1_CFG, u8Mode);
}

void vLIS2_EnableWakeUpDetect (void)
{
	// Configure threshold detector for activity wake-up event
	vThresholdConfigure (THS_FS2G_32mg, 0, (XHIE | YHIE | ZHIE));
}


void vLIS2_PowerDown (void)
{
	// Set the ODR value to put device into power-down mode.
	vTWI_Write(CTRL_REG1, ODR_PDOWN);
}

static void vInterruptHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	// Register interrupt event 
	boInterruptEvent = true;
}

static void vInterruptInit (void)
{
	ret_code_t err_code;
	boInterruptEvent = false;
	
	if (nrf_drv_gpiote_is_init())
		Uart_Printf("nrf_drv_gpiote_init already installed\r\n");
	else
		nrf_drv_gpiote_init();

	// input pin, +ve edge interrupt, no pull-up
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	in_config.pull = NRF_GPIO_PIN_NOPULL;

	// Link this pin interrupt source to its interrupt handler
	err_code = nrf_drv_gpiote_in_init(READY_PIN, &in_config, vInterruptHandler);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(READY_PIN, true);
}

void vLIS2_Init (void)
{
	// If this function is called after power on, then the LIS2DH12
	// will still be in boot mode, allow time for boot to complete. 
	nrf_delay_ms(20);
	
	// Disable all interrupt sources
	vTWI_Write(INT1_CFG,  0);

	// Clear any pending interrupts
	uint8_t u8Reg;
	vTWI_Read(INT1_SRC, &u8Reg);	

	// Put device into power-down
	vLIS2_PowerDown();
	nrf_delay_ms(10);
	
	// Enable MCU interrupts from INT1 pin
	vInterruptInit();
}



void vLIS2_EnableInactivityDetect (void)
{
	// Configure threshold detector for inactivity event
	vThresholdConfigure (THS_FS2G_32mg, 30, (XLIE | YLIE | ZLIE));
}

bool boLIS2_InterruptOccurred (void)
{
	if (boInterruptEvent)
	{
		boInterruptEvent = false;
		return true;
	}
	
	return false;
}

uint8_t u8LIS2_EventStatus (void)
{
	uint8_t u8status;
	vTWI_Read(INT1_SRC, &u8status);

	return u8status;
}



/**@brief Function for handling the ADC interrupt.
 *
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
uint8_t           percentage_batt_lvl;
void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        nrf_saadc_value_t adc_result;
        uint16_t          batt_lvl_in_milli_volts;
        
        uint32_t          err_code;

        adc_result = p_event->data.done.p_buffer[0];

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);
        APP_ERROR_CHECK(err_code);

        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) +
                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;
        percentage_batt_lvl = battery_level_in_percent(batt_lvl_in_milli_volts);
        Uart_Printf("Battery: %d %d\r\n", percentage_batt_lvl, batt_lvl_in_milli_volts);
    }
}

/**@brief Function for configuring ADC to do battery level conversion.
 */
static void adc_configure(void)
{
   ret_code_t err_code = nrf_drv_saadc_init(NULL, saadc_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_saadc_channel_config_t config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
    err_code = nrf_drv_saadc_channel_init(0, &config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&adc_buf[0], 1);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&adc_buf[1], 1);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{

}



/**@brief Application main function.
 */
int main(void)
{
    ret_code_t err_code;
    bool erase_bonds;
    uint8_t u8Reading, u8LoData;
    short s16AccelX, s16AccelY, s16AccelZ; 
    uint8_t buff[10] = {0,};

    // Initialize.
#if (DEBUG_UART_ON	==1 )
	//init_JeegyUart();
    uart_init();
#endif
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);

    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();

    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    create_app_timer();
    peer_manager_init();

    adc_configure();

    bsp_board_led_on(0);
   
    // Start execution.
    Uart_Printf("\r\nUART started...\r\n");
    NRF_LOG_INFO("Debug logging for UART over RTT started.");

    init_ButtonEvent();
    
    init_speaker();
   
    twi_init();
    
    //start_battery_sense_timer();
    uint8_t u8ID = u8LIS2_TestRead();
    
	Uart_Printf("LIS2DH12 - Who am I code:%x\r\n", u8ID);

    vLIS2_Init();

    // enable wake up CPU through accel int 
    vLIS2_EnableWakeUpDetect();
    
    advertising_start();

    start_one_second_timer();
    start_threeSec_Power_on_timer();
    start_long_push_power_on_timer();

    tx_power_set();
    
    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
        
        if (boLIS2_InterruptOccurred())
		{
			uint8_t u8Status = u8LIS2_EventStatus();
			//Uart_Printf("accel event code: 0x%x\n", u8Status);
			if (u8Status & (XHIE | YHIE | ZHIE))
			{
				// Any combination of X,Y,Z motion passing
				// high threshold generates an interrupt
                //Uart_Printf("accel high threshold int! \n");
                vTWI_Read(OUT_X_LO, &u8Reading);
                u8LoData = u8Reading;
                vTWI_Read(OUT_X_HI, &u8Reading);
                s16AccelX = ( (u8Reading << 8) | u8LoData);

                vTWI_Read(OUT_Y_LO, &u8Reading);
                u8LoData = u8Reading;
                vTWI_Read(OUT_Y_HI, &u8Reading);
                s16AccelY = ( (u8Reading << 8) | u8LoData);
				
                vTWI_Read(OUT_Z_LO, &u8Reading);
                u8LoData = u8Reading;
                vTWI_Read(OUT_Z_HI, &u8Reading);
                s16AccelZ = ( (u8Reading << 8) | u8LoData);
                //Uart_Printf("accel: %d %d %d \n", (short)s16AccelX/64, (short)s16AccelY/64, (short)s16AccelZ/64);
                float fX = (float)(s16AccelX*s16AccelX);
                float fY = (float)(s16AccelY*s16AccelY);
                float fZ = (float)(s16AccelZ*s16AccelZ);
                //Uart_Printf("accle %d \n",(short)sqrtf(fX+fY+fZ));
                uint16_t s16ThreshValue = (short)sqrtf(fX+fY+fZ);

                switch(u8_accel_sense){
                    case SENSE_HIGH:
                    {
                        if(s16ThreshValue > 20000){
                            touch_detected();
                        }
                    }
                    break;
                    case SENSE_MIDDLE:
                    {
                        if(s16ThreshValue > 15000){
                            touch_detected();
                        }
                    }
                    break;
                    case SENSE_LOW:
                    {
                        if(s16ThreshValue > 10000){
                            touch_detected();
                        }
                    }
                    break;
                    default:
                    break;
                }        
			}
			else // Assume low threshold (plus delay) detected
			{
                Uart_Printf("accel low threshold int!\n");
				//vLIS2_EnableWakeUpDetect();
			}
		}else if(b_send_3_time_sense == true){
            switch(u8_accel_sense){
                case SENSE_MIDDLE:
                {
                    buff[0] = 'M';
                    buff[1] = SENSE_MIDDLE;
                    buff[2] = 0x00;
                    buff[3] = 0x00;
                    buff[4] = 0x00;
                }
                break;
                case SENSE_HIGH:
                {
                    buff[0] = 'H';
                    buff[1] = SENSE_HIGH;
                    buff[2] = 0x00;
                    buff[3] = 0x00;
                    buff[4] = 0x00;
                }
                break;
                case SENSE_LOW:
                {
                    buff[0] = 'L';
                    buff[1] = SENSE_LOW;
                    buff[2] = 0x00;
                    buff[3] = 0x00;
                    buff[4] = 0x00;
                }
                break;
            }
            send_nus_data(buff);
            b_send_3_time_sense = false;

        }else{
			// Put MCU into low-power mode until event occurs.
            nrf_drv_wdt_channel_feed(m_channel_id);
			__WFE();
		}
    }
}
/**
 * @}
 */
