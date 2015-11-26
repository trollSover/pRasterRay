#include "PerfTracker.h"

#include <tchar.h>
#include <stdio.h>
#include <cstdlib>
#include <set>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "../CoreSystems/CoreStd.h"
//using Performance::Counter;
using AmdPerf::PerfLibrary;
using AmdPerf::PerfContext;
using AmdPerf::LibTable;

static void AmdPerf::PrintError(GPA_Logging_Type _msgType, const char* _msg)
{
	//printf("%s\n", _msg);
	std::string str = std::string("GPA: ") + _msg + "\n";
	//str += "\n";
	Log(str.c_str());
}

template <typename T>
T function_pointer_cast(const void* p)
{
	static_assert (sizeof(T) == sizeof(void*), "Size of a plain function pointer "
		"must be equal to size of a plain pointer.");
	T result;
	::memcpy(&result, &p, sizeof(void*));
	return result;
}


void* LoadFunction(HINSTANCE _lib, const char* _name)
{
	void* result = nullptr;

	result = GetProcAddress(_lib, _name);


	if (result == nullptr) {
		throw std::runtime_error(std::string("Could not load function: ") + _name);
	}

	return result;
}

void LibTable::LoadFunctions(HINSTANCE _lib, LibTable& _table)
{
	_table.initialize			= function_pointer_cast<GPA_InitializePtrType>			(LoadFunction(_lib, "GPA_Initialize"));
	_table.destroy				= function_pointer_cast<GPA_DestroyPtrType>				(LoadFunction(_lib, "GPA_Destroy"));
	_table.openContext			= function_pointer_cast<GPA_OpenContextPtrType>			(LoadFunction(_lib, "GPA_OpenContext"));
	_table.selectContext		= function_pointer_cast<GPA_SelectContextPtrType>		(LoadFunction(_lib, "GPA_SelectContext"));
	_table.closeContext			= function_pointer_cast<GPA_CloseContextPtrType>		(LoadFunction(_lib, "GPA_CloseContext"));
	_table.enableAllCounters	= function_pointer_cast<GPA_EnableAllCountersPtrType>	(LoadFunction(_lib, "GPA_EnableAllCounters"));

	//_table.getNumCounters = function_pointer_cast<GPA_GetNumCountersPtrType>		(LoadFunction(_lib, "GPA_GetNumCounters"));
	_table.getCounterName		= function_pointer_cast<GPA_GetCounterNamePtrType>		(LoadFunction(_lib, "GPA_GetCounterName"));
	_table.getCounterDataType	= function_pointer_cast<GPA_GetCounterDataTypePtrType>	(LoadFunction(_lib, "GPA_GetCounterDataType"));
	//_table.getCounterUsageType = function_pointer_cast<GPA_GetCounterUsageTypePtrType>	(LoadFunction(_lib, "GPA_GetCounterUsageType"));
	_table.getCounterIndex		= function_pointer_cast<GPA_GetCounterIndexPtrType>		(LoadFunction(_lib, "GPA_GetCounterIndex"));
	_table.enableCounter		= function_pointer_cast<GPA_EnableCounterPtrType>		(LoadFunction(_lib, "GPA_EnableCounter"));
	_table.disableCounter		= function_pointer_cast<GPA_DisableCounterPtrType>		(LoadFunction(_lib, "GPA_DisableCounter"));
	_table.getPassCount			= function_pointer_cast<GPA_GetPassCountPtrType>		(LoadFunction(_lib, "GPA_GetPassCount"));
	_table.beginSession			= function_pointer_cast<GPA_BeginSessionPtrType>		(LoadFunction(_lib, "GPA_BeginSession"));
	_table.endSession			= function_pointer_cast<GPA_EndSessionPtrType>			(LoadFunction(_lib, "GPA_EndSession"));
	_table.beginPass			= function_pointer_cast<GPA_BeginPassPtrType>			(LoadFunction(_lib, "GPA_BeginPass"));
	_table.endPass				= function_pointer_cast<GPA_EndPassPtrType>				(LoadFunction(_lib, "GPA_EndPass"));
	_table.beginSample			= function_pointer_cast<GPA_BeginSamplePtrType>			(LoadFunction(_lib, "GPA_BeginSample"));
	_table.endSample			= function_pointer_cast<GPA_EndSamplePtrType>			(LoadFunction(_lib, "GPA_EndSample"));
	_table.isSessionReady		= function_pointer_cast<GPA_IsSessionReadyPtrType>		(LoadFunction(_lib, "GPA_IsSessionReady"));
	
	_table.getSampleUInt64		= function_pointer_cast<GPA_GetSampleUInt64PtrType>		(LoadFunction(_lib, "GPA_GetSampleUInt64"));
	_table.getSampleUInt32		= function_pointer_cast<GPA_GetSampleUInt32PtrType>		(LoadFunction(_lib, "GPA_GetSampleUInt32"));
	_table.getSampleFloat32		= function_pointer_cast<GPA_GetSampleFloat32PtrType>	(LoadFunction(_lib, "GPA_GetSampleFloat32"));
	_table.getSampleFloat64		= function_pointer_cast<GPA_GetSampleFloat64PtrType>	(LoadFunction(_lib, "GPA_GetSampleFloat64"));
	_table.getEnabledCount		= function_pointer_cast<GPA_GetEnabledCountPtrType>		(LoadFunction(_lib, "GPA_GetEnabledCount"));
	_table.getEnabledIndex		= function_pointer_cast<GPA_GetEnabledIndexPtrType>		(LoadFunction(_lib, "GPA_GetEnabledIndex"));

	_table.getSampleCount			= function_pointer_cast<GPA_GetSampleCountPtrType>				(LoadFunction(_lib, "GPA_GetSampleCount"));
	_table.registerLoggingCallback	= function_pointer_cast<GPA_RegisterLoggingCallbackPtrType>		(LoadFunction(_lib, "GPA_RegisterLoggingCallback"));
}

PerfLibrary::PerfLibrary(void)
	: m_lib(nullptr)
{	}

PerfLibrary::~PerfLibrary(void)
{
	if (m_lib != nullptr)
	{
		m_libTable.closeContext();
		m_libTable.destroy();
		FreeLibrary(m_lib);
	}
}

bool PerfLibrary::Init(void)
{
	m_lib = LoadLibraryA("GPUPerfAPIDX11-x64.dll");

	if (m_lib == nullptr)
	{
		printf("failed to load GPUPerfAPI\n");
		return false;
	}

	// fetch API functions
	try
	{
		LibTable::LoadFunctions(m_lib, m_libTable);
	}
	catch (std::runtime_error& e)
	{
		printf("%s\n", e.what());
		return false;
	}
	// try initialize 
	try
	{
		GPA_OPEN_SAFE(m_libTable.initialize());
	}
	catch (std::runtime_error& e)
	{
		printf("%s\n", e.what());
		return false;
	}
	printf("GPUPerfAPI loaded successfully\n");

	return true;
}


PerfContext::PerfContext(void)
	: m_pContext(nullptr), m_pLibTable(nullptr)
{	

}

bool PerfContext::Init(PerfLibrary* _lib, void* _pContext)
{
	m_pContext = _pContext;
	m_pLibTable = &_lib->m_libTable;

	try
	{
		GPA_OPEN_SAFE(m_pLibTable->openContext(m_pContext));
	}
	catch (std::runtime_error& e)
	{
		printf("%s\n", e.what());
		return false;
	}
	try
	{
		m_pLibTable->registerLoggingCallback(GPA_Logging_Type::GPA_LOGGING_ERROR_AND_MESSAGE, PrintError);
	}
	catch (std::runtime_error& e)
	{

	}

	std::ifstream counters("counter_enable.txt");
	if (counters.is_open())
	{
		std::string line;
		while (std::getline(counters, line))
		{
			//printf("%s\n", line.c_str());
			// clean out any comments
			size_t pos = line.find_first_of('#');
			if (pos != std::string::npos)
			{
				line.erase(pos, line.length());
			}
			// clean out any whitespace
			pos = line.find_first_of(' ');
			if (pos != std::string::npos)
			{
				line.erase(pos, line.length());
			}
			pos = line.find_first_of('\t');
			if (pos != std::string::npos)
			{
				line.erase(pos, line.length());
			}

			if (line.length() == 0)
				continue;

			// enable counter
			GPA_Status status;
			gpa_uint32 index;
			status = m_pLibTable->getCounterIndex(line.c_str(), &index);

			if (status != GPA_STATUS_OK)
			{
				printf("GPA: couldn't find %s counter\n", line.c_str());
				continue;
			}

			m_counters[line] = index;

			std::string str = "GPA: ";
			if (EnableCounter(line))
			{
				str += "registered " + line + " counter\n";
			}
			else
			{
				str += "failed to register " + line + " counter\n";
			}
			Log(str.c_str());
		}
	}
	counters.close();

	printf("Performance Context successfully created\n");

	return true;
}



bool PerfContext::EnableCounter(const std::string& _counter)
{
	if (m_counters.find(_counter) == m_counters.end())
	{
		printf("%s counter not registered\n", _counter.c_str());
		return false;
	}
	
	gpa_uint32 index = m_counters[_counter];
	GPA_Status status = m_pLibTable->enableCounter(index);

	if (status != GPA_STATUS_OK)
	{
		printf("couldn't enable %i: %s counter\n", index, _counter.c_str());
		return false;
	}

	return true;
}

bool PerfContext::DisableCounter(const std::string& _counter)
{
	if (m_counters.find(_counter) == m_counters.end())
	{
		printf("%s counter not registered\n", _counter.c_str());
		return false;
	}

	gpa_uint32 index = m_counters[_counter];
	GPA_Status status = m_pLibTable->disableCounter(index);

	if (status != GPA_STATUS_OK)
	{
		printf("couldn't disable %i: %s counter\n", index, _counter.c_str());
		return false;
	}

	return true;
}

void PerfContext::Sample(IRenderer* _pRenderer)
{

}

#pragma warning( disable : 4996 )
/// Given a sessionID, query the counter values and save them to a file
void PerfContext::WriteSession(gpa_uint32 currentWaitSessionID, const char* filename)
{
	static bool doneHeadings = false;
	gpa_uint32 count;
	m_pLibTable->getEnabledCount(&count);
	FILE* f;
	if (!doneHeadings)
	{
		const char* name;
		f = fopen(filename, "w");
		assert(f);
		fprintf(f, "Identifier, , ");
		for (gpa_uint32 counter = 0; counter < count; counter++)
		{
			gpa_uint32 enabledCounterIndex;
			m_pLibTable->getEnabledIndex(counter, &enabledCounterIndex);
			m_pLibTable->getCounterName(enabledCounterIndex, &name);
			fprintf(f, "%s, ", name);
		}
		fprintf(f, "\n");
		fclose(f);
		doneHeadings = true;
	}
	f = fopen(filename, "a+");

	assert(f);
	gpa_uint32 sampleCount;
	m_pLibTable->getSampleCount(currentWaitSessionID, &sampleCount);
	for (gpa_uint32 sample = 0; sample < sampleCount; sample++)
	{
		fprintf(f, "session: %d; sample: %d, ", currentWaitSessionID, sample);
		for (gpa_uint32 counter = 0; counter < count; counter++)
		{
			gpa_uint32 enabledCounterIndex;
			m_pLibTable->getEnabledIndex(counter, &enabledCounterIndex);
			GPA_Type type;
			m_pLibTable->getCounterDataType(enabledCounterIndex, &type);
			if (type == GPA_TYPE_UINT32)
			{
				gpa_uint32 value;
				m_pLibTable->getSampleUInt32(currentWaitSessionID, sample, enabledCounterIndex, &value);
				fprintf(f, "%u,", value);
			}
			else if (type == GPA_TYPE_UINT64)
			{
				gpa_uint64 value;
				m_pLibTable->getSampleUInt64(currentWaitSessionID, sample, enabledCounterIndex, &value);
				fprintf(f, "%I64u,", value);
			}
			else if (type == GPA_TYPE_FLOAT32)
			{
				gpa_float32 value;
				m_pLibTable->getSampleFloat32(currentWaitSessionID, sample, enabledCounterIndex, &value);
				fprintf(f, "%f,", value);
			}
			else if (type == GPA_TYPE_FLOAT64)
			{
				gpa_float64 value;
				m_pLibTable->getSampleFloat64(currentWaitSessionID, sample, enabledCounterIndex, &value);
				fprintf(f, "%f,", value);
			}
			else
			{
				assert(false);
			}
		}
		fprintf(f, "\n");
	}
	fclose(f);
}
#pragma warning( default : 4996 )