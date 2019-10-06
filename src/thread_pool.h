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
#include <thread>
#include <mutex>

/// @brief thread pool implementation
/// @see test_thread_pool()
/// @see original code is from http://stackoverflow.com/questions/12215395/thread-pool-using-boost-asio
typedef class thread_pool
{
private:
	std::queue<std::function<void()>> _tasks;
	std::list<std::thread*>	_threads;
    std::size_t                             _pool_size;
    std::size_t                             _available;
	std::mutex	_lock;
	std::condition_variable _condition;
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
			std::thread* t = new std::thread(std::bind(&thread_pool::pool_main, 
													   this));
			_threads.push_back(t);
            
        }
    }

    /// @brief Destructor.
    ~thread_pool()
    {
        // Set running flag to false then notify all threads.
        {
			std::lock_guard<std::mutex> lock(_lock);
            _running = false;
            _condition.notify_all();
        }

        try
        {
			std::for_each(
				_threads.begin(), 
				_threads.end(), 
				[](std::thread* t) 
			{
				if (nullptr != t) t->join(); 
			});
			_threads.clear();
            
            //log_dbg 
            //    "all thread joined. available = %u, remain task = %u ", 
            //    this->_available, 
            //    this->_tasks.size()
            //log_end
        }
        catch (...) 
		{
			// Suppress all exceptions.
		}
    }

    /// @brief return task count in the queue.
    std::size_t get_task_count()
    {
		std::lock_guard<std::mutex> lock(_lock);
        return _tasks.size();
    }

    /// @brief  return true if all thread in pool is idle.
    bool is_idle()
    {
		std::lock_guard<std::mutex> lock(_lock);
        return (_tasks.empty() && _available == _pool_size) ? true : false;
    }

    /// @brief Add task to the thread pool if a thread is currently available.
    template < typename Task >
    bool run_task(Task task)
    {
		std::lock_guard<std::mutex> lock(_lock);

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
		std::lock_guard<std::mutex> lock(_lock);
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
            //
			//	_tasks 가 empty 이고, _running 이 true 이면 다른 누군가가 
			//	_condition.notify_one() / notify_all() 을 호출할 때까지
			//	wait 한다. 
			//
			std::unique_lock<std::mutex> lock(_lock);
			try
			{	
				_condition.wait(lock, 
								[&]() {return !(_tasks.empty() && true == _running); });
			}
			catch (...)
			{
				Sleep(1000);
				continue;
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
                auto task = _tasks.front();
                _tasks.pop();

                lock.unlock();
				                
                try
                {
                    task();
                }                
                catch (...) 
				{
					log_err "task thrown an exception..." log_end;
				}
            }

            // Task has finished, so increment count of available threads.
            lock.lock();
            ++_available;
        } 
    }
} *pthread_pool;