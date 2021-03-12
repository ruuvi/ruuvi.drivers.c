#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_communication_uart.h"
#if RUUVI_NRF5_SDK15_UART_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_nrf5_sdk15_gpio.h"
#include "nrf_serial.h"

#ifndef RUUVI_NRF5_SDK15_UART_LOG_LEVEL
#define LOG_LEVEL RI_LOG_LEVEL_INFO
#else
#define LOG_LEVEL RUUVI_NRF5_SDK15_UART_LOG_LEVEL
#endif
#define LOG(msg)  (ri_log(LOG_LEVEL, msg))
#define LOGD(msg)  (ri_log(RI_LOG_LEVEL_DEBUG, msg))

static const ri_comm_channel_t * m_channel; //!< Pointer to application control structure.
static uint16_t m_rxcnt = 0; //!< Counter of received bytes after last read.
static uint16_t m_txcnt = 0; //!< Counter of bytes to send before tx complete.

static void sleep_handler (void)
{
    ri_yield();
}

// Handle UART events
static void uart_handler (struct nrf_serial_s const * p_serial, nrf_serial_event_t event)
{
    if (m_channel && m_channel->on_evt)
    {
        switch (event)
        {
            case NRF_SERIAL_EVENT_TX_DONE: ///< Requested TX transfer completed.
                LOGD ("TX\r\n");

                if (0 == (--m_txcnt))
                {
                    m_channel->on_evt (RI_COMM_SENT, NULL, 0);
                }

                break;

            case NRF_SERIAL_EVENT_RX_DATA: ///< Requested RX transfer completed.
                LOGD ("RX\r\n");

                if ( ( ( (char *) (p_serial->p_ctx->p_config->p_buffers->p_rxb)) [0] == '\n')
                        || ++m_rxcnt >= RI_COMM_MESSAGE_MAX_LENGTH)
                {
                    ri_comm_message_t msg = {0};
                    msg.data_length = RI_COMM_MESSAGE_MAX_LENGTH;
                    m_channel->read (&msg);
                    m_channel->on_evt (RI_COMM_RECEIVED, (void *) &msg.data[0], msg.data_length);
                }

                break;

            case NRF_SERIAL_EVENT_DRV_ERR:   ///< Error reported by UART peripheral.
                LOGD ("UART Error\r\n");
                break;

            case NRF_SERIAL_EVENT_FIFO_ERR:
                LOGD ("FIFO Error\r\n");
                break;

            default:
                LOGD ("UART unknown event\r\n");
                break;
        }
    }
}

static nrf_drv_uart_config_t m_uart0_drv_config;

// FIFOs have a guard byte
#define GUARD_SIZE (1U)
#define SERIAL_FIFO_TX_SIZE (RI_COMM_MESSAGE_MAX_LENGTH + GUARD_SIZE)
#define SERIAL_FIFO_RX_SIZE (RI_COMM_MESSAGE_MAX_LENGTH + GUARD_SIZE)

NRF_SERIAL_QUEUES_DEF (serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);


#define SERIAL_BUFF_TX_SIZE (1U)
#define SERIAL_BUFF_RX_SIZE (1U)

NRF_SERIAL_BUFFERS_DEF (serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_CONFIG_DEF (serial_config, NRF_SERIAL_MODE_DMA,
                       &serial_queues, &serial_buffs, uart_handler, sleep_handler);


NRF_SERIAL_UART_DEF (serial_uart, 0);

// Convert constants from Ruuvi to nRF52 SDK
static nrf_uarte_baudrate_t ruuvi_to_nrf_baudrate (const ri_uart_baudrate_t baud)
{
    switch (baud)
    {
        case RI_UART_BAUD_9600:
            return NRF_UARTE_BAUDRATE_9600;

        case RI_UART_BAUD_115200:
        default:
            return NRF_UARTE_BAUDRATE_115200;
    }
}

static rd_status_t ri_uart_send_async (ri_comm_message_t * const msg)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == msg)
    {
        err_code |= RD_ERROR_NULL;
    }

    if (1 < msg->repeat_count)
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }
    else if (0 != m_txcnt)
    {
        if (NRF_MTX_LOCKED == serial_uart.p_ctx->write_lock)
        {
            err_code |= RD_ERROR_BUSY;
        }
        else
        {
            m_txcnt = 0;
            m_channel->on_evt (RI_COMM_SENT, NULL, 0);
        }
    }
    else
    {
        nrfx_err_t status = NRF_SUCCESS;
        size_t written = 0;
        status |= nrf_serial_write (&serial_uart, msg->data, msg->data_length, &written, 0);
        m_txcnt = written;
        err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (status);

        if (written != msg->data_length)
        {
            err_code |= RD_ERROR_NO_MEM;
        }
    }

    return err_code;
}

static rd_status_t ri_uart_read_buf (ri_comm_message_t * const msg)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == msg)
    {
        err_code |= RD_ERROR_NULL;
    }

    if (1 < msg->repeat_count)
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }
    else
    {
        nrfx_err_t status = NRF_SUCCESS;
        size_t bytes_read = nrf_queue_utilization_get (&serial_queues_rxq);

        if (msg->data_length < bytes_read)
        {
            bytes_read = msg->data_length;
        }

        status |= nrf_queue_read (&serial_queues_rxq, msg->data, bytes_read);
        msg->data_length = bytes_read;
        msg->repeat_count = 1;
        err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (status);
        m_rxcnt = 0;
    }

    return err_code;
}

rd_status_t ri_uart_init (ri_comm_channel_t * const channel)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == channel)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (NULL != m_channel)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        channel->send = ri_uart_send_async;
        channel->read = ri_uart_read_buf;
        channel->init = ri_uart_init;
        channel->uninit = ri_uart_uninit;
        m_channel = channel;
    }

    return err_code;
}

rd_status_t ri_uart_config (const ri_uart_init_t * const config)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == config)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        nrfx_err_t nrf_status = NRF_SUCCESS;
        m_uart0_drv_config.baudrate = ruuvi_to_nrf_baudrate (config->baud);
        m_uart0_drv_config.pseltxd = ruuvi_to_nrf_pin_map (config->tx);
        m_uart0_drv_config.pselrxd = ruuvi_to_nrf_pin_map (config->rx);

        if (config->hwfc_enabled)
        {
            m_uart0_drv_config.pselcts = ruuvi_to_nrf_pin_map (config->cts);
            m_uart0_drv_config.pselrts = ruuvi_to_nrf_pin_map (config->rts);
        }
        else
        {
            m_uart0_drv_config.pselcts = NRF_UARTE_PSEL_DISCONNECTED;
            m_uart0_drv_config.pselrts = NRF_UARTE_PSEL_DISCONNECTED;
        }

        m_uart0_drv_config.parity  = config->parity_enabled ? NRF_UARTE_PARITY_INCLUDED :
                                     NRF_UARTE_PARITY_EXCLUDED;
        m_uart0_drv_config.hwfc    = config->hwfc_enabled ? NRF_UARTE_HWFC_ENABLED :
                                     NRF_UARTE_HWFC_DISABLED;
        m_uart0_drv_config.interrupt_priority = APP_IRQ_PRIORITY_HIGH;
        nrf_status |= nrf_serial_init (&serial_uart, &m_uart0_drv_config, &serial_config);
        err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_status);
    }

    return err_code;
}

rd_status_t ri_uart_uninit (ri_comm_channel_t * const channel)
{
    m_channel = NULL;
    return nrf_serial_uninit (&serial_uart);
}

#endif