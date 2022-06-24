
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
//    level = LOGGER_ALL; // print all messages
//    level = LOGGER_DEBUG; // print debugs and above messages
//    level = LOGGER_INFO; // default, print info and above messages
//    level = LOGGER_WARN; // print warn and above messages
//    level = LOGGER_ERROR; // print error and above messages
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

void Logger::log(Level msg_level, std::string_view message)
{
    if (level > msg_level) return;
    std::scoped_lock<std::mutex> lock(_mutex);

    switch(msg_level)
    {
        case(LOGGER_DEBUG): debug_implementation(message); break;
        case(LOGGER_INFO): info_implementation(message); break;
        case(LOGGER_WARN): warn_implementation(message); break;
        case(LOGGER_ERROR): error_implementation(message); break;
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

    switch(msg_level)
    {
        case(LOGGER_DEBUG): debug_implementation(_stream.str()); break;
        case(LOGGER_INFO): info_implementation(_stream.str()); break;
        case(LOGGER_WARN): warn_implementation(_stream.str()); break;
        case(LOGGER_ERROR): error_implementation(_stream.str()); break;
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

void StdLogger::debug_implementation(std::string_view message)
{
    std::cout << "debug: " << message << '\n';
}

void StdLogger::info_implementation(std::string_view message)
{
    std::cout << "info: " << message << '\n';
}

void StdLogger::warn_implementation(std::string_view message)
{
    std::cerr << "warn: " << message << std::endl;
}

void StdLogger::error_implementation(std::string_view message)
{
    std::cerr << "error: " << message << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NullLogger
//
NullLogger::NullLogger()
{
    level = LOGGER_OFF;
}

void NullLogger::debug_implementation(std::string_view)
{
}
void NullLogger::info_implementation(std::string_view)
{
}
void NullLogger::warn_implementation(std::string_view)
{
}
void NullLogger::error_implementation(std::string_view)
{
}
