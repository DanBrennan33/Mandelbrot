#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

class Timer { // C++11 chrono stop-watch timer class. Needs "#include <chrono>".
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> stop;
	public:
	void Start() { start  = std::chrono::high_resolution_clock::now(); }
	void Stop()  { stop   = std::chrono::high_resolution_clock::now(); }
	uint64_t msecs() {
		typedef std::chrono::duration<int,std::milli> millisecs_t ;
		millisecs_t duration_get( std::chrono::duration_cast<millisecs_t>(stop-start) ) ;
		uint64_t ms = duration_get.count();
		return ms;
	}
	uint64_t usecs() {
		typedef std::chrono::duration<int,std::micro> microsecs_t ;
		microsecs_t duration_get( std::chrono::duration_cast<microsecs_t>(stop-start) ) ;
		uint64_t us = duration_get.count();
		return us;
	}
	uint64_t nsecs() {
		typedef std::chrono::duration<int,std::nano> nanosecs_t ;
		nanosecs_t duration_get( std::chrono::duration_cast<nanosecs_t>(stop-start) ) ;
		uint64_t ns = duration_get.count();
		return ns;
	}
};

class ThreadPool {
	private:
		std::vector<std::thread>		threads;
		std::queue< std::function<void()> >  	jobQueue;
		std::mutex				mtx;
		std::condition_variable 		cv;
		bool					quit;
		bool					stopped;
		void 					Run(int threadnum); // note this is a private member function
	public:
		ThreadPool(unsigned int numberOfThreads = std::thread::hardware_concurrency());
		~ThreadPool();
		void AddJob(std::function<void()> job);
		void ShutDown();
};
