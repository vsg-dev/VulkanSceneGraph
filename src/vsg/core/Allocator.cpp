/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>

#include <iostream>

using namespace vsg;


Allocator::~Allocator()
{
}

void* Allocator::allocate(std::size_t n, const void* hint )
{
    std::cout<<"Allocator::allocate(std::size_t "<<n<<", const void*"<< hint<<" )"<<std::endl;

    return ::operator new (n);
}

void* Allocator::allocate(std::size_t size)
{
    void* ptr = ::operator new (size);
    std::cout<<"Allocator::allocate(std::size_t "<<size<<") "<<ptr<<std::endl;
    return ptr;
}

void Allocator::deallocate(const void* ptr, std::size_t size)
{
    std::cout<<"Allocator::deallocate("<<ptr<<", std::size_t "<<size<<")"<<std::endl;
    ::operator delete(const_cast<void*>(ptr));
}
