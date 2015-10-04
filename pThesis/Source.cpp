#include "CoreSystems/Core.h"
#include "Application/VoxelApp.h"

IApplication*	g_pApplication	= nullptr;
Core*			g_pCoreSystem	= nullptr;

template<typename TApplication>
int Launch(HINSTANCE _hInstance)
{
#ifdef FDEBUG
	printf("Launching\n");
#endif

	g_pCoreSystem	= new Core();
	g_pApplication  = new TApplication();

	if (!g_pCoreSystem || !g_pApplication)
	{
		return -1;
	}

	if (!g_pCoreSystem->Init(g_pApplication, _hInstance))
	{
		return -10;
	}

	g_pCoreSystem->Run();

#ifdef FDEBUG
	printf("Closing\n");
#endif

	g_pCoreSystem->Close();

	SAFE_DELETE(g_pCoreSystem);

	return 0;
}

#ifdef _DEBUG

#include <crtdbg.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	printf("Debug\n");
	HINSTANCE hInstance = GetModuleHandle(NULL);

	int retval = Launch<VoxelApp>(hInstance);

	//printf("preparing memory dump...\n");
	//_CrtDumpMemoryLeaks();			// lots of memory leaking that needs to be cleaned up
	//printf("\n/memory dump\n");

	Sleep(5000);

	return retval;
}

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int retval = Launch<VoxelApp>(hInstance);
	return retval;
}

#endif

