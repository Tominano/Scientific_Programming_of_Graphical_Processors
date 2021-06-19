// C standard includes
#include <stdio.h>
#include <vector>
#include <random>
#include <numeric>
#include <future>
#include <chrono>
#include <iostream>
#include <algorithm>

using namespace std;

// OpenCL includes
#include <CL/cl.h>

int main()
{
    vector<cl::Platform>platforms;
    cl::Platform::get(&platforms);

    for (const auto& platform : platforms)
        cout << "Found platform: " << platform.getInfo<CL_PLATFORM_VENDOR>() << endl;


    auto plat = max_element(platforms.cbegin(), platforms.cend(), [](const cl::Platform& lhs, const cl::Platform& rhs)
        {
            auto dp_counter = [](const cl::Platform& platform)
            {
                vector<cl::Device> devices;
                platform.getDdevices(CL_DEVICE_TYPE_ALL, &devices);

                return count_if(devices.cbegin(), devices.cend(), is_device_dp_capable);
            };

            return dp_counter(lhs) < dp_counter(rhs)
        }


    return 0;
}