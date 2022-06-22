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

namespace vsg
{

    /// pure virtual Logger base class that provides extensible message logging facilities
    class VSG_DECLSPEC Logger : public Object
    {
    public:
        Logger();

        Logger(const Logger& rhs);

        enum Level
        {
            LOGGER_ALL = 0,
            LOGGER_DEBUG,
            LOGGER_INFO,
            LOGGER_WARN,
            LOGGER_ERROR,
            LOGGER_OFF
        };

        Level level = LOGGER_INFO;

        void debug(const std::string& message)
        {
            if (level > LOGGER_DEBUG) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            debug_implementation(message);
        }

        void info(const std::string& message)
        {
            if (level > LOGGER_INFO) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(message);
        }

        void warn(const std::string& message)
        {
            if (level > LOGGER_WARN) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(message);
        }

        void error(const std::string& message)
        {
            if (level > LOGGER_ERROR) return;
            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(message);
        }

        template<typename... Args>
        void debug(Args&&... args)
        {
            if (level > LOGGER_DEBUG) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            debug_implementation(_stream.str());
        }

        template<typename... Args>
        void info(Args&&... args)
        {
            if (level > LOGGER_INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            info_implementation(_stream.str());
        }

        template<typename... Args>
        void warn(Args&&... args)
        {
            if (level > LOGGER_WARN) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            info_implementation(_stream.str());
        }

        template<typename... Args>
        void error(Args&&... args)
        {
            if (level > LOGGER_ERROR) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            info_implementation(_stream.str());
        }

        using PrintToStreamFunction = std::function<void(std::ostream&)>;

        /// thread safe access to stream for writing debug output.
        void debug_stream(PrintToStreamFunction print)
        {
            if (level > LOGGER_DEBUG) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();

            print(_stream);

            debug_implementation(_stream.str());
        }

        /// thread safe access to stream for writing info output.
        void info_stream(PrintToStreamFunction print)
        {
            if (level > LOGGER_INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();

            print(_stream);

            info_implementation(_stream.str());
        }

        /// thread safe access to stream for writing warn output.
        void warn_stream(PrintToStreamFunction print)
        {
            if (level > LOGGER_WARN) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();

            print(_stream);

            warn_implementation(_stream.str());
        }

        /// thread safe access to stream for writing error output.
        void error_stream(PrintToStreamFunction print)
        {
            if (level > LOGGER_ERROR) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();

            print(_stream);

            error_implementation(_stream.str());
        }

        /// Logger singleton, defaults to using vsg::StdLogger
        static ref_ptr<Logger>& instance();

    protected:
        virtual ~Logger();

        std::mutex _mutex;
        std::ostringstream _stream;

        virtual void debug_implementation(const std::string& message) = 0;
        virtual void info_implementation(const std::string& message) = 0;
        virtual void warn_implementation(const std::string& message) = 0;
        virtual void error_implementation(const std::string& message) = 0;
    };


    /// write debug message using ostringstream to convert parameters to a string that is passed to the current vsg::Logger::instance() logger.
    /// i.e. debug("array.size() = ", array.size());
    template<typename... Args>
    void debug(Args&&... args)
    {
        Logger::instance()->debug(args...);
    }

    /// write simple debug std::string message to the current vsg::Logger::instance() logger.
    inline void debug(const std::string& str)
    {
        Logger::instance()->debug(str);
    }

    /// thread safe access to stream for writing debug output.
    inline void debug_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->debug_stream(print);
    }

    /// write info message using ostringstream to convert parameters to a string that is passed to the current vsg::Logger::instance() logger.
    /// i.e. info("vertex = ", vsg::vec3(x,y,z));
    template<typename... Args>
    void info(Args&&... args)
    {
        Logger::instance()->info(args...);
    }

    /// write simple info std::string message to the current vsg::Logger::instance() logger.
    inline void info(const std::string& str)
    {
        Logger::instance()->info(str);
    }

    /// thread safe access to stream for writing info output.
    inline void info_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->info_stream(print);
    }

    /// write warn message using ostringstream to convert parameters to a string that is passed to the current vsg::Logger::instance() logger.
    template<typename... Args>
    void warn(Args&&... args)
    {
        Logger::instance()->warn(args...);
    }

    /// write simple warn std::string message to the current vsg::Logger::instance() logger.
    inline void warn(const std::string& str)
    {
        Logger::instance()->warn(str);
    }

    /// thread safe access to stream for writing warn output.
    inline void warn_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->warn_stream(print);
    }

    /// write warn error using ostringstream to convert parameters to a string that is passed to the current vsg::Logger::instance() logger.
    template<typename... Args>
    void error(Args&&... args)
    {
        Logger::instance()->error(args...);
    }

    /// write simple error std::string message to the current vsg::Logger::instance() logger.
    inline void error(const std::string& str)
    {
        Logger::instance()->error(str);
    }

    inline void error_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->error_stream(print);
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
    class VSG_DECLSPEC NullLogger : public Inherit<Logger, NullLogger>
    {
    public:
        NullLogger();

        void debug_implementation(const std::string&) override;
        void info_implementation(const std::string&) override;
        void warn_implementation(const std::string&) override;
        void error_implementation(const std::string&) override;
    };

} // namespace vsg
