#ifndef LECTOR_LOG_HPP
#define LECTOR_LOG_HPP

#include <syslog.h>
#define LECTOR_DEBUG

#define LECTOR_LOG_ERROR(msg, args...) \
    syslog(LOG_ERR, msg, ##args)

#ifdef LECTOR_DEBUG

#define LECTOR_LOG(msg, args...) \
    syslog(LOG_INFO, msg, ##args)

namespace lector
{
    namespace debug
    {
        struct log_guard
        {
            log_guard(const char* start, const char* end) : end_(end)
            {
                LECTOR_LOG("%s", start);
            }
            
            ~log_guard()
            {
                LECTOR_LOG("%s", end_);
            }

            const char* end_;
        };

        struct log_func_guard
        {
            log_func_guard(const char* name) : name_(name)
            {
                LECTOR_LOG("Entering %s", name);
            }
            
            ~log_func_guard()
            {
                LECTOR_LOG("Exiting %s", name_);
            }
            
            const char* name_;
        };
    }
}

#define LECTOR_LOG_GUARD(begin, end) \
    ::lector::debug::log_guard log_guard_##__FILE__##__LINE__(begin, end)

#define LECTOR_LOG_FUNC \
    ::lector::debug::log_func_guard log_func_guard_##__FILE__##__LINE__(__func__)

#else
#define LECTOR_LOG(msg, args...)
#define LECTOR_LOG_GUARD(msg)
#define LECTOR_LOG_FUNC
#endif // LECTOR_DEBUG


#endif
