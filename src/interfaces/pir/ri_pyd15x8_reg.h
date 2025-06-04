/*
 * pyd15x8_reg.h – Register definitions and low‑level bit‑bang helpers
 * for Excelitas PYD15x8 low‑power DigiPyro™ family.
 *
 * Copyright (c) 2025 Ruuvi Innovations Ltd
 * SPDX‑License‑Identifier: BSD‑3‑Clause
 */
#ifndef PYD15X8_REG_H
#define PYD15X8_REG_H

#include <stdint.h>
#include <stdbool.h>

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_yield.h"

/* =========================================================
 * Configuration‑register bit layout (25 bits, MSB‑first)
 * ========================================================= */

#define PYD15X8_CFG_THRESHOLD_POS        17U
#define PYD15X8_CFG_THRESHOLD_MASK       (0xFFU << PYD15X8_CFG_THRESHOLD_POS)

#define PYD15X8_CFG_BLIND_POS            13U
#define PYD15X8_CFG_BLIND_MASK           (0x0FU << PYD15X8_CFG_BLIND_POS)   /* +0.5 s steps */

#define PYD15X8_CFG_PULSES_POS           11U
#define PYD15X8_CFG_PULSES_MASK          (0x03U << PYD15X8_CFG_PULSES_POS)  /* 0→1 … 3→4 */

#define PYD15X8_CFG_WINDOW_POS           9U
#define PYD15X8_CFG_WINDOW_MASK          (0x03U << PYD15X8_CFG_WINDOW_POS)  /* 0→2 … 3→8 s */

#define PYD15X8_CFG_MODE_POS             7U
#define PYD15X8_CFG_MODE_MASK            (0x03U << PYD15X8_CFG_MODE_POS)

#define PYD15X8_CFG_SRC_POS              5U
#define PYD15X8_CFG_SRC_MASK             (0x03U << PYD15X8_CFG_SRC_POS)

#define PYD15X8_CFG_RSVD_POS             3U
#define PYD15X8_CFG_RSVD_PATTERN         (0x02U << PYD15X8_CFG_RSVD_POS)    /* Must be “10” */
#define PYD15X8_CFG_RSVD_MASK            (0x03U << PYD15X8_CFG_RSVD_POS)   /* bits [4:3] */

#define PYD15X8_CFG_HPF_POS              2U
#define PYD15X8_CFG_HPF_MASK             (0x01U << PYD15X8_CFG_HPF_POS)

#define PYD15X8_CFG_COUNTMODE_POS        0U
#define PYD15X8_CFG_COUNTMODE_MASK       0x01U

/* ---------- Enumerations for field values ---------- */

typedef enum
{
    PYD15X8_MODE_FORCED      = 0,  /*!< Host initiates each read‑out.               */
    PYD15X8_MODE_INTERRUPT   = 1,  /*!< Sensor toggles DIRECT LINK periodically.    */
    PYD15X8_MODE_WAKEUP      = 2   /*!< Motion detection pulls DIRECT LINK high.    */
} pyd15x8_mode_t;

typedef enum
{
    PYD15X8_SRC_PIR_BPF      = 0,  /*!< Band‑pass filtered PIR signal (signed). */
    PYD15X8_SRC_PIR_LPF      = 1,  /*!< Low‑pass filtered PIR signal (unsigned). */
    PYD15X8_SRC_TEMP         = 3   /*!< Internal temperature sensor (unsigned). */
} pyd15x8_source_t;

typedef enum
{
    PYD15X8_HPF_0_4_HZ       = 0,  /*!< 0.4 Hz high‑pass cut‑off. */
    PYD15X8_HPF_0_2_HZ       = 1   /*!< 0.2 Hz high‑pass cut‑off. */
} pyd15x8_hpf_t;

/* =========================================================
 * Helper macros – encode/decode fields
 * ========================================================= */

#define PYD15X8_ENCODE_FIELD(value, mask, pos)   (((uint32_t)(value) & ((mask) >> (pos))) << (pos))
#define PYD15X8_DECODE_FIELD(reg,   mask, pos)   (((reg) & (mask)) >> (pos))

/* Build a full 25‑bit configuration word from individual fields.                        */
static inline uint32_t pyd15x8_build_cfg(const uint8_t  threshold,
                                         const uint8_t  blind_time,
                                         const uint8_t  pulses,
                                         const uint8_t  window_time,
                                         const pyd15x8_mode_t mode,
                                         const pyd15x8_source_t src,
                                         const pyd15x8_hpf_t hpf,
                                         const bool     count_no_sign)
{
    uint32_t cfg = 0;
    cfg |= PYD15X8_ENCODE_FIELD(threshold,   PYD15X8_CFG_THRESHOLD_MASK,  PYD15X8_CFG_THRESHOLD_POS);
    cfg |= PYD15X8_ENCODE_FIELD(blind_time,  PYD15X8_CFG_BLIND_MASK,      PYD15X8_CFG_BLIND_POS);
    cfg |= PYD15X8_ENCODE_FIELD(pulses,      PYD15X8_CFG_PULSES_MASK,     PYD15X8_CFG_PULSES_POS);
    cfg |= PYD15X8_ENCODE_FIELD(window_time, PYD15X8_CFG_WINDOW_MASK,     PYD15X8_CFG_WINDOW_POS);
    cfg |= PYD15X8_ENCODE_FIELD(mode,        PYD15X8_CFG_MODE_MASK,       PYD15X8_CFG_MODE_POS);
    cfg |= PYD15X8_ENCODE_FIELD(src,         PYD15X8_CFG_SRC_MASK,        PYD15X8_CFG_SRC_POS);
    cfg |= PYD15X8_CFG_RSVD_PATTERN;                                         /* Bits [4:3] = 10b */
    cfg |= PYD15X8_ENCODE_FIELD(hpf,         PYD15X8_CFG_HPF_MASK,        PYD15X8_CFG_HPF_POS);
    if (count_no_sign) { cfg |= PYD15X8_CFG_COUNTMODE_MASK; }
    return cfg;
}

/* =========================================================
 * Bit‑bang timings
 * ========================================================= */

#define PYD15X8_TSL_US        1U     /* Clock LOW  (min 0.2 µs max 2.0 µs)  */
#define PYD15X8_TSH_US        1U     /* Clock HIGH (min 0.2 µs max 2.0 µs)  */
#define PYD15X8_TSHD_US       100U   /* Data hold  (min 80 µs)   */
#define PYD15X8_TSLT_US       700U   /* Latching time after 25 b */
#define PYD15X8_TDS_US        120U   /* DIRECT LINK setup time   */
#define PYD15X8_TBIT_US       10U    /* Max bit‑time for read‑out TSL+TSH+TBIT < 22 µs*/
#define PYD15X8_TUP_US        1300U  /* Update time after read   */

/* =========================================================
 * One‑wire pin bundle descriptor
 * ========================================================= */

typedef struct
{
    ri_gpio_id_t pin_serin;   /*!< Serial‑in configuration line.  */
    ri_gpio_id_t pin_dl;      /*!< DIRECT LINK bidirectional line.*/
} pyd15x8_bus_t;

/* =========================================================
 * Low‑level helpers – tiny, header‑only to avoid extra .c
 * ========================================================= */

/**
 * @brief   Write a 25‑bit configuration word over the SERIN pin.
 * @warning Make sure GPIO is initialised *and* pin_dl is kept LOW during transfer.
 */
static inline rd_status_t pyd15x8_serin_write(const pyd15x8_bus_t * bus,
                                             uint32_t cfg)
{
    if (NULL == bus) { return RD_ERROR_NULL; }
    if (!ri_gpio_is_init()) { return RD_ERROR_INVALID_STATE; }
    /* SERIN as output, DIRECT LINK as output‑low to satisfy datasheet. */
    rd_status_t err = RD_SUCCESS;
    err |= ri_gpio_configure(bus->pin_serin, RI_GPIO_MODE_OUTPUT_STANDARD);
    err |= ri_gpio_configure(bus->pin_dl,    RI_GPIO_MODE_OUTPUT_STANDARD);
    err |= ri_gpio_write    (bus->pin_dl,    RI_GPIO_LOW);
    ri_delay_us(PYD15X8_TSLT_US); /* ensure line idle */

    for (int8_t bit = 24; bit >= 0; --bit)
    {
        /* Clock LOW */
        err |= ri_gpio_write(bus->pin_serin, RI_GPIO_LOW);
        ri_delay_us(PYD15X8_TSL_US);

        /* Rising edge */
        err |= ri_gpio_write(bus->pin_serin, RI_GPIO_HIGH);
        ri_delay_us(PYD15X8_TSH_US);

        /* Drive data level */
        const bool one = ((cfg >> bit) & 0x1U) != 0U;
        err |= ri_gpio_write(bus->pin_serin, one ? RI_GPIO_HIGH : RI_GPIO_LOW);
        ri_delay_us(PYD15X8_TSHD_US);
    }

    /* Final LOW & latch   */
    err |= ri_gpio_write(bus->pin_serin, RI_GPIO_LOW);
    ri_delay_us(PYD15X8_TSLT_US);
    err |= ri_gpio_configure(bus->pin_dl, RI_GPIO_MODE_INPUT_NOPULL);
    return err;
}

/**
 * @brief Initiate a forced readout sequence and collect the 40‑bit frame.
 * @note  The frame layout is [39] status | [38:25] ADC | [24:0] configuration.
 *        Caller is responsible for decoding using masks above.
 */
static inline rd_status_t pyd15x8_forced_read(const pyd15x8_bus_t * bus,
                                             uint64_t * frame)
{
    if (NULL == bus || NULL == frame) { return RD_ERROR_NULL; }
    if (!ri_gpio_is_init()) { return RD_ERROR_INVALID_STATE; }

    rd_status_t err = RD_SUCCESS;
    /* Host drives DIRECT LINK HIGH for ≥ tDS, then releases (high‑Z) */
    err |= ri_gpio_configure(bus->pin_dl, RI_GPIO_MODE_OUTPUT_STANDARD);
    err |= ri_gpio_write    (bus->pin_dl, RI_GPIO_HIGH);
    ri_delay_us(PYD15X8_TDS_US);

    /* Collect 40 bits, MSB first */
    uint64_t data = 0U;
    for (int8_t bit = 39; bit >= 0; --bit)
    {
        /* Host pulls LOW (output) for >0.2 and < 2.0 µs */
        err |= ri_gpio_write    (bus->pin_dl, RI_GPIO_LOW); // Write low first to avoid glitching
        err |= ri_gpio_configure(bus->pin_dl, RI_GPIO_MODE_OUTPUT_STANDARD);
        ri_delay_us(PYD15X8_TSL_US);

        /* Host drives HIGH (output) for >0.2 and < 2.0 µs */
        err |= ri_gpio_write    (bus->pin_dl, RI_GPIO_HIGH);
        ri_delay_us(PYD15X8_TSH_US);

        /* Release and wait */
        err |= ri_gpio_configure(bus->pin_dl, RI_GPIO_MODE_INPUT_NOPULL);
        ri_delay_us(PYD15X8_TBIT_US);

        /* Sample */
        ri_gpio_state_t level = RI_GPIO_LOW;
        err |= ri_gpio_read(bus->pin_dl, &level);
        if (RI_GPIO_HIGH == level) { data |= ((uint64_t)1U << bit); }
    }

    /* Terminate frame – drive LOW for tUP */
    err |= ri_gpio_write    (bus->pin_dl, RI_GPIO_LOW);
    ri_delay_us(PYD15X8_TUP_US);

    *frame = data;
    return err;
}

/**
 * @brief Clear a motion interrupt in WAKE‑UP mode.
 */
static inline rd_status_t pyd15x8_clear_interrupt(const pyd15x8_bus_t * bus)
{
    if (NULL == bus) { return RD_ERROR_NULL; }
    if (!ri_gpio_is_init()) { return RD_ERROR_INVALID_STATE; }

    rd_status_t err = RD_SUCCESS;
    err |= ri_gpio_configure(bus->pin_dl, RI_GPIO_MODE_OUTPUT_STANDARD);
    err |= ri_gpio_write    (bus->pin_dl, RI_GPIO_LOW);
    ri_delay_us(200U); /* ≥160 µs */

    /* Release back to input */
    err |= ri_gpio_configure(bus->pin_dl, RI_GPIO_MODE_INPUT_NOPULL);
    return err;
}

#endif /* PYD15X8_REG_H */