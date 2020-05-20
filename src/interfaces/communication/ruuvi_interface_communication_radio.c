#include "ruuvi_interface_communication_radio.h"
#if RI_RADIO_ENABLED
#include <stdint.h>

uint8_t ri_radio_num_channels_get (const ri_radio_channels_t channels)
{
    return channels.channel_37 + channels.channel_38 + channels.channel_39;
}

#endif