// THREAPOOL Implementation file
////////////////////////////////
#include "TP.h"


ThreadPool::ThreadPool(unsigned int numberOfThreads)
  : quit(false), stopped(false) {
	//if(numberOfThreads == 2) numberOfThreads = 4;
	for(unsigned int t=1; t < numberOfThreads+1; t++)
		threads.emplace_back( std::thread(&ThreadPool::Run, this, t) );
			
	std::cout << "Running with " << numberOfThreads << " threads\n";
      	// NOTE: The above line is tricky: 
	// how to add a function pointer to a member function of a class
	// to a STL container, so I am giving you the exact line of code

	// Notice it needs to know about the member function as well as 'this' 
	// which is the pointer to class data.
}

ThreadPool::~ThreadPool() {
	//if not stopped, call ShutDown
	if(!stopped) ShutDown();
}

void ThreadPool::ShutDown() {
	{ 
		// lock the thread pool
		std::unique_lock<std::mutex> myLock(mtx);
		// set the 'quit' flag	
		quit = true;
		// wake up all threads
		cv.notify_all();
	}
	// join all threads  // wait for threads to exit
	for(auto& e : threads ) e.join();	
	// set the stopped flag
	stopped = true;
}
void ThreadPool::AddJob(std::function<void(void)> job) {
	// if 'stopped', throw "thread pool shutdown, not accepting jobs"
	if(stopped) throw "thread pool shutdown, not accepting jobs.\n"; 
	// if 'quit',    throw "thread pool shutting down, not accepting jobs"
	if (quit)   throw "thread pool shutting down, not accepting jobs.\n"; 
   	{ 
		// lock the thread pool
		std::unique_lock<std::mutex> myLock(mtx);
		// push 'job' onto the 'jobQueue'
		jobQueue.push(job);
	}
	//wake up one thread
	cv.notify_one();
}
void ThreadPool::Run(int threadnum) { // note this is a private member function
	// declare a job 'void(void)' function pointer 'job'
	std::function<void(void)> job;
	while(true) 
	{
		{ 
			// lock the thread pool
			std::unique_lock<std::mutex>myLock(mtx);
			
			// wait on the condition variable for a job to arrive
			// to prevent sperious wakeups, code a lambda 
			// function which returns true if either
			// - jobs are available (use job.empty() )
			// - or the quit flag is set
			auto f = [this] () ->bool
			{ return !jobQueue.empty() || quit; };
			cv.wait(myLock, f);
			
			// if the quit flag is set and there aren't any jobs
				// return
			if(quit && jobQueue.empty()) return;

			// get the job off the job queue
			// job = jobQueue.???
			// jobQueue.???
			job = jobQueue.front();
			jobQueue.pop();
		}	
		// run 'job'
		job();
	}	
}
