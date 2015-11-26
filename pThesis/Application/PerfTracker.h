#pragma once

#include "GPUPerfAPI-2.17.1695.0\Include\GPUPerfAPI.h"
#include <Windows.h>
#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include "../Render/Renderer.h"
/* Counters
CInvocations	N primitives sent to rasterizer
ClippedPrims	N primitives on one or more clip ops
CPrimitives		N primites rendered

COMPUTE SHADER
CSComplePath		total bytes read & written
CSFastPath			total bytes written
CSFetchInsts		avg N of fetch instructions per execution
CSFetchSize			total kb fetched video memory
CSALUStalledByLDS	Local Data Share stalls

CSWriteSize		total kb written to video memory
CSTime

CulledPrims
D3DGPUTime				time spent in GPU
DepthStencilTestBusy	% gpu time for depth ops

PAPixelsPerTriangle		pixel:triangle ratio after culling
PAStalledOnRasterizer	% gpu time of waiting for rasterization ops

PSBusy
PSExportStalls
PSInvocations
PSPixelsIn
PSPixelsOut
PSTime

VSBusy
VSInvocations
VSTime

ShaderBusyCS
ShaderBusyVS
ShaderBusyPS
*/

namespace AmdPerf
{
	class PerfTracker
	{
		/* Members */
	private:
		HINSTANCE m_gpaLib;

	protected:
	public:

		/* Methods */
	private:
	protected:
	public:
	};

#define GPA_OPEN_SAFE(expr) do { const auto r = (expr); if (r != GPA_STATUS_OK) throw std::runtime_error(#expr + std::string(" failed to open")); } while(0)

	struct LibTable
	{
		/* Members */
		GPA_InitializePtrType 			initialize;
		GPA_DestroyPtrType 				destroy;

		GPA_OpenContextPtrType			openContext;
		GPA_SelectContextPtrType		selectContext;
		GPA_CloseContextPtrType			closeContext;

		GPA_EnableAllCountersPtrType	enableAllCounters;

		//GPA_GetNumCountersPtrType		getNumCounters;
		GPA_GetCounterNamePtrType		getCounterName;
		GPA_GetCounterDataTypePtrType	getCounterDataType;
		
		//GPA_GetCounterUsageTypePtrType  getCounterUsageType;
		GPA_GetCounterIndexPtrType		getCounterIndex;
		GPA_EnableCounterPtrType		enableCounter;
		GPA_DisableCounterPtrType		disableCounter;

		GPA_GetPassCountPtrType 		getPassCount;

		GPA_BeginSessionPtrType 		beginSession;
		GPA_EndSessionPtrType 			endSession;

		GPA_BeginPassPtrType 			beginPass;
		GPA_EndPassPtrType 				endPass;

		GPA_BeginSamplePtrType 			beginSample;
		GPA_EndSamplePtrType 			endSample;

		GPA_GetEnabledCountPtrType		getEnabledCount;
		GPA_GetEnabledIndexPtrType		getEnabledIndex;

		GPA_IsSessionReadyPtrType 		isSessionReady;
		GPA_GetSampleUInt64PtrType 		getSampleUInt64;
		GPA_GetSampleUInt32PtrType 		getSampleUInt32;
		GPA_GetSampleFloat32PtrType 	getSampleFloat32;
		GPA_GetSampleFloat64PtrType 	getSampleFloat64;

		GPA_GetSampleCountPtrType		getSampleCount;
		GPA_RegisterLoggingCallbackPtrType	registerLoggingCallback;
		/* Methods */
		static void LoadFunctions(HINSTANCE _lib, LibTable& _table);
	};

	static void PrintError(GPA_Logging_Type _msgType, const char* _msg);
	//{
	//	//printf("%s\n", _msg);
	//	std::string str = _msg;
	//	str += "\n";
	//	Log(str.c_str());
	//}

	class PerfLibrary
	{
		friend class PerfContext;
		//typedef std::map<std::string, Counter> CounterMap;

		/* Members */
	private:
		HINSTANCE	m_lib;
		LibTable	m_libTable;
	protected:
	public:

		/* Methods */
	private:
		//PerfTracker(const PerfTracker&) = delete;

	protected:
	public:
		PerfLibrary(void);
		~PerfLibrary(void);

		bool Init(void);

		//PerfContext OpenContext(void*);

		//void WriteSession(const char*);
	};

	class PerfContext
	{
		/* Members */
	private:
		void*		m_pContext;
		std::map<const std::string, gpa_int32>	m_counters;

	protected:
	public:
		LibTable*	m_pLibTable;

		/* Methods */
	private:
		

	protected:
	public:
		PerfContext(void);
		bool Init(PerfLibrary*, void*);

		bool EnableCounter	(const std::string&);
		bool DisableCounter	(const std::string&);

		/// Given a sessionID, query the counter values and save them to a file
		void WriteSession(gpa_uint32 currentWaitSessionID, const char* filename);

		void Sample(IRenderer*);
	};
}