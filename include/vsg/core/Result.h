#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <string>
#include <ostream>

#include <vsg/core/ref_ptr.h>

namespace vsg
{
    extern std::ostream& notice_stream();
    extern std::ostream& error_stream();

    template<class T, typename R, R validValue>
    class Result
    {
    public:

        Result(R result) : _printMessageOnError(true), _result(result) {}
        Result(const std::string& message, R result) : _printMessageOnError(true), _result(result), _message(message) {}
        explicit Result(ref_ptr<T> ptr) : _printMessageOnError(false), _result(validValue), _ptr(ptr) {}

        Result(const Result& rhs) :
            _printMessageOnError(rhs._printMessageOnError),
            _result(rhs._result),
            _message(rhs._message),
            _ptr(rhs._ptr)
        {
            rhs._printMessageOnError = false;
        }

        Result& operator = (const Result& rhs)
        {
            _printMessageOnError = rhs._printMessageOnError;
            _result = rhs._result;
            _message = rhs._message;
            _ptr = rhs._ptr;
            rhs._printMessageOnError = false;
            return *this;
        }

        ~Result()
        {
            if (_result!=validValue && _printMessageOnError)
            {
                if (_message.empty()) error_stream()<<"Warning: unhandled error value : "<<_result<<std::endl;
                else error_stream()<<_message<<std::endl;
            }
        }

        R result() { _printMessageOnError = false; return _result; }

        const std::string& message() { _printMessageOnError = false; return _message; }

        bool valid() const { return _ptr.valid(); }

        ref_ptr<T> object() { return _ptr; }

        operator ref_ptr<T> () { return _ptr; }

    protected:

        mutable bool    _printMessageOnError;
        R               _result;
        std::string     _message;
        ref_ptr<T>      _ptr;
    };

}
