#include <vector>
#include <random>
#include <numeric>
#include <future>
#include <chrono>
#include <iostream>
#include <algorithm>




#include<CL/cl.h>
#include<CL/cl2.hpp>




#include "E:\CL\OpenCL-C++-API.hpp"
#include "E:\CL\OpenCL-C++-API-config.in.hpp"




// Checks weather device is DP calable or not
bool is_device_dp_capable(const cl::Device& device)
{
    return (device.getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_fp64")) ||
        (device.getInfo<CL_DEVICE_EXTENSIONS>().find("cl_amd_fp64"));
}


int main()
{
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        for (const auto& platform : platforms) std::cout << "Found platform: " << platform.getInfo<CL_PLATFORM_VENDOR>() << std::endl;

        // Choose platform with most DP capable devices
        auto plat = std::max_element(platforms.cbegin(), platforms.cend(), [](const cl::Platform& lhs, const cl::Platform& rhs)
            {
                auto dp_counter = [](const cl::Platform& platform)
                {
                    std::vector<cl::Device> devices;
                    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

                    return std::count_if(devices.cbegin(), devices.cend(), is_device_dp_capable);
                };

                return dp_counter(lhs) < dp_counter(rhs);
            });

        if (plat != platforms.cend())
            std::cout << "Selected platform: " << plat->getInfo<CL_PLATFORM_VENDOR>() << std::endl;
        else
            throw std::runtime_error{ "No double-precision capable device found." };

        // Obtain DP capable devices
        std::vector<cl::Device> devices;
        plat->getDevices(CL_DEVICE_TYPE_ALL, &devices);

        std::remove_if(devices.begin(), devices.end(), [](const cl::Device& dev) {return !is_device_dp_capable(dev); });

        cl::Device device = devices.at(0);

        std::cout << "Selected device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

        // Create context and queue
        std::vector<cl_context_properties> props{ CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>((*plat)()), 0 };
        cl::Context context{ devices, props.data() };

        cl::CommandQueue queue{ context, device, cl::QueueProperties::Profiling };

        // Load program source
        std::ifstream source_file{ kernel_location };
        if (!source_file.is_open())
            throw std::runtime_error{ std::string{"Cannot open kernel source: "} + kernel_location };

        // Create program and kernel
        cl::Program program{ context, std::string{ std::istreambuf_iterator<char>{ source_file },
                                                   std::istreambuf_iterator<char>{} } };

        program.build({ device }, "-cl-std=CL1.0"); // Any warning counts as a compilation error, simplest kernel syntax

        auto vecMean = cl::KernelFunctor<cl::Buffer>(program, "vecMean");
        auto vecDev = cl::KernelFunctor<cl::Buffer>(program, "vecDev");


        // Init computation
        const std::size_t chainlength = std::size_t(std::pow(2u, 20u)); // 1M, cast denotes floating-to-integral conversion,
                                                                        //     promises no data is lost, silences compiler warning
        std::valarray<cl_double> vec_x(chainlength);




        // Fill arrays with random values between 0 and 100
        auto prng = [engine = std::default_random_engine{},
            distribution = std::uniform_real_distribution<cl_double>{ -100.0, 100.0 }]() mutable { return distribution(engine); };

        std::generate_n(std::begin(vec_x), chainlength, prng);

        cl::Buffer buf_x{ context, std::begin(vec_x), std::end(vec_x), true };


        // Explicit (blocking) dispatch of data before launch
        cl::copy(queue, begin(vec_x), end(vec_x), buf_x);


        // Launch kernels
        cl::Event kernel_event1{ vecMean(cl::EnqueueArgs{ queue, cl::NDRange{ chainlength } }, buf_x) };
        cl::Event kernel_event2{ vecDev(cl::EnqueueArgs{ queue, cl::NDRange{ chainlength } },buf_x) };


        kernel_event1.wait();

        kernel_event2.wait();

        std::cout <<
            "Device (kernel) execution took: " <<
            util::get_duration<CL_PROFILING_COMMAND_START,
            CL_PROFILING_COMMAND_END,
            std::chrono::microseconds>(kernel_event1).count() <<
            " us." << std::endl;

        std::cout <<
            "Device (kernel) execution took: " <<
            util::get_duration<CL_PROFILING_COMMAND_START,
            CL_PROFILING_COMMAND_END,
            std::chrono::microseconds>(kernel_event2).count() <<
            " us." << std::endl;


        // Compute validation set on host
       
        return EXIT_SUCCESS;
    
}