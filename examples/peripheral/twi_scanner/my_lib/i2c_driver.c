

/***********************************************************************************************************************
 * History      : DD.MM.YYYY Version    Description
 *              : 22.03.2020   1.00     First Release
 *              : DD.MM.YYYY   x.00     First Release
 *              : DD.MM.YYYY   x.00     First Release
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Pragma directive
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes <System Includes>
 ***********************************************************************************************************************/
#include <stdint.h>

#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_twi.h"

/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/
/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID 0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID 1
#endif
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
#define SDA_PIN 7
#define SCL_PIN 5
#define MPU_TWI_TIMEOUT 40000
volatile static bool twi_tx_done = false;
volatile static bool twi_rx_done = false;



/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static void merge_register_and_data(uint8_t *new_buffer, uint8_t reg, const uint8_t *p_data, uint32_t length);

/**
 * @brief TWI events handler.
 */
static void twi_handler(nrf_drv_twi_evt_t const *p_event, void *p_context);
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name:
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
bool user_i2c_init(void)
{
    ret_code_t error_code = NRF_SUCCESS;
    const nrf_drv_twi_config_t twi_config = {
        .scl = SCL_PIN,
        .sda = SDA_PIN,
        .frequency = NRF_DRV_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init = false};

    if ((error_code = (int8_t)nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL)) == NRF_SUCCESS)
    {
        nrf_drv_twi_enable(&m_twi);
    }
    return (error_code == NRF_SUCCESS) ? true : false;
}
/***********************************************************************************************************************
 * Function Name:
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name:
 * Description  : user
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name:
 * Description  : user
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/

int8_t user_i2c_write(uint8_t device_address, uint8_t reg_addr, const uint8_t *reg_data, uint32_t len)
{
    uint8_t twi_tx_buffer[5];
    memset(twi_tx_buffer, 0, sizeof(twi_tx_buffer));
    uint32_t timeout = MPU_TWI_TIMEOUT;
    uint32_t err_code;

    merge_register_and_data(twi_tx_buffer, reg_addr, reg_data, len);

    err_code = nrf_drv_twi_tx(&m_twi, device_address, twi_tx_buffer, len + 1, false);
    // for (int i = 0; i < len + 1; i++)
    //{
    //     /* code */
    //     NRF_LOG_INFO("twi_tx_buffer[%i] = %x\r\n", i, twi_tx_buffer[i]);
    //     NRF_LOG_FLUSH();
    // }

    //if (err_code != NRF_SUCCESS)
    //    return -1;

    //while ((!twi_tx_done) && --timeout)
    //    ;
    //if (!timeout)
    //    return -1;

    twi_tx_done = false;
    return (err_code == NRF_SUCCESS) ? 0 : -1;
}

/***********************************************************************************************************************
 * static functions
 ***********************************************************************************************************************/
//  without repeating the ADDRESS + Write bit byte
// Hence we need to merge the register address with the buffer and then transmit all as one transmission
static void merge_register_and_data(uint8_t *new_buffer, uint8_t reg, const uint8_t *p_data, uint32_t length)
{
    new_buffer[0] = reg;
    memcpy((new_buffer + 1), p_data, length);
//     uint8_t j = 1;
//     for (int i = length - 1; i >= 0; i--)
//     {
//         new_buffer[j] = p_data[i];
//         j++;
//     }
}

/**
 * @brief TWI events handler.
 */
static void twi_handler(nrf_drv_twi_evt_t const *p_event, void *p_context)
{
    switch (p_event->type)
    {
    case NRF_DRV_TWI_EVT_DONE:
        switch (p_event->xfer_desc.type)
        {
        case NRF_DRV_TWI_XFER_TX:
            twi_tx_done = true;
            break;
        case NRF_DRV_TWI_XFER_TXTX:
            twi_tx_done = true;
            break;
        case NRF_DRV_TWI_XFER_RX:
            twi_rx_done = true;
            break;
        case NRF_DRV_TWI_XFER_TXRX:
            twi_rx_done = true;
            break;
        default:
            break;
        }
        break;
    case NRF_DRV_TWI_EVT_ADDRESS_NACK:
        break;
    case NRF_DRV_TWI_EVT_DATA_NACK:
        break;
    default:
        break;
    }
}

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
