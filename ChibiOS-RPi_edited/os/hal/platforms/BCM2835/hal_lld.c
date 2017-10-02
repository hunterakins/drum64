/*
    ChibiOS/RT - Copyright (C) 1606,1607,1608,1609,1610,
                 1611,1612 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    templates/hal_lld.c
 * @brief   HAL Driver subsystem low level driver source template.
 *
 * @addtogroup HAL
 * @{
 */

#include "ch.h"
#include "hal.h"

// HACK
#define GPEDS0 REG(0x20200040)

volatile uint32_t incrementer = 0;

volatile int times[64] = {0};

volatile uint32_t current_time = 0;
volatile uint32_t old_time = 0;
volatile uint32_t indexer = 0;

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief Start the system timer
 *
 * @notapi
 */
static void systimer_init( void )
{

  // 1 MHz clock, Counter=1000, 1 ms tick
  // ticks
  ARM_TIMER_CTL = 0x003E0000;
  ARM_TIMER_LOD = 1000-1;
  ARM_TIMER_RLD = 1000-1;
  // HACK so that ARM_TIMER_VAL decrements every 0.5 us instead of 1us ...?
  //ARM_TIMER_DIV = 0x000000F9; // = 249. ticks at 1MHz.
  ARM_TIMER_DIV = 124; // ticks at 2 MHz now ...


  ARM_TIMER_CLI = 0;
  ARM_TIMER_CTL = 0x003E00A2;

  IRQ_ENABLE_BASIC |= 1;
}

/**
 * @brief Process system timer interrupts, if present.
 *
 * @notapi
 */
static void systimer_serve_interrupt( void )
{
  // Update the system time
  chSysLockFromIsr();
  chSysTimerHandlerI();
  chSysUnlockFromIsr();

  // Clear timer interrupt
  ARM_TIMER_CLI = 0;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief Interrupt handler
 *
 */
CH_IRQ_HANDLER(IrqHandler)
{
  CH_IRQ_PROLOGUE();


  current_time = SYSTIMER_CLO;

  // HACK!! // if interrupt is from GPIO pin 26, do a LOT
  if (GPEDS0 == (1 << 26)) { // if interrupt is from GPIO pin 26
    GPEDS0 = (1 << 26); // This resets the event interrupt
    incrementer++; // increment incrementer
    indexer++;     // increment indexer

    if ((current_time - old_time > 1000) | (indexer == 64)) {
      indexer = 0; //reset indexer to 0 if current_time - old_time is too long
    }

    // add value to times array 
    times[indexer] = ARM_TIMER_VAL; // incrementing at 2MHz (see systimer_init)

    old_time = current_time;
  }

  else {
  // HACK
  systimer_serve_interrupt();


  #if HAL_USE_SERIAL
    sd_lld_serve_interrupt(&SD1);
  #endif

  #if HAL_USE_I2C
    i2c_lld_serve_interrupt(&I2C0);
  #endif

  #if HAL_USE_SPI
    spi_lld_serve_interrupt(&SPI0);
  #endif

  #if HAL_USE_GPT
    gpt_lld_serve_interrupt();
  #endif

  }



  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief Synchronize function for short delays.
 *
 */
void delayMicroseconds(uint32_t n)
{
  uint32_t compare = SYSTIMER_CLO + n;
  while (SYSTIMER_CLO < compare);
}

/**
 * @brief Returns incrementer
 *
 */
uint32_t read_incrementer ( void )
{
  return incrementer;
}

/**
 * @brief copies time to array
 *
 */
void read_times ( uint32_t *buf )
{
  for (int i = 0; i < 64; i++) {
    buf[i] = times[i];
    //buf[i] = i;
  }

  return;
}


/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {
  systimer_init();
}

/**
 * @brief Start watchdog timer
 */
void watchdog_start ( uint32_t timeout )
{
    /* Setup watchdog for reset */
    uint32_t pm_rstc = PM_RSTC;

    //* watchdog timer = timer clock / 16; need password (31:16) + value (11:0) */
    uint32_t pm_wdog = PM_PASSWORD | (timeout & PM_WDOG_TIME_SET);
    pm_rstc = PM_PASSWORD | (pm_rstc & PM_RSTC_WRCFG_CLR) | PM_RSTC_WRCFG_FULL_RESET;
    PM_WDOG = pm_wdog;
    PM_RSTC = pm_rstc;
}

/**
 * @brief Start watchdog timer
 */
void watchdog_stop ( void )
{
  PM_RSTC = PM_PASSWORD | PM_RSTC_RESET;
}

/**
 * @brief Get remaining watchdog time.
 */
uint32_t watchdog_get_remaining ( void )
{
  return PM_WDOG & PM_WDOG_TIME_SET;
}

/** @} */
