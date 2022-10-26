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
}

ref_ptr<Logger>& Logger::instance()
{
    static ref_ptr<Logger> s_logger = StdLogger::create();
    //static ref_ptr<Logger> s_logger = ThreadLogger::create();
    return s_logger;
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
    std::cout.flush();
    std::cerr.flush();
}

void StdLogger::debug_implementation(const std::string_view& message)
{
    std::cout << debugPrefix << message << std::endl;
}

void StdLogger::info_implementation(const std::string_view& message)
{
    std::cout << infoPrefix << message << std::endl;
}

void StdLogger::warn_implementation(const std::string_view& message)
{
    std::cerr << warnPrefix << message << std::endl;
}

void StdLogger::error_implementation(const std::string_view& message)
{
    std::cerr << errorPrefix << message << std::endl;
}

void StdLogger::fatal_implementation(const std::string_view& message)
{
    std::cerr << fatalPrefix << message << std::endl;
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
    std::cout.flush();
    std::cerr.flush();
}

void ThreadLogger::setThreadPrefix(std::thread::id id, const std::string& str)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _threadPrefixes[id] = str;
}

void ThreadLogger::print_id(std::ostream& out, std::thread::id id)
{
    if (auto itr = _threadPrefixes.find(id); itr != _threadPrefixes.end())
    {
        out << itr->second;
    }
    else
    {
        out << "thread::id = " << id << " | ";
    }
}

void ThreadLogger::debug_implementation(const std::string_view& message)
{
    print_id(std::cout, std::this_thread::get_id());
    std::cout << debugPrefix << message << std::endl;
}

void ThreadLogger::info_implementation(const std::string_view& message)
{
    print_id(std::cout, std::this_thread::get_id());
    std::cout << infoPrefix << message << std::endl;
}

void ThreadLogger::warn_implementation(const std::string_view& message)
{
    print_id(std::cout, std::this_thread::get_id());
    std::cerr << warnPrefix << message << std::endl;
}

void ThreadLogger::error_implementation(const std::string_view& message)
{
    print_id(std::cout, std::this_thread::get_id());
    std::cerr << errorPrefix << message << std::endl;
}

void ThreadLogger::fatal_implementation(const std::string_view& message)
{
    print_id(std::cout, std::this_thread::get_id());
    std::cerr << fatalPrefix << message << std::endl;
    throw vsg::Exception{std::string(message)};
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
    throw vsg::Exception{std::string(message)};
}
