#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>
#include <vsg/core/type_name.h>
#include <vsg/io/Options.h>
#include <vsg/io/stream.h>

#include <vector>

namespace vsg
{

    template<typename T>
    constexpr std::size_t type_num_elements(T) noexcept { return 1; }
    template<typename T>
    constexpr std::size_t type_num_elements(const t_vec2<T>&) noexcept { return 2; }
    template<typename T>
    constexpr std::size_t type_num_elements(const t_vec3<T>&) noexcept { return 3; }
    template<typename T>
    constexpr std::size_t type_num_elements(const t_vec4<T>&) noexcept { return 4; }
    template<typename T>
    constexpr std::size_t type_num_elements(const t_mat4<T>&) noexcept { return 16; }
    template<typename T, typename R>
    constexpr std::size_t type_num_elements(const std::pair<T, R>&) noexcept { return 2; }

    // forward declare
    class Options;

    class VSG_DECLSPEC CommandLine
    {
    public:
        CommandLine(int* argc, char** argv);

        int& argc() { return *_argc; }
        char** argv() { return _argv; }

        char* operator[](int i) { return _argv[i]; }

        template<typename T>
        bool read(int& i, T& v)
        {
            const int num_args = *_argc;
            if (i >= num_args) return false;

            if constexpr (std::is_same_v<T, std::string>)
            {
                v = _argv[i++];
                return true;
            }
            else
            {
                std::size_t num_elements = type_num_elements(v);

                _istr.clear();
                if (num_elements == 1)
                {
                    _istr.str(_argv[i]);
                    ++i;
                }
                else
                {
                    std::string str;
                    for (; num_elements > 0 && i < num_args; --num_elements, ++i)
                    {
                        str += ' ';
                        str += _argv[i];
                    }

                    _istr.str(str);
                }
                _istr >> v;

                return (!_istr.fail());
            }
        }

        void remove(int i, int num)
        {
            if (i >= *_argc) return;

            int source = i + num;
            if (source >= *_argc)
            {
                // removed section is at end of argv so just reset argc to i
                *_argc = i;
                return;
            }

            // shift all the remaining entries down to fill the removed space
            for (; source < *_argc; ++i, ++source)
            {
                _argv[i] = _argv[source];
            }

            *_argc -= num;
        }

        template<typename... Args>
        bool read(const std::string& match, Args&... args)
        {
            for (int i = 1; i < *_argc; ++i)
            {
                if (match == _argv[i])
                {
                    int start = i;
                    ++i;

                    // match any parameters
                    bool result = (read(i, args) && ...);

                    if (result)
                    {
                        remove(start, i - start);
                    }
                    else
                    {
                        std::string parameters = ((match + " ") + ... + type_name(args));
                        std::string errorMessage = std::string("Failed to match command line required parameters for ") + parameters;
                        _errorMessages.push_back(errorMessage);
                    }

                    return result;
                }
            }
            return false;
        }

        template<typename... Args>
        bool read(std::initializer_list<std::string> matches, Args&... args)
        {
            bool result = false;
            for (auto str : matches) result = read(str, args...) | result;
            return result;
        }

        template<typename T, typename... Args>
        T value(T defaultValue, const std::string& match, Args&... args)
        {
            T v{defaultValue};
            read(match, args..., v);
            return v;
        }

        template<typename T, typename... Args>
        T value(T defaultValue, std::initializer_list<std::string> matches, Args&... args)
        {
            T v{defaultValue};
            read(matches, args..., v);
            return v;
        }

        template<typename T>
        bool readAndAssign(const std::string& match, Options* options)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                if (options && read(std::string("--") + match))
                {
                    options->setValue(match, true);
                    return true;
                }
            }
            else
            {
                T v;
                if (options && read(std::string("--") + match, v))
                {
                    options->setValue(match, v);
                    return true;
                }
            }
            return false;
        }

        bool read(Options* options);

        using Messages = std::vector<std::string>;
        bool errors() const { return !_errorMessages.empty(); }

        Messages& getErrorMessages() { return _errorMessages; }
        const Messages& getErrorMessages() const { return _errorMessages; }

        int writeErrorMessages(std::ostream& out) const
        {
            if (_errorMessages.empty()) return 1;
            for (auto message : _errorMessages) out << message << std::endl;
            return 0;
        }

    protected:
        int* _argc;
        char** _argv;
        std::istringstream _istr;
        Messages _errorMessages;
    };

} // namespace vsg
