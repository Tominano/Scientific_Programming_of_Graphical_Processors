#include <vector>
#include <random>
#include <numeric>
#include <future>
#include <chrono>
#include <iostream>
#include <algorithm>


/*Process a large vector with many threads via async:*/
int main()
{
	std::vector<double> vec1(4096);
	std::vector<double> vec2(4096);


	std::random_device random_source;
	std::normal_distribution<double> dist(5.0, 10.0); /* normal distribution with average 5 and sigma = 10, generating doubles*/
	std::generate( vec1.begin(), vec1.end(), [&](){ return dist(random_source); } ); /*fill the vector with random numbers*/
	std::generate(vec2.begin(), vec2.end(), [&]() { return dist(random_source); }); /*fill the vector with random numbers*/



	auto averager = [](auto it0, auto it1)//lambda to average numbers between two iterators
	{
		auto difference = std::distance(it0, it1);
		auto sum = std::accumulate(it0, it1, 0.0, [](double x, double y){ return x+y; });
		return sum / difference;
	};

	auto deviation = [](auto it0, auto it1)//lambda to average numbers between two iterators
	{
 	    auto sqr = std::accumulate(it0, it1, 0.0, [](double x, double y){ return x*x+y*y; });
		auto sum = std::accumulate(it0, it1, 0.0, [](double x, double y){ return x+y; });
		return sqrt(sqr/sum);
	};


	int max_num_of_threads = (int)std::thread::hardware_concurrency(); //query number of threads
	std::cout << "Using " << max_num_of_threads << " threads.\n";
	std::vector<std::future<double>> futures1(max_num_of_threads);
	std::vector<std::future<double>> futures2(max_num_of_threads);



	auto time0 = std::chrono::high_resolution_clock::now();

	//start threads:
	for(int n=0; n<max_num_of_threads; ++n )
	{
		auto it0 = vec1.begin() + n * vec1.size() / max_num_of_threads;
        	auto it1 = vec1.begin() + (n + 1) * vec1.size() / max_num_of_threads;
		futures1[n] = std::async( std::launch::async, averager, it0, it1 );
	}

		//wait on the futures and add results when they become available:
	auto parallel_result1 = std::accumulate(futures1.begin(), futures1.end(), 0.0, [](double acc, std::future<double>& f){ return acc + f.get(); } );

	auto time1 = std::chrono::high_resolution_clock::now();


	for (int n = 0; n < max_num_of_threads; ++n)
	{
		auto it0 = vec2.begin() + n * vec2.size() / max_num_of_threads;
		auto it1 = vec2.begin() + (n + 1) * vec2.size() / max_num_of_threads;
		futures2[n] = std::async(std::launch::async, deviation, it0, it1);
	}


	auto parallel_result2 = std::accumulate(futures2.begin(), futures2.end(), 0.0, [](double acc, std::future<double>& f){ return acc + f.get(); });

	auto time2 = std::chrono::high_resolution_clock::now();



	auto serial_result_of_av = std::accumulate(vec1.begin(), vec1.end(), 0.0);

	auto time3 = std::chrono::high_resolution_clock::now();

	auto serial_result_of_dev = std::accumulate(vec2.begin(), vec2.end(), 0.0);

	auto time4 = std::chrono::high_resolution_clock::now();


	std::cout << "Serial average is:   " << serial_result_of_av / (double)vec1.size() << " \t Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(time3-time2).count() << " usec.\n";
	std::cout << "Parallel average is: " << parallel_result1 / (double)max_num_of_threads << " \t Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(time1-time0).count() << " usec\n";
	
	std::cout << "Serial deviation is:   " << serial_result_of_dev / (double)vec1.size() << " \t Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(time4 - time3).count() << " usec.\n";
	std::cout << "Parallel deviation is: " << parallel_result2 / (double)max_num_of_threads << " \t Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(time2 - time1).count() << " usec\n";

	std::cout << "Elapsed time of serial computing is: " << std::chrono::duration_cast<std::chrono::microseconds>(time2 - time1).count() << " usec.\n";
	std::cout << "Elapsed computing of Parallel deviation is: " << std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count() << " usec\n";

}