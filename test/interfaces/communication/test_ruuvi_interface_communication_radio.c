#include "unity.h"

#include "ruuvi_interface_communication_radio.h"

void setUp (void)
{
}

void tearDown (void)
{
}

void test_ruuvi_interface_communication_radio_getChannels (void)
{
    uint8_t num_ch = 0;
    ri_radio_channels_t channels = { 0 };
    num_ch = ri_radio_num_channels_get (channels);
    TEST_ASSERT (0 == num_ch);
    channels.channel_37 = 1;
    num_ch = ri_radio_num_channels_get (channels);
    TEST_ASSERT (1 == num_ch);
    channels.channel_39 = 1;
    num_ch = ri_radio_num_channels_get (channels);
    TEST_ASSERT (2 == num_ch);
    channels.channel_38 = 1;
    num_ch = ri_radio_num_channels_get (channels);
    TEST_ASSERT (3 == num_ch);
}
