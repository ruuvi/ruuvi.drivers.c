#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_log.h"
#if RUUVI_FRUITY_LOG_ENABLED
#include <Logger.h>

void ri_log (const ri_log_severity_t severity,
             const char * const message)
{
    //logs(message);
    Logger::getInstance().logTag_f(Logger::LogType::LOG_MESSAGE_ONLY, NULL, 0, "", message);
}
#endif