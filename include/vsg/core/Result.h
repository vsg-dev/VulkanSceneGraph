#pragma once

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
        Result(T* ptr) : _printMessageOnError(false), _result(validValue), _ptr(ptr) {}
        Result(ref_ptr<T> ptr) : _printMessageOnError(false), _result(validValue), _ptr(ptr) {}

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

        ref_ptr<T> object() { return _ptr; }

        operator ref_ptr<T> () { return _ptr; }

    protected:

        mutable bool    _printMessageOnError;
        R               _result;
        std::string     _message;
        ref_ptr<T>      _ptr;
    };

}
