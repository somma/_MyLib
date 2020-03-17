#include "stdafx.h"
#include <iostream>
#include <boost\thread\thread.hpp>
#include <boost\bind.hpp>
#include <boost\thread\condition.hpp>
#include "_MyLib/src/log.h"



const int _buf_size = 10;
const int _iters = 100;

boost::mutex io_mutex;

/// @brief
class buffer
{
public:
    buffer() :p(0), c(0), full(0)
    {
    }
    ~buffer()
    {
    }

    void put(int m)
    {

		// boost::condition 은 unique_lock 을 사용해야 하기 때문에 scoped_lock 을 사용
		// boost::mutex::sclped_lock 은 unique_lock 의 typedef 임
		boost::mutex::scoped_lock lock(mutex);
        if (full == _buf_size)
        {
            {
                boost::mutex::scoped_lock lock2(io_mutex);
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
		boost::mutex::scoped_lock lock(mutex);

        if (full == 0)
        {
            {
                boost::mutex::scoped_lock lock2(io_mutex);
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






class ConditionNotifyBeforeWait
{
public: 
	ConditionNotifyBeforeWait()
	{
	}

	~ConditionNotifyBeforeWait()
	{
	}

	void notifier()
	{
		boost::unique_lock<boost::mutex> lock(_lock);
		_cond.notify_one();
		log_info "notified" log_end;
	}

	void waiter()
	{
		boost::unique_lock<boost::mutex> lock(_lock);
		boost::cv_status wait_ret = _cond.wait_for(lock, boost::chrono::seconds(1));
		if (wait_ret == boost::cv_status::timeout)
		{
			log_info "wait time out" log_end;
		}
		else
		{
			log_info "awake" log_end;
		}
	}

	///	cond.notify() 를 먼저 호출하고, cond.wait() 을 나중에 호출하면 
	/// 또는 그 반대의 경우 어떻게 되는지 테스트 	
	void ConditionNotifyWaitTest()
	{
		/// wait 보다 notify 를 먼저 호출하면 깨어나지 않는다. 
		/// 당연한걸 뭐....
		boost::thread t1(boost::bind(&ConditionNotifyBeforeWait::notifier,
									 this));
		boost::thread t2(boost::bind(&ConditionNotifyBeforeWait::waiter,
									 this));

		t1.join();
		t2.join();

		/// wait( ) -> notify() 하면 의도대로 잘 깨어나지
		boost::thread t3(boost::bind(&ConditionNotifyBeforeWait::waiter,
									 this));
		boost::thread t4(boost::bind(&ConditionNotifyBeforeWait::notifier,
									 this));
		
		t3.join();
		t4.join();
	}

	/// SetEvent(), WaitForSingleObject() 순서에 따라
	/// 어떻게 동작하는지 테스트 
	///
	/// 이벤트 객체는 당연히 상태가 유지되는 커널객체니까
	/// Set 을 먼저하든 Wait 을 먼저하든 잘 되어야 정상
	void EventSetAfterWait()
	{
		HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
		_ASSERTE(NULL != event);
		
		boost::thread t1([event]()
		{
			SetEvent(event);
		});


		boost::thread t2([event]()
		{
			Sleep(5000);

			DWORD wait_ret = WaitForSingleObject(event, 1000);
			if (WAIT_OBJECT_0 == wait_ret)
			{
				log_info "event signaled." log_end;
			}
			else
			{
				log_info "wait failed. ret=%u", wait_ret log_end;
			}
		});

		t1.join();
		t2.join();
		CloseHandle(event);
	}


	void EventWaitAfterSet()
	{
		HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
		_ASSERTE(NULL != event);

		boost::thread t1([event]()
		{
			Sleep(1000);
			DWORD wait_ret = WaitForSingleObject(event, 1000);
			if (WAIT_OBJECT_0 == wait_ret)
			{
				log_info "event signaled." log_end;
			}
			else
			{
				log_info "wait failed. ret=%u", wait_ret log_end;
			}
		});


		boost::thread t2([event]()
		{			
			SetEvent(event);
		});

		t1.join();
		t2.join();
		CloseHandle(event);
	}


	boost::mutex _lock;
	boost::condition_variable _cond;
};




void test_func(int timeout)
{
	boost::this_thread::sleep_for(boost::chrono::milliseconds(timeout));
}

bool boost_try_join_for()
{
	boost::thread t(test_func, 1000);
	_ASSERTE(!t.try_join_for(boost::chrono::milliseconds(50)));


	boost::thread tt(test_func, 3000);
	_ASSERTE(t.try_join_for(boost::chrono::milliseconds(4000)));

	return true;
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

    //// #4 boost::condition 
    //boost::thread t13(&reader);
    //boost::thread t23(&writer);
    //t13.join();
    //t23.join();

	//ConditionNotifyBeforeWait cnbw;
	//cnbw.ConditionNotifyWaitTest();
	//cnbw.EventSetAfterWait();
	//cnbw.EventWaitAfterSet();

	boost_try_join_for();

    return true;
}








