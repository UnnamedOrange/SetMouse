#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class timer_thread
{
private:
	std::function<void()> proc;
	bool finish{};
	std::thread thread_timer;
	std::mutex mutex_exit;
	std::condition_variable cv;
	void thread_routine()
	{
		while (true)
		{
			std::unique_lock lock(mutex_exit);
			cv.wait_for(lock, std::chrono::milliseconds(elapse),
				[&]()->bool
				{
					return finish;
				});
			if (finish)
				break;

			proc();
		}
	}
public:
	unsigned elapse;
private:
	std::thread thread_work;

public:
	timer_thread(std::function<void()> proc, unsigned elapse = 1000) : proc(proc), elapse(elapse),
		thread_work(&timer_thread::thread_routine, this)
	{
	}
	~timer_thread()
	{
		finish = true;
		cv.notify_all();
		thread_work.join();
	}
};