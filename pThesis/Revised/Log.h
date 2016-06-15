#pragma once

#include <fstream>
#include <sstream>
#include <mutex>

namespace Verdandi {
namespace Debug {
	
	enum class LogNotification
	{
		Debug		= 0,
		Warning		= 0x1,
		SoftError	= 0x2,
		FatalError	= 0x4,
		Custom		= 0x8,
	};

	class IOWriter
	{
		friend class Log;

		/* Members */
	private:
		std::mutex		m_filewrite_mutex;
		std::ofstream	m_file_stream;

	protected:
	public:

		/* Methods */
	private:
		void WriteToFile(const LogNotification, const std::string&);
	protected:
	public:
		IOWriter(const char*);
		~IOWriter(void);
	};

	class Log
	{
		/* Members */
	private:
		LogNotification		m_log_notification;
		std::ostringstream	m_log_stream;
		IOWriter&			m_writer;

	protected:
	public:

		/* Methods */
	private:
		Log(void) = delete;
	protected:
	public:
		Log(const LogNotification);
		//Log(const LogNotification, const char*);	// todo: add support for several log writers
		~Log(void);

		template<typename T>
		Log& operator << (T const& _value)
		{
			m_log_stream << _value;
			return *this;
		}
	};
}
}