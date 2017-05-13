#pragma once
#include <thread>
#include <vector>
#include <mutex>

// Samples:
//
// Simple for each:
// parallel::for_each_RA(vector.begin(), vector.end(), [](std::vector<int>::iterator it){
//		// incrementing every value by 2
//		*it += 2;
// });
//
// Gathering partial solutions
// size_t sum = 0;
// auto partialSums = parallel::for_each_RA(vector, sum, [](std::vector<size_t>::iterator it, size_t& partSum){
//		partSum += *it;
//	});
//	// add partial sums on local thread
//	sum = std::accumulate(partialSums.begin(), partialSums.end(), size_t(0));
//	

// helper functions for parallel processing
namespace parallel
{
	/**
	* \brief executes a for each loop parallel
	* \tparam RAIter random access iterator
	* \tparam Functor functor which takes one RAIter as argument
	* \param nThreads number of cuncurrent threads
	*/
	template<typename RAIter, typename Functor>
	inline void for_each_RA(RAIter begin, RAIter end, Functor func, size_t nThreads = std::thread::hardware_concurrency())
	{
		size_t length = size_t(end - begin);
		size_t step = length / nThreads;

		// intervall function
		auto doTask = [func](RAIter begin, RAIter end)
		{
			for (; begin != end; ++begin)
				func(begin);
		};

		// create threads
		std::vector<std::thread> threads;
		threads.reserve(nThreads - 1);
		for (size_t i = 0; i < nThreads - 1; ++i)
		{
			threads.emplace_back(doTask, begin, begin + step);
			begin += step;
		}
		// run last on this thread
		doTask(begin, end);

		// join
		for (auto& t : threads)
			t.join();
	}

	/**
	 * \brief executes a for each loop parallel
	 * \tparam RAContainer container with random access iterator
	 * \tparam Functor functor which takes one RAIter as argument
	 * \param nThreads number of cuncurrent threads
	 */
	template<typename RAContainer, typename Functor>
	inline void for_each_RA(RAContainer& container, Functor func, size_t nThreads = std::thread::hardware_concurrency())
	{
		for_each_RA(container.begin(), container.end(), func, nThreads);
	}

	/**
	 * \brief executes a for each loop parallel
	 * \tparam FIter forward iterator
	 * \tparam Functor functor which takes one FIter as argument
	 * \param nThreads number of cuncurrent threads
	 */
	template<typename FIter, typename Functor>
	inline void for_each_FI(FIter begin, FIter end, Functor func, size_t nThreads = std::thread::hardware_concurrency())
	{
		// length is unknown. therefore make a provide function which takes the next 
		// not used iterator
		std::mutex mutex;
		auto provide = [&begin, &end, &mutex]()
		{
			std::lock_guard<std::mutex> g(mutex);
			if (begin == end)
				return end;
			// take current value
			auto ret = begin;
			// increment iterator
			++begin;
			return ret;
		};

		// do task as long as end not reached
		auto doTask = [&provide, &func, &end]()
		{
			FIter i = provide();
			while (i != end)
			{
				func(i);
				i = provide();
			}
		};

		// start threads
		std::vector<std::thread> threads;
		threads.reserve(nThreads - 1);
		for (size_t i = 0; i < nThreads - 1; ++i)
			threads.emplace_back(doTask);
		doTask();

		// join
		for (auto& t : threads)
			t.join();
	}

	/**
	* \brief executes a for each loop parallel
	* \tparam FIContainer container with forward iterator
	* \tparam Functor functor which takes one FIter as argument
	* \param nThreads number of cuncurrent threads
	*/
	template<typename FIContainer, typename Functor>
	inline void for_each_FI(FIContainer& container, Functor func, size_t nThreads = std::thread::hardware_concurrency())
	{
		for_each_FI(container.begin(), container.end(), func, nThreads);
	}


	/**
	 * \brief executes a for each loop parallel and returns a vector of gathered solutions
	 * \tparam RAIter random access iterator
	 * \tparam SolutionT type of the solution
	 * \tparam Functor1 functor which takes one RAIter as first argument and a SolutionT& as second
	 * \param s initial state for the solution
	 * \param nThreads number of cuncurrent threads 
	 * \return vector with solutions gathered in each thread
	 */
	template<typename RAIter, typename SolutionT, typename Functor1>
	inline std::vector<SolutionT> for_each_gather_RA(RAIter begin, RAIter end, SolutionT s, Functor1 func, size_t nThreads = std::thread::hardware_concurrency())
	{
		size_t length = size_t(end - begin);
		size_t step = length / nThreads;

		// intervall function
		auto doTask = [func](RAIter begin, RAIter end, SolutionT* solution)
		{
			for (; begin != end; ++begin)
				func(begin, *solution);
		};

		// create threads
		std::vector<std::thread> threads;
		threads.reserve(nThreads - 1);
		// create the solutions
		std::vector<SolutionT> solutions;
		solutions.assign(nThreads, s);

		for (size_t i = 0; i < nThreads - 1; ++i)
		{
			threads.emplace_back(std::thread(doTask, begin, begin + step, &solutions[i]));
			begin += step;
		}
		// run last on this thread
		doTask(begin, end, &solutions[nThreads - 1]);

		// join
		for (auto& t : threads)
			t.join();

		// gather the solutions
		return solutions;
	}

	/**
	* \brief executes a for each loop parallel and returns a vector of gathered solutions
	* \tparam RAContainer container with random access iterator
	* \tparam SolutionT type of the solution
	* \tparam Functor1 functor which takes one RAIter as first argument and a SolutionT& as second
	* \param s initial state for the solution
	* \param nThreads number of cuncurrent threads
	* \return vector with solutions gathered in each thread
	*/
	template<typename RAContainer, typename SolutionT, typename Functor1>
	inline std::vector<SolutionT> for_each_gather_RA(RAContainer& container, SolutionT s, Functor1 func, size_t nThreads = std::thread::hardware_concurrency())
	{
		return for_each_RA(container.begin(), container.end(), s, nThreads);
	}

	/**
	* \brief executes a for each loop parallel and returns a vector of gathered solutions
	* \tparam FIter forward iterator
	* \tparam SolutionT type of the solution
	* \tparam Functor1 functor which takes one FIter as first argument and a SolutionT& as second
	* \param s initial state for the solution
	* \param nThreads number of cuncurrent threads
	* \return vector with solutions gathered in each thread
	*/
	template<typename FIter, typename SolutionT, typename Functor1>
	inline std::vector<SolutionT> for_each_gather_FI(FIter begin, FIter end, SolutionT s, Functor1 func, size_t nThreads = std::thread::hardware_concurrency())
	{
		// length is unknown. therefore make a provide function which takes the next 
		// not used iterator
		std::mutex mutex;
		auto provide = [&begin, &end, &mutex]()
		{
			std::lock_guard<std::mutex> g(mutex);
			if (begin == end)
				return end;
			// take current value
			auto ret = begin;
			// increment iterator
			++begin;
			return ret;
		};

		// do task as long as end not reached
		auto doTask = [&provide, &func, &end](SolutionT* solution)
		{
			FIter i = provide();
			while (i != end)
			{
				func(i, *solution);
				i = provide();
			}
		};

		// start threads
		std::vector<std::thread> threads;
		// create solutions
		std::vector<SolutionT> solutions;
		solutions.assign(nThreads - 1, s);

		threads.reserve(nThreads - 1);
		for (size_t i = 0; i < nThreads - 1; ++i)
			threads.emplace_back(doTask, &solutions[i]);
		doTask(&solutions[nThreads - 1]);

		// join
		for (auto& t : threads)
			t.join();

		return solutions;
	}

	/**
	* \brief executes a for each loop parallel and returns a vector of gathered solutions
	* \tparam FIContainer container with forward iterator
	* \tparam SolutionT type of the solution
	* \tparam Functor1 functor which takes one FIter as first argument and a SolutionT& as second
	* \param s initial state for the solution
	* \param nThreads number of cuncurrent threads
	* \return vector with solutions gathered in each thread
	*/
	template<typename FIContainer, typename SolutionT, typename Functor1>
	inline std::vector<SolutionT> for_each_gather_FI(FIContainer& container, SolutionT s, Functor1 func, size_t nThreads = std::thread::hardware_concurrency())
	{
		return for_each_gather_FI(container.begin(), container.end(), s, func, nThreads);
	}
}