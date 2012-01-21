#ifndef UTIL_CONCURRENT_QUEUE_HPP
#define UTIL_CONCURRENT_QUEUE_HPP

#include <boost/thread.hpp>
#include <queue>

#include <syslog.h>

namespace lector
{

    template <typename T>
    class concurrent_queue
    {
    public:
        typedef boost::mutex mutex_type;
        void push (T const& data)
        {
            syslog(LOG_INFO, "PUSH");
            {
                boost::lock_guard<mutex_type> lock(mutex_);
                queue_.push(data);
            }

            cv_.notify_one();
            syslog(LOG_INFO, "PUSHED");
        }

        void pop(T& data)
        {
            syslog(LOG_INFO, "POP");
            boost::unique_lock<mutex_type> lock(mutex_);
            while (queue_.empty())
                cv_.wait(lock);

            data = queue_.front();
            queue_.pop();
            syslog(LOG_INFO, "POPPED");
        }

    private:
        mutex_type mutex_;
        boost::condition_variable_any cv_;
        std::queue<T> queue_;
    };
}

#endif
