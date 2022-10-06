/**
 * @file   test for thread_pool 
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2020/07/19 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "_MyLib/src/thread_pool.h"


/**
 * @brief thread_pool test
 */

void work()
{
	log_info "tid = %u, running", GetCurrentThreadId() log_end;
};

struct worker
{
	void operator()()
	{
		log_info "tid = %u, running", GetCurrentThreadId() log_end;
	};
};

void more_work(int v)
{
	log_info "tid = %u, running = %d", GetCurrentThreadId(), v log_end;
	//getchar();
};



class RunClass
{
public:
	bool CalledByThread(_In_ const char* msg)
	{
		log_info "tid=%u, msg=%s",
			GetCurrentThreadId(),
			msg
			log_end;
		return true;
	}
};


bool test_thread_pool()
{
	thread_pool pool(8);
	pool.run_task(work);                        // Function pointer.
	pool.run_task(worker());                    // Callable object.
	pool.run_task(boost::bind(more_work, 5)); // Callable object.	

	pool.run_task([]()								// lambda
	{
		log_info "tid=%u", GetCurrentThreadId() log_end;
	});


	RunClass rc;
	pool.run_task([&]()
	{
		if (true != rc.CalledByThread("test msg"))
		{
			log_err "rc.CalledByThread() failed." log_end;
		}
		else
		{
			log_info "rc.CalledByThread() succeeded." log_end;
		}
	});


	// Wait until all tasks are done.
	while (0 < pool.get_task_count())
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return true;
}