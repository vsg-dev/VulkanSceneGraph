/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/io/Options.h>

#include <iostream>


using namespace vsg;

Allocator::Allocator()
{
    std::cout<<"Allocator::Allocator()"<<std::endl;
}

Allocator::~Allocator()
{
    std::cout<<"Allocator::~Allocator() "<<std::endl;
}

#if 1
static std::unique_ptr<Allocator> s_allocator(new Allocator());
#else
static std::unique_ptr<Allocator> s_allocator;
#endif

std::unique_ptr<Allocator>& Allocator::instance()
{
    return s_allocator;
}

void* Allocator::allocate(std::size_t count, MemoryAffinity /*memoryAffinity*/)
{
    auto ptr = ::operator new(count);
    return ptr;
}

void Allocator::deallocate(void* ptr)
{
    ::operator delete(ptr);
}


void* vsg::allocate(std::size_t count, MemoryAffinity memoryAffinity)
{
    if (Allocator::instance()) return Allocator::instance()->allocate(count, memoryAffinity);
    else return std::malloc(count);
}

void vsg::deallocate(void* ptr)
{
    if (Allocator::instance()) return Allocator::instance()->deallocate(ptr);
    else return std::free(ptr);
}
