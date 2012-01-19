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
        void push (T const& data)
        {
            boost::mutex::scoped_lock lock(mutex_);
            queue_.push(data);
            lock.unlock();
            cv_.notify_one();
        }

        void pop(T& data)
        {
            boost::mutex::scoped_lock lock(mutex_);
            while (queue_.empty())
                cv_.wait(lock);

            data = queue_.front();
            queue_.pop();
        }

    private:
        boost::mutex mutex_;
        boost::condition_variable cv_;
        std::queue<T> queue_;
    };
}

#endif
