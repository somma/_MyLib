/**
 * @file    thread_pool.h
 * @brief   thread pool implementation.
 * 
 * This file contains thread pool implementation using boost thread.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2015:08:01 10:02 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include <queue>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

/// @brief thread pool implementation
/// @see test_thread_pool()
/// @see original code is from http://stackoverflow.com/questions/12215395/thread-pool-using-boost-asio
typedef class thread_pool
{
private:
    std::queue< boost::function< void() > > _tasks;
    boost::thread_group                     _threads;  
    std::size_t                             _pool_size;
    std::size_t                             _available;
    boost::mutex                            _lock;
    boost::condition_variable               _condition;
    bool                                    _running;

public:
    /// @brief  Constructor
    thread_pool(std::size_t pool_size)
        :
        _pool_size(pool_size),
        _available(pool_size),
        _running(true)
    {
        for (std::size_t i = 0; i < pool_size; ++i)
        {
            _threads.create_thread(boost::bind(&thread_pool::pool_main, this));
        }
    }

    /// @brief Destructor.
    ~thread_pool()
    {
        // Set running flag to false then notify all threads.
        {
            boost::unique_lock< boost::mutex > lock(_lock);
            _running = false;
            _condition.notify_all();
        }

        try
        {
            _threads.join_all();
            //log_dbg 
            //    "all thread joined. available = %u, remain task = %u ", 
            //    this->_available, 
            //    this->_tasks.size()
            //log_end
        }
        // Suppress all exceptions.
        catch (...) {}
    }

    /// @brief return task count in the queue.
    std::size_t get_task_count()
    {
        boost::unique_lock< boost::mutex > lock(_lock);
        return _tasks.size();
    }

    /// @brief  return true if all thread in pool is idle.
    bool is_idle()
    {
        boost::lock_guard< boost::mutex > lock(_lock);
        return (_tasks.empty() && _available == _pool_size) ? true : false;
    }

    /// @brief Add task to the thread pool if a thread is currently available.
    template < typename Task >
    bool run_task(Task task)
    {
        // We will use condition variable later of this function, so we have to 
        // use unique_lock.
        boost::unique_lock< boost::mutex > lock(_lock);

        // If no threads are available, then return.
        if (0 == _available) return false;

        // Decrement count, indicating thread is no longer available.
        --_available;

        // Set task and signal condition variable so that a worker thread will
        // wake up andl use the task.
        _tasks.push(boost::function< void() >(task));
        _condition.notify_one();
        return true;
    }

    /// @brief return true if idle thread exists.
    bool is_available()
    {
        // boost::lock_guard is enough becauese we will not use any condition variables.
        boost::lock_guard< boost::mutex > lock(_lock);
        if (0 == _available)
            return false;
        else
            return true;
    }

    /// @brief Entry point for pool threads.
    void pool_main()
    {
        while (_running)
        {
            // Wait on condition variable while the task is empty and the pool is
            // still running.
            boost::unique_lock< boost::mutex > lock(_lock);
            while (_tasks.empty() && _running)
            {
                _condition.wait(lock);
            }

            // If pool is no longer running, break out.
            if (!_running) 
            {
                ++_available;
                break;
            }

            // Copy task locally and remove from the queue.  This is done within
            // its own scope so that the task object is destructed immediately
            // after running the task.  This is useful in the event that the
            // function contains shared_ptr arguments bound via bind.
            {
                boost::function< void() > task = _tasks.front();
                _tasks.pop();

                lock.unlock();

                // Run the task.
                try
                {
                    task();
                }
                // Suppress all exceptions.
                catch (...) {}
            }

            // Task has finished, so increment count of available threads.
            lock.lock();
            ++_available;
        } // while _running
    }
} *pthread_pool;