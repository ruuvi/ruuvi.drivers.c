#include "unity.h"

#include "ruuvi_task_flash.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_interface_flash.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_power.h"
#include "mock_ruuvi_interface_yield.h"

void setUp (void)
{
    ri_log_Ignore();
    ri_error_to_string_IgnoreAndReturn (0);
}

void tearDown (void)
{
}

/**
 * @brief Initialize flash storage.
 *
 * If flash initialization fails, flash is purged and device tries to enter bootloader.
 *
 * @return RD_SUCCESS on success
 * @return error code from stack on error
 * @warning Erases entire flash storage and reboots on failure.
 */
void test_rt_flash_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_flash_init_ExpectAndReturn (RD_SUCCESS);
    ri_flash_record_get_ExpectAnyArgsAndReturn (RD_ERROR_NOT_FOUND);
    err_code |= rt_flash_init();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_flash_init_error (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_flash_init_ExpectAndReturn (RD_ERROR_INTERNAL);
    ri_flash_purge_Expect();
    ri_power_reset_Expect();
    (void) rt_flash_init();
}

/**
 * @brief Store data to flash.
 *
 * The flash storage implements a simple file system, where data is arranged to files which have records.
 * You can for example have a file for sensor configurations and record for each sensor. Underlying implementation
 * provides wear leveling. Garbage collection may be triggered manually and is tried automatically if there is not enough space in flash.
 * This function only queues the data to be written, you must verify that write was completed before freeing message.
 *
 * @param[in] file_id ID of a file to store. Valid range 1 ... 0xBFFF
 * @param[in] record_id ID of a record to store. Valid range 1 ... 0x
 * @param[in] message Data to store. Must be aligned to a 4-byte boundary.
 * @param[in] message_length Length of stored data. Maximum 4000 bytes per record on nRF52.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_NULL if data pointer is NULL.
 * @return RD_ERROR_INVALID_STATE if flash is not initialized.
 * @return RD_ERROR_BUSY if another operation was ongoing.
 * @return RD_ERROR_NO_MEM if there was no space for the record in flash.
 * @return RD_ERROR_DATA_SIZE if record exceeds maximum size.
 *
 * @warning triggers garbage collection if there is no space available, which leads to long processing time.
 */
void test_rt_flash_store_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_flash_record_set_ExpectAndReturn (0xABU, 0xCDU, sizeof ("Message"), "Message",
                                         RD_SUCCESS);
    err_code |= rt_flash_store (0xABU, 0xCDU, "Message", sizeof ("Message"));
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_flash_store_gc_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_flash_record_set_ExpectAndReturn (0xABU, 0xCDU, sizeof ("Message"), "Message",
                                         RD_ERROR_NO_MEM);
    ri_flash_gc_run_ExpectAndReturn (RD_SUCCESS);
    ri_flash_is_busy_ExpectAndReturn (true);
    ri_yield_ExpectAndReturn (RD_SUCCESS);
    ri_flash_is_busy_ExpectAndReturn (false);
    ri_flash_record_set_ExpectAndReturn (0xABU, 0xCDU, sizeof ("Message"), "Message",
                                         RD_SUCCESS);
    err_code |= rt_flash_store (0xABU, 0xCDU, "Message", sizeof ("Message"));
    TEST_ASSERT (RD_SUCCESS == err_code);
}

/**
 * @brief Load data from flash.
 *
 * The flash storage implements a simple file system, where data is arranged to files which have records.
 * You can for example have a file for sensor configurations and record for each sensor.
 *
 * @param[in] file_id ID of a file to load. Valid range 1 ... 0xBFFF
 * @param[in] record_id ID of a record to load. Valid range 1 ... 0x
 * @param[in] message Data to load. Must be aligned to a 4-byte boundary.
 * @param[in] message_length Length of loaded data. Maximum 4000 bytes per record on nRF52.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_NULL if data pointer is NULL.
 * @return RD_ERROR_INVALID_STATE if flash is not initialized.
 * @return RD_ERROR_BUSY if another operation was ongoing.
 * @return RD_ERROR_NOT_FOUND if given record was not found.
 * @return RD_ERROR_DATA_SIZE if record exceeds maximum size.
 *
 * @warning triggers garbage collection if there is no space available, which leads to long processing time.
 */
void test_rt_flash_load_ok (void)
{
    // Tested by integration test
    uint8_t message[128];
    ri_flash_record_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rt_flash_load (0xABU, 0xCDU, message, 128U);
}

/**
 * @brief Free data from flash.
 *
 * The flash storage implements a simple file system, where data is arranged to files which have records.
 * You can for example have a file for sensor configurations and record for each sensor.
 *
 * This function does not physically erase the data, it only marks the record as deleteable for garbage collection.
 *
 * @param[in] file_id ID of a file to delete. Valid range 1 ... 0xBFFF
 * @param[in] record_id ID of a record to delete. Valid range 1 ... 0x
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_INVALID_STATE if flash is not initialized.
 * @return RD_ERROR_BUSY if another operation was ongoing.
 * @return RD_ERROR_NOT_FOUND if given record was not found.
 *
 * @warning triggers garbage collection if there is no space available, which leads to long processing time.
 */
void test_rt_flash_free_ok (void)
{
    // Tested by integration test
    uint8_t message[128];
    ri_flash_record_delete_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rt_flash_free (0xABU, 0xCDU);
}

/**
 * @brief Trigger garbage collection
 *
 * Free up space in flash by erasing old records. It's generally a good idea to check remaining
 * space on flash after a write and trigger GC if the space remaining is smaller than expected record sizes.
 * This function physically erases the records which are garbage collected.
 * Waits until GC can be triggered, and returns when GC operation has been queued.
 *
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_INVALID_STATE if flash is not initialized.
 *
 */
void test_rt_flash_gc_ok (void)
{
    // Tested by integration test
    ri_flash_gc_run_ExpectAndReturn (RD_SUCCESS);
    rt_flash_gc_run();
}

void test_print_error_cause_ok (void)
{
    ri_flash_record_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    print_error_cause();
}