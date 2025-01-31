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
#include <string_view>
#include <thread>

namespace vsg
{

    /// thread safe, pure virtual Logger base class that provides extensible message logging facilities
    class VSG_DECLSPEC Logger : public Object
    {
    public:
        Logger();

        Logger(const Logger& rhs);

        enum Level
        {
            LOGGER_ALL = 0,
            LOGGER_DEBUG = 1,
            LOGGER_INFO = 2,
            LOGGER_WARN = 3,
            LOGGER_ERROR = 4,
            LOGGER_FATAL = 5,
            LOGGER_OFF = 6
        };

        Level level = LOGGER_INFO;

        /// Logger singleton, defaults to using vsg::StdLogger
        static ref_ptr<Logger>& instance();

        /// redirect std::cout and std::cerr output to vsg::Logger at level LOGGER_INFO and LOGGER_ERROR respectively.
        virtual void redirect_std();

        virtual void flush() {}

        inline void debug(char* message) { debug(std::string_view(message)); }
        inline void debug(const char* message) { debug(std::string_view(message)); }
        inline void debug(std::string& message) { debug(std::string_view(message)); }
        inline void debug(const std::string& message) { debug(std::string_view(message)); }

        void debug(const std::string_view& str)
        {
            if (level > LOGGER_DEBUG) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            debug_implementation(str);
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

        inline void info(char* message) { info(std::string_view(message)); }
        inline void info(const char* message) { info(std::string_view(message)); }
        inline void info(std::string& message) { info(std::string_view(message)); }
        inline void info(const std::string& message) { info(std::string_view(message)); }

        void info(const std::string_view& str)
        {
            if (level > LOGGER_INFO) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            info_implementation(str);
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

        inline void warn(char* message) { warn(std::string_view(message)); }
        inline void warn(const char* message) { warn(std::string_view(message)); }
        inline void warn(std::string& message) { warn(std::string_view(message)); }
        inline void warn(const std::string& message) { warn(std::string_view(message)); }

        void warn(const std::string_view& str)
        {
            if (level > LOGGER_WARN) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            warn_implementation(str);
        }

        template<typename... Args>
        void warn(Args&&... args)
        {
            if (level > LOGGER_WARN) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            warn_implementation(_stream.str());
        }

        inline void error(char* message) { error(std::string_view(message)); }
        inline void error(const char* message) { error(std::string_view(message)); }
        inline void error(std::string& message) { error(std::string_view(message)); }
        inline void error(const std::string& message) { error(std::string_view(message)); }

        void error(const std::string_view& str)
        {
            if (level > LOGGER_ERROR) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            error_implementation(str);
        }

        template<typename... Args>
        void error(Args&&... args)
        {
            if (level > LOGGER_ERROR) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            error_implementation(_stream.str());
        }

        inline void fatal(char* message) { fatal(std::string_view(message)); }
        inline void fatal(const char* message) { fatal(std::string_view(message)); }
        inline void fatal(std::string& message) { fatal(std::string_view(message)); }
        inline void fatal(const std::string& message) { fatal(std::string_view(message)); }

        void fatal(const std::string_view& str)
        {
            if (level > LOGGER_FATAL) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            fatal_implementation(str);
        }

        template<typename... Args>
        void fatal(Args&&... args)
        {
            if (level > LOGGER_FATAL) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            fatal_implementation(_stream.str());
        }

        using PrintToStreamFunction = std::function<void(std::ostream&)>;

        /// thread safe access to stream for writing debug output.
        void debug_stream(PrintToStreamFunction print);

        /// thread safe access to stream for writing info output.
        void info_stream(PrintToStreamFunction print);

        /// thread safe access to stream for writing warn output.
        void warn_stream(PrintToStreamFunction print);

        /// thread safe access to stream for writing error output.
        void error_stream(PrintToStreamFunction print);

        /// thread safe access to stream for writing fatal output and throwing vsg::Exception
        void fatal_stream(PrintToStreamFunction print);

        /// pass message to debug()/info()/warn()/error() based on specified level
        inline void log(Level msg_level, char* message) { log(msg_level, std::string_view(message)); }
        inline void log(Level msg_level, const char* message) { log(msg_level, std::string_view(message)); }
        inline void log(Level msg_level, std::string& message) { log(msg_level, std::string_view(message)); }
        inline void log(Level msg_level, const std::string& message) { log(msg_level, std::string_view(message)); }

        void log(Level msg_level, const std::string_view& message);

        /// pass message to debug()/info()/warn()/error() based on specified level
        template<typename... Args>
        void log(Level msg_level, Args... args)
        {
            if (level > msg_level) return;

            std::scoped_lock<std::mutex> lock(_mutex);
            _stream.str({});
            _stream.clear();
            (_stream << ... << args);

            switch (msg_level)
            {
            case (LOGGER_DEBUG): debug_implementation(_stream.str()); break;
            case (LOGGER_INFO): info_implementation(_stream.str()); break;
            case (LOGGER_WARN): warn_implementation(_stream.str()); break;
            case (LOGGER_ERROR): error_implementation(_stream.str()); break;
            case (LOGGER_FATAL): fatal_implementation(_stream.str()); break;
            default: break;
            }
        }

        /// thread safe access to stream for writing error output.
        void log_stream(Level msg_level, PrintToStreamFunction print);

    protected:
        virtual ~Logger();

        std::mutex _mutex;
        std::ostringstream _stream;

        std::unique_ptr<std::streambuf> _override_cout;
        std::unique_ptr<std::streambuf> _override_cerr;
        std::streambuf* _original_cout = nullptr;
        std::streambuf* _original_cerr = nullptr;

        virtual void debug_implementation(const std::string_view& message) = 0;
        virtual void info_implementation(const std::string_view& message) = 0;
        virtual void warn_implementation(const std::string_view& message) = 0;
        virtual void error_implementation(const std::string_view& message) = 0;
        virtual void fatal_implementation(const std::string_view& message) = 0;
    };
    VSG_type_name(vsg::Logger);

    /// write debug message to the current vsg::Logger::instance().
    /// i.e. debug("array.size() = ", array.size());
    template<typename... Args>
    void debug(Args&&... args)
    {
        Logger::instance()->debug(std::forward<Args>(args)...);
    }

    /// thread safe access to stream for writing debug output.
    inline void debug_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->debug_stream(print);
    }

    /// write info message to the current vsg::Logger::instance().
    /// i.e. info("vertex = ", vsg::vec3(x,y,z));
    template<typename... Args>
    void info(Args&&... args)
    {
        Logger::instance()->info(std::forward<Args>(args)...);
    }

    /// thread safe access to stream for writing info output.
    inline void info_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->info_stream(print);
    }

    /// write warn message to the current vsg::Logger::instance().
    template<typename... Args>
    void warn(Args&&... args)
    {
        Logger::instance()->warn(std::forward<Args>(args)...);
    }

    /// thread safe access to stream for writing warn output.
    inline void warn_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->warn_stream(print);
    }

    /// write error message to the current vsg::Logger::instance().
    template<typename... Args>
    void error(Args&&... args)
    {
        Logger::instance()->error(std::forward<Args>(args)...);
    }

    /// thread safe access to stream for writing error output.
    inline void error_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->error_stream(print);
    }

    /// write fatal message to the current vsg::Logger::instance().
    template<typename... Args>
    void fatal(Args&&... args)
    {
        Logger::instance()->fatal(std::forward<Args>(args)...);
    }

    /// thread safe access to stream for writing fatal output.
    inline void fatal_stream(Logger::PrintToStreamFunction print)
    {
        Logger::instance()->fatal_stream(print);
    }

    /// write message at specified level to the current vsg::Logger::instance() logger.
    template<typename... Args>
    void log(Logger::Level msg_level, Args&&... args)
    {
        Logger::instance()->log(msg_level, std::forward<Args>(args)...);
    }

    /// thread safe access to stream for writing output for specified Logger level.
    inline void log_stream(Logger::Level msg_level, Logger::PrintToStreamFunction print)
    {
        Logger::instance()->log_stream(msg_level, print);
    }

    /// default Logger that sends debug and info messages to std:cout, and warn and error messages to std::cerr
    class VSG_DECLSPEC StdLogger : public Inherit<Logger, StdLogger>
    {
    public:
        StdLogger();

        std::string debugPrefix = "debug: ";
        std::string infoPrefix = "info: ";
        std::string warnPrefix = "Warning: ";
        std::string errorPrefix = "ERROR: ";
        std::string fatalPrefix = "FATAL: ";

        void flush() override;

    protected:
        void debug_implementation(const std::string_view& message) override;
        void info_implementation(const std::string_view& message) override;
        void warn_implementation(const std::string_view& message) override;
        void error_implementation(const std::string_view& message) override;
        void fatal_implementation(const std::string_view& message) override;
    };
    VSG_type_name(vsg::StdLogger);

    /// Logger that prefixes message lines with user defined thread prefix, or std::thread::id when none is assigned.
    /// To use the ThreadLogger use:
    ///     vsg::Logger::instance() = ThreadLogger::create();
    class VSG_DECLSPEC ThreadLogger : public vsg::Inherit<vsg::Logger, ThreadLogger>
    {
    public:
        ThreadLogger();

        /// assign prefix for std::thread::id. The id can be obtained from std::thread::get_id() i.e. thread->get_id() or this_thread::get_id().
        void setThreadPrefix(std::thread::id id, const std::string& str);

        std::string debugPrefix = "debug: ";
        std::string infoPrefix = "info: ";
        std::string warnPrefix = "Warning: ";
        std::string errorPrefix = "ERROR: ";
        std::string fatalPrefix = "FATAL: ";

        void flush() override;

    protected:
        void print_id(FILE* out, std::thread::id id);

        void debug_implementation(const std::string_view& message) override;
        void info_implementation(const std::string_view& message) override;
        void warn_implementation(const std::string_view& message) override;
        void error_implementation(const std::string_view& message) override;
        void fatal_implementation(const std::string_view& message) override;

        std::map<std::thread::id, std::string> _threadPrefixes;
    };
    VSG_type_name(vsg::ThreadLogger);

    /// Logger that ignores all messages
    /// To use the NullLogger use:
    ///     vsg::Logger::instance() = NullLogger::create();
    class VSG_DECLSPEC NullLogger : public Inherit<Logger, NullLogger>
    {
    public:
        NullLogger();

    protected:
        void debug_implementation(const std::string_view&) override;
        void info_implementation(const std::string_view&) override;
        void warn_implementation(const std::string_view&) override;
        void error_implementation(const std::string_view&) override;
        void fatal_implementation(const std::string_view&) override;
    };
    VSG_type_name(vsg::NullLogger);

} // namespace vsg
