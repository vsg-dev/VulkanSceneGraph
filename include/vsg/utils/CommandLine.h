#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <sstream>
#include <typeinfo>
#include <vector>

namespace vsg
{

class CommandLine
{
public:

    CommandLine(int* argc, char** argv) : _argc(argc), _argv(argv) {}

    template<typename T>
    bool read(int& i, T& value)
    {
        if (i>=*_argc) return false;

        _istr.clear();
        _istr.str(_argv[i]);
        _istr >> value;

        ++i;

        return (!_istr.fail());
    }

    void remove(int i, int num)
    {
        if (i>=*_argc) return;

        int source = i+num;
        if (source>=*_argc)
        {
            // removed section is at end of argv so just reset argc to i
            *_argc = i;
            return;
        }

        // shift all the remaining entries down to fill the removed space
        for(; source<*_argc; ++i, ++source)
        {
            _argv[i] = _argv[source];
        }

        *_argc -= num;
    }

    template<typename T> std::string type_name(const T&) { return vsg::make_string(" <",typeid(T).name(),">"); }
    std::string type_name(const std::string&) { return " <string>"; }
    std::string type_name(const int&) { return " <int>"; }
    std::string type_name(const unsigned int&) { return " <uint>"; }
    std::string type_name(const float&) { return " <float>"; }
    std::string type_name(const double&) { return "<double>"; }

    template< typename ... Args>
    bool read(const std::string& match, Args& ... args)
    {
        for(int i=0; i< *_argc; ++i)
        {
            if (match == _argv[i])
            {
                int start = i;
                ++i;

                // match any parameters
                bool result = ( read(i, args) && ... );

                if (result)
                {
                    remove(start, i-start);
                }
                else
                {
                    std::string parameters = ( match + ... + type_name(args));
                    std::string errorMessage = std::string("Failed to match command line required parameters for ") + parameters;
                    _errorMessages.push_back(errorMessage);
                }

                return result;
            }
        }
        return false;
    }

    template< typename ... Args>
    bool read(std::initializer_list<std::string> matches, Args& ... args)
    {
        bool result = false;
        for(auto str : matches) result = read(str, args...) | result;
        return result;
    }

    template< typename T, typename ... Args>
    T value(T defaultValue, const std::string& match, Args& ... args)
    {
        T value{defaultValue};
        read(match, args..., value);
        return value;
    }

    template< typename T, typename ... Args>
    T value(T defaultValue, std::initializer_list<std::string> matches, Args& ... args)
    {
        T value{defaultValue};
        read(matches, args..., value);
        return value;
    }


    using Messages = std::vector<std::string>;
    bool errors() const { return !_errorMessages.empty(); }

    Messages& getErrorMessages() { return _errorMessages; }
    const Messages& getErrorMessages() const { return _errorMessages; }

    int writeErrorMessages(std::ostream& out) const
    {
        if (_errorMessages.empty()) return 1;
        for(auto message : _errorMessages) out << message << std::endl;
        return 0;
    }

protected:
    int*                _argc;
    char**              _argv;
    std::istringstream  _istr;
    Messages            _errorMessages;
};


} // vsg
