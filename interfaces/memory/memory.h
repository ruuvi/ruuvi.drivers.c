#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include "ruuvi_error.h"

typedef ruuvi_status_t (*transfer_blocking_fp)(const uint8_t, uint8_t* const, const size_t, uint8_t**, size_t*, bool);
#endif