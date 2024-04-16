/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>

#include <iostream>

using namespace vsg;

namespace vsg
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // intercept_streambuf takes std::cout/cerr output and redirects it the Logger
    //

    class intercept_streambuf : public std::streambuf
    {
    public:
        explicit intercept_streambuf(Logger* in_logger, Logger::Level in_level) :
            logger(in_logger),
            level(in_level)
        {
        }

        Logger* logger = nullptr;
        Logger::Level level = Logger::LOGGER_INFO;

        std::streamsize xsputn(const char_type* s, std::streamsize n) override
        {
            std::scoped_lock<std::mutex> lock(_mutex);
            _line.append(s, static_cast<std::size_t>(n));
            return n;
        }

        std::streambuf::int_type overflow(std::streambuf::int_type c) override
        {
            std::scoped_lock<std::mutex> lock(_mutex);
            if (c=='\n')
            {
                logger->log(level, _line);
                _line.clear();
            }
            else
            {
                _line.push_back(static_cast<char>(c));
            }
            return c;
        }

    protected:
        std::string _line;
        std::mutex _mutex;
    };

} // namespace vsg

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Logger
//
Logger::Logger()
{
    // level = LOGGER_ALL; // print all messages
    // level = LOGGER_DEBUG; // print debugs and above messages
    // level = LOGGER_INFO; // default, print info and above messages
    // level = LOGGER_WARN; // print warn and above messages
    // level = LOGGER_ERROR; // print error and above messages
    // level = LOGGER_FATAL; // print error and above messages
}

Logger::Logger(const Logger& rhs) :
    Logger()
{
    level = rhs.level;
}

Logger::~Logger()
{
    flush();

    if (_original_cout) std::cout.rdbuf(_original_cout);
    if (_original_cerr) std::cerr.rdbuf(_original_cerr);
}

ref_ptr<Logger>& Logger::instance()
{
    static ref_ptr<Logger> s_logger = StdLogger::create();
    //static ref_ptr<Logger> s_logger = ThreadLogger::create();
    return s_logger;
}

void Logger::redirect_std()
{
    _override_cout.reset(new intercept_streambuf(this, LOGGER_INFO));
    _original_cout = std::cout.rdbuf(_override_cout.get());

    _override_cerr.reset(new intercept_streambuf(this, LOGGER_ERROR));
    _original_cerr = std::cerr.rdbuf(_override_cerr.get());
}

void Logger::debug_stream(PrintToStreamFunction print)
{
    if (level > LOGGER_DEBUG) return;

    std::scoped_lock<std::mutex> lock(_mutex);
    _stream.str({});
    _stream.clear();

    print(_stream);

    debug_implementation(_stream.str());
}

void Logger::info_stream(PrintToStreamFunction print)
{
    if (level > LOGGER_INFO) return;

    std::scoped_lock<std::mutex> lock(_mutex);
    _stream.str({});
    _stream.clear();

    print(_stream);

    info_implementation(_stream.str());
}

void Logger::warn_stream(PrintToStreamFunction print)
{
    if (level > LOGGER_WARN) return;

    std::scoped_lock<std::mutex> lock(_mutex);
    _stream.str({});
    _stream.clear();

    print(_stream);

    warn_implementation(_stream.str());
}

void Logger::error_stream(PrintToStreamFunction print)
{
    if (level > LOGGER_ERROR) return;

    std::scoped_lock<std::mutex> lock(_mutex);
    _stream.str({});
    _stream.clear();

    print(_stream);

    error_implementation(_stream.str());
}

void Logger::fatal_stream(PrintToStreamFunction print)
{
    if (level > LOGGER_FATAL) return;

    std::scoped_lock<std::mutex> lock(_mutex);
    _stream.str({});
    _stream.clear();

    print(_stream);

    fatal_implementation(_stream.str());
}

void Logger::log(Level msg_level, const std::string_view& message)
{
    if (level > msg_level) return;
    std::scoped_lock<std::mutex> lock(_mutex);

    switch (msg_level)
    {
    case (LOGGER_DEBUG): debug_implementation(message); break;
    case (LOGGER_INFO): info_implementation(message); break;
    case (LOGGER_WARN): warn_implementation(message); break;
    case (LOGGER_ERROR): error_implementation(message); break;
    case (LOGGER_FATAL): fatal_implementation(message); break;
    default: break;
    }
}

void Logger::log_stream(Level msg_level, PrintToStreamFunction print)
{
    if (level > msg_level) return;

    std::scoped_lock<std::mutex> lock(_mutex);
    _stream.str({});
    _stream.clear();

    print(_stream);

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

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// StdLogger
//
StdLogger::StdLogger()
{
}

void StdLogger::flush()
{
    fflush(stdout);
    fflush(stderr);
}

void StdLogger::debug_implementation(const std::string_view& message)
{
    fprintf(stdout, "%s%.*s\n", debugPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void StdLogger::info_implementation(const std::string_view& message)
{
    fprintf(stdout, "%s%.*s\n", infoPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void StdLogger::warn_implementation(const std::string_view& message)
{
    fprintf(stderr, "%s%.*s\n", warnPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void StdLogger::error_implementation(const std::string_view& message)
{
    fprintf(stderr, "%s%.*s\n", errorPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void StdLogger::fatal_implementation(const std::string_view& message)
{
    fprintf(stderr, "%s%.*s\n", fatalPrefix.c_str(), static_cast<int>(message.length()), message.data());
    throw vsg::Exception{std::string(message)};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ThreadLogger
//
ThreadLogger::ThreadLogger()
{
}

void ThreadLogger::flush()
{
    fflush(stdout);
    fflush(stderr);
}

void ThreadLogger::setThreadPrefix(std::thread::id id, const std::string& str)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _threadPrefixes[id] = str;
}

void ThreadLogger::print_id(FILE* out, std::thread::id id)
{
    if (auto itr = _threadPrefixes.find(id); itr != _threadPrefixes.end())
    {
        fprintf(out, "%s", itr->second.c_str());
    }
    else
    {
        fprintf(out, "thread::id = %s | ", itr->second.c_str());
    }
}

void ThreadLogger::debug_implementation(const std::string_view& message)
{
    print_id(stdout, std::this_thread::get_id());
    fprintf(stdout, "%s%.*s\n", debugPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void ThreadLogger::info_implementation(const std::string_view& message)
{
    print_id(stdout, std::this_thread::get_id());
    fprintf(stdout, "%s%.*s\n", infoPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void ThreadLogger::warn_implementation(const std::string_view& message)
{
    print_id(stderr, std::this_thread::get_id());
    fprintf(stderr, "%s%.*s\n", warnPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void ThreadLogger::error_implementation(const std::string_view& message)
{
    print_id(stderr, std::this_thread::get_id());
    fprintf(stderr, "%s%.*s\n", errorPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

void ThreadLogger::fatal_implementation(const std::string_view& message)
{
    print_id(stderr, std::this_thread::get_id());
    fprintf(stderr, "%s%.*s\n", fatalPrefix.c_str(), static_cast<int>(message.length()), message.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NullLogger
//
NullLogger::NullLogger()
{
    level = LOGGER_OFF;
}

void NullLogger::debug_implementation(const std::string_view&)
{
}
void NullLogger::info_implementation(const std::string_view&)
{
}
void NullLogger::warn_implementation(const std::string_view&)
{
}
void NullLogger::error_implementation(const std::string_view&)
{
}
void NullLogger::fatal_implementation(const std::string_view& message)
{
    throw Exception{std::string(message)};
}
