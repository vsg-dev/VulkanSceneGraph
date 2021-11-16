/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/Affinity.h>

#ifdef _WIN32

#    include <process.h>
#    include <windows.h>

static void win32_setAffinity(HANDLE tid, const vsg::Affinity& affinity)
{
    uint32_t numProcessors = std::thread::hardware_concurrency();

    DWORD_PTR affinityMask = 0x0;
    if (affinity)
    {
        for (auto cpu : affinity.cpus)
        {
            if (cpu < numProcessors)
            {
                affinityMask |= (0x1LL << cpu);
            }
        }
    }
    else
    {
        // set affinity to all CPU cores
        for (uint32_t cpu = 0; cpu < numProcessors; ++cpu)
        {
            affinityMask |= (0x1LL << cpu);
        }
    }

    /*DWORD_PTR res =*/SetThreadAffinityMask(tid, affinityMask);
}

void vsg::setAffinity(std::thread& thread, const Affinity& affinity)
{
    win32_setAffinity((HANDLE)thread.native_handle(), affinity);
}

void vsg::setAffinity(const Affinity& affinity)
{
    win32_setAffinity(GetCurrentThread(), affinity);
}

#elif defined(__APPLE__)

#    include <mach/mach.h>
#    include <pthread.h>

static void macos_setAffinity(pthread_t thread_native_handle, const vsg::Affinity& affinity)
{
    uint32_t numProcessors = std::thread::hardware_concurrency();

    integer_t cpuset = 0;
    if (affinity)
    {
        for (auto cpu : affinity.cpus)
        {
            if (cpu < numProcessors)
            {
                cpuset |= (0x1 << cpu);
            }
        }
    }
    else
    {
        // set affinity to all CPU cores
        for (uint32_t cpu = 0; cpu < numProcessors; ++cpu)
        {
            cpuset |= (0x1 << cpu);
        }
    }

    auto mach_thread = pthread_mach_thread_np(thread_native_handle);
    thread_affinity_policy_data_t policy = {cpuset};
    thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
}

void vsg::setAffinity(std::thread& thread, const Affinity& affinity)
{
    macos_setAffinity(thread.native_handle(), affinity);
}

void vsg::setAffinity(const Affinity& affinity)
{
    macos_setAffinity(pthread_self(), affinity);
}

#elif defined(__ANDROID__)

void vsg::setAffinity(std::thread&, const Affinity&)
{
    // Not currently implemented
}

void vsg::setAffinity(const Affinity&)
{
    // Not currently implemented
}

#else // unices

static void pthread_setAffinity(pthread_t thread_native_handle, const vsg::Affinity& affinity)
{
    uint32_t numProcessors = std::thread::hardware_concurrency();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    if (affinity)
    {
        for (auto cpu : affinity.cpus)
        {
            if (cpu < numProcessors)
            {
                CPU_SET(cpu, &cpuset);
            }
        }
    }
    else
    {
        // set affinity to all CPU cores
        for (uint32_t cpu = 0; cpu < numProcessors; ++cpu)
        {
            CPU_SET(cpu, &cpuset);
        }
    }

    /*int rc =*/pthread_setaffinity_np(thread_native_handle, sizeof(cpu_set_t), &cpuset);
}

void vsg::setAffinity(std::thread& thread, const Affinity& affinity)
{
    pthread_setAffinity(thread.native_handle(), affinity);
}

void vsg::setAffinity(const Affinity& affinity)
{
    pthread_setAffinity(pthread_self(), affinity);
}

#endif
