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

Allocator::~Allocator()
{
    std::cout << "Allocator::~Allocator() " << this << std::endl;
    std::cout << "     _bytesAllocated = " << _bytesAllocated << std::endl;
    std::cout << "     _countAllocated = " << _countAllocated << std::endl;
    std::cout << "     _bytesDeallocated = " << _bytesDeallocated << std::endl;
    std::cout << "     _countDellocated = " << _countDeallocated << std::endl;
}

void* Allocator::allocate(std::size_t size, const void* hint)
{
    std::cout << "Allocator::allocate(std::size_t " << size << ", const void*" << hint << " )" << std::endl;
    _bytesAllocated += size;
    ++_countAllocated;
    return ::operator new(size);
}

void* Allocator::allocate(std::size_t size)
{
    void* ptr = ::operator new(size);
    std::cout << "Allocator::allocate(std::size_t " << size << ") " << ptr << std::endl;
    _bytesAllocated += size;
    ++_countAllocated;
    return ptr;
}

void Allocator::deallocate(const void* ptr, std::size_t size)
{
    std::cout << "Allocator::deallocate(" << ptr << ", std::size_t " << size << ")" << std::endl;
    ::operator delete(const_cast<void*>(ptr));
    _bytesDeallocated += size;
    ++_countDeallocated;
}

Auxiliary* Allocator::getOrCreateSharedAuxiliary()
{
    if (!_sharedAuxiliary)
    {
        void* ptr = allocate(sizeof(Auxiliary));
        _sharedAuxiliary = new (ptr) Auxiliary(this);
        std::cout << "Allocator::getOrCreateSharedAuxiliary() creating new : " << _sharedAuxiliary << std::endl;
    }
    else
    {
        std::cout << "Allocator::getOrCreateSharedAuxiliary() returning existing : " << _sharedAuxiliary << std::endl;
    }
    return _sharedAuxiliary;
}

void Allocator::detachSharedAuxiliary(Auxiliary* auxiliary)
{
    if (_sharedAuxiliary == auxiliary)
    {
        std::cout << "Allocator::detachSharedAuxiliary(" << auxiliary << ") detecting auxiliary" << std::endl;
    }
    else
    {
        std::cout << "Allocator::detachSharedAuxiliary(" << auxiliary << ") auxiliary not matched" << std::endl;
    }
}
