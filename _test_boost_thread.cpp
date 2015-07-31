#include "stdafx.h"
#include <iostream>
#include <boost\thread\thread.hpp>
#include <boost\bind.hpp>
#include <boost\thread\condition.hpp>

const int _buf_size = 10;
const int _iters = 100;

boost::mutex io_mutex;

/// @brief
class buffer
{
public:
    typedef boost::mutex::scoped_lock scoped_lock;

    buffer() :p(0), c(0), full(0)
    {
    }
    ~buffer()
    {
    }

    void put(int m)
    {
        scoped_lock lock(mutex);
        if (full == _buf_size)
        {
            {
                boost::mutex::scoped_lock lock(io_mutex);
                std::cout << "buffer is full. waiting..." << std::endl;
            }

            while (full == _buf_size)
            {
                cond.wait(lock);    // unlock mutex and wait for cond.notify_xxx()
                                    // cond automatically lock before return wait().
            }
        }

        buf[p] = m;
        p = (p+1) % _buf_size;
        ++full;
        cond.notify_one();          // unlock and wake up waiting thread.
    }

    int get()
    {
        scoped_lock lock(mutex);

        if (full == 0)
        {
            {
                boost::mutex::scoped_lock lock(io_mutex);
                std::cout << "buffer is empty. waiting..." << std::endl;
            }

            while (full == 0)
            {
                cond.wait(lock);
            }
        }

        int i = buf[c];
        c = (c+1) % _buf_size;
        --full;
        cond.notify_one();      
        return i;
    }

private:
    boost::mutex        mutex;
    boost::condition    cond;
    unsigned int        p, c, full;
    int                 buf[_buf_size];
};

/// @brief 
void hello()
{
    log_info "tid = %u, hello, boost thread.", GetCurrentThreadId() log_end
}


/// @brief 
struct stcount
{
    stcount(int id) : id(id)
    {
    }

    void operator()()
    {
        for (int i = 0; i < 10; ++i)
        {
            boost::mutex::scoped_lock lock(io_mutex);
            std::cout << id << ":" << i << std::endl;
        }
    }

    int id;
};

/// @brief  
void count(int id)
{
    for (int i = 0; i < 10; ++i)
    {
        boost::mutex::scoped_lock lock(io_mutex);
        std::cout << id << ":" << i << std::endl;
    }
}

/// @brief
buffer buf;

void writer()
{
    for (int n = 0; n < _iters; ++n)
    {
        {
            boost::mutex::scoped_lock lock(io_mutex);
            std::cout << "sending: " << n << std::endl;
        }

        buf.put(n);
    }
}

void reader()
{
    for (int x = 0; x < _iters; ++x)
    {
        int n = buf.get();
        {
            boost::mutex::scoped_lock lock(io_mutex);
            std::cout << "received: " << n << std::endl;
        }
    }
}

/// @brief boost thread test 
bool test_boost_thread()
{
    //// #1 boost thread
    //boost::thread t(&hello);
    //log_info "tid = %u", t.get_id() log_end
    //t.join();
    //log_info "=======================" log_end

    //// #2 boost::mutex
    //boost::thread t1(stcount(1));
    //boost::thread t2(stcount(2));
    //t1.join();
    //t2.join();
    //log_info "=======================" log_end

    //// #3 boost::mutex using bind
    //boost::thread t12(boost::bind(&count, 1));
    //boost::thread t22(boost::bind(&count, 2));
    //t12.join();
    //t22.join();
    //log_info "=======================" log_end

    // #4 boost::condition 
    boost::thread t13(&reader);
    boost::thread t23(&writer);
    t13.join();
    t23.join();


    return true;
}
