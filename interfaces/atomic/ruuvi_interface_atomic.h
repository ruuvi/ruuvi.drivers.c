#ifndef RUUVI_INTERFACE_ATOMIC_H
#define RUUVI_INTERFACE_ATOMIC_H

/**
 * @defgroup Atomic Atomic functions
 * @brief Functions for atomic operations. 
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_atomic.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-07-23
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic atomic operations
 *
 */

#define RUUVI_INTERFACE_ATOMIC_FLAG_INIT 0 //!< Initial value for atomic flag

typedef volatile uint32_t ruuvi_interface_atomic_t; //!< define atomic type - not portable to 8-bit.

/**
 * @brief atomic flag check and set/clear function
 *
 * Uses whatever mechanism underlying platform provides to check and set
 * or clear flag. Used to implement mutex, check-and-set flag to reserve
 *
 * Generally used like this:
 * \code{.c}
 * static volatile uint32_t 
 * if(!buffer->lock(&(buffer->readlock), true))  { return RUUVI_LIBRARY_ERROR_CONCURRENCY; }
 * do_some_critical_stuff();
 * if(!buffer->lock(&(buffer->readlock), false)) { return RUUVI_LIBRARY_ERROR_FATAL; }
 * \endcode
 *
 * It's important to return if lock can't be had rather than busylooping:
 * if the interrupt level which fails to get lock is higher than the call which has the lock
 * program will deadlock in the busyloop. Likewise failure to release the lock will
 * cause deadlock in the next execution, fail immediately.
 *
 * @param[in] flag uint32_t address of bitfield to check
 * @param[in] set true to set flag, false to clear flag.
 * @return    true if operation was successful
 */
bool ruuvi_interface_atomic_flag(ruuvi_interface_atomic_t* const flag, const bool set);

/*@}*/

#endif