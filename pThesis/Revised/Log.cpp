#include "Log.h"
#include <chrono>
#include <ctime>

using Verdandi::Debug::Log;
using Verdandi::Debug::IOWriter;
using Verdandi::Debug::LogNotification;

// not sure what to do with this but keep one static logger here for convenience
static IOWriter defaultWriter("Verdandi.log");

IOWriter::IOWriter(const char* _fileName)
{
	m_filewrite_mutex.lock();
	m_file_stream.open(_fileName, std::ostream::out | std::ostream::ate);

	// time stamp
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::chrono::system_clock::now();
	std::tm tm;
	localtime_s(&tm, &now);

	m_file_stream << '[' << 1900 + tm.tm_year << '-' << tm.tm_mon << '-' << tm.tm_mday << ' ' << tm.tm_hour << ':' << tm.tm_min << ':' << tm.tm_sec << "]\nLog Stream Opened\n";
	m_file_stream << "--------------------\n";
	m_filewrite_mutex.unlock();
}

IOWriter::~IOWriter(void)
{
	m_filewrite_mutex.lock();
	m_file_stream.flush();
	m_file_stream.close();
	m_filewrite_mutex.unlock();
}

void IOWriter::WriteToFile(const LogNotification _logLevel, const std::string& _message)
{
	m_filewrite_mutex.lock();

	// lots of opinions how to extract string from enums, seems reflection will be present soon but for now
	// just use a simple switch case (don't want to handle static char* array)

	switch (_logLevel)
	{
	case LogNotification::Debug:
		m_file_stream << "Debug Notification";
		break;
	case LogNotification::Warning:
		m_file_stream << "Warning";
		break;
	case LogNotification::SoftError:
		m_file_stream << "Soft Error";
		break;
	case LogNotification::FatalError:
		m_file_stream << "Fatal Error";
		break;
	case LogNotification::Custom:
		m_file_stream << "Custom Notification";
	default:
		m_file_stream << "Undefined Error";	// this should not be able to happen
		break;
	}

	// error message
	m_file_stream << _message.c_str() << "\n--------------------" << std::endl;

	m_filewrite_mutex.unlock();
}

Log::Log(const LogNotification _level)
	: m_log_notification(_level), m_writer(defaultWriter)
{ 
	// time stamp
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::chrono::system_clock::now();
	std::tm tm;
	localtime_s(&tm, &now);

	m_log_stream << " [" << tm.tm_hour << ':' << tm.tm_min << ':' << tm.tm_sec << "]\n";
}

Log::~Log(void)
{
	m_writer.WriteToFile(m_log_notification, m_log_stream.str());
}