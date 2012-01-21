#ifndef UTIL_CONCURRENT_QUEUE_HPP
#define UTIL_CONCURRENT_QUEUE_HPP

#include <boost/thread.hpp>
#include <queue>

namespace lector
{

    template <typename T>
    class concurrent_queue
    {
    public:
        typedef boost::mutex mutex_type;
        void push (T const& data)
        {
            {
                boost::lock_guard<mutex_type> lock(mutex_);
                queue_.push(data);
            }

            cv_.notify_one();
        }

        void pop(T& data)
        {
            boost::unique_lock<mutex_type> lock(mutex_);
            while (queue_.empty())
                cv_.wait(lock);

            data = queue_.front();
            queue_.pop();
        }

    private:
        mutex_type mutex_;
        boost::condition_variable_any cv_;
        std::queue<T> queue_;
    };
}

#endif
