#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/io/stream.h>

#include <mutex>
#include <sstream>

namespace vsg
{

    /// pure virtual Logger base class that provides extensible message logging facilities
    class VSG_DECLSPEC Logger : public Object
    {
    public:
        Logger();

        Logger(const Logger&) = delete;

        enum Level
        {
            ALL = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            OFF
        };

        Level level = INFO;

        void debug(const std::string& message)
        {
            if (level > DEBUG) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            debug_implementation(message);
        }

        void info(const std::string& message)
        {
            if (level > INFO) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(message);
        }

        void warn(const std::string& message)
        {
            if (level > WARN) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(message);
        }

        void error(const std::string& message)
        {
            if (level > ERROR) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(message);
        }

        template<typename... Args>
        void debug(Args&&... args)
        {
            if (level > INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            debug_implementation(_stream.str());
        }

        template<typename... Args>
        void info(Args&&... args)
        {
            if (level > INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            info_implementation(_stream.str());
        }

        template<typename... Args>
        void warn(Args&&... args)
        {
            if (level > INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            info_implementation(_stream.str());
        }

        template<typename... Args>
        void error(Args&&... args)
        {
            if (level > INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            info_implementation(_stream.str());
        }

    protected:
        virtual ~Logger();

        std::mutex _mutex;
        std::ostringstream _stream;

        virtual void debug_implementation(const std::string& message) = 0;
        virtual void info_implementation(const std::string& message) = 0;
        virtual void warn_implementation(const std::string& message) = 0;
        virtual void error_implementation(const std::string& message) = 0;
    };

    /// logger() singleton, defaults to using vsg::StdLogger
    extern VSG_DECLSPEC ref_ptr<Logger>& logger();

    /// write debug message using ostringstream to convert parameters to a string that is passed to the current vsg::logger() logger.
    /// i.e. debug("array.size() = ", array.size());
    template<typename... Args>
    void debug(Args&&... args)
    {
        logger()->debug(args...);
    }

    /// write simple debug std::string message to the current vsg::logger() logger.
    inline void debug(const std::string& str)
    {
        logger()->debug(str);
    }

    /// write info message using ostringstream to convert parameters to a string that is passed to the current vsg::logger() logger.
    /// i.e. info("vertex = ", vsg::vec3(x,y,z));
    template<typename... Args>
    void info(Args&&... args)
    {
        logger()->info(args...);
    }

    /// write simple info std::string message to the current vsg::logger() logger.
    inline void info(const std::string& str)
    {
        logger()->info(str);
    }

    /// write warn message using ostringstream to convert parameters to a string that is passed to the current vsg::logger() logger.
    template<typename... Args>
    void warn(Args&&... args)
    {
        logger()->warn(args...);
    }

    /// write simple warn std::string message to the current vsg::logger() logger.
    inline void warn(const std::string& str)
    {
        logger()->warn(str);
    }

    /// write warn error using ostringstream to convert parameters to a string that is passed to the current vsg::logger() logger.
    template<typename... Args>
    void error(Args&&... args)
    {
        logger()->error(args...);
    }

    /// write simple error std::string message to the current vsg::logger() logger.
    inline void error(const std::string& str)
    {
        logger()->error(str);
    }

    /// default Logger that sends debug and info messages to std:cout, and warn and errpr messages to std::cert
    class VSG_DECLSPEC StdLogger : public Inherit<Logger, StdLogger>
    {
    public:
        StdLogger();

    protected:
        void debug_implementation(const std::string& message) override;
        void info_implementation(const std::string& message) override;
        void warn_implementation(const std::string& message) override;
        void error_implementation(const std::string& message) override;
    };

    /// Logger that ignores all messages
    class VSG_DECLSPEC NullLogger : public vsg::Inherit<vsg::Logger, NullLogger>
    {
    public:
        NullLogger();

        void debug_implementation(const std::string&) override;
        void info_implementation(const std::string&) override;
        void warn_implementation(const std::string&) override;
        void error_implementation(const std::string&) override;
    };

} // namespace vsg
