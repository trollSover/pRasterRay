#include "Core.h"
#include "IApplication.h"
#include "../Global/TimerQPC.h"

Core::Core() :	m_pTimer(nullptr),
				m_pApplication(nullptr)
{

}

Core::~Core()
{
	SAFE_DELETE(m_pApplication);
	SAFE_DELETE(m_pTimer);
}

bool Core::Init(IApplication* _application, HINSTANCE _hInstance)
{
	if (!_application)
		return false;

	m_pApplication = _application;

	/* init timer */
	m_pTimer = new TimerQPC();
	if (m_pTimer == nullptr || !m_pTimer->VInit())
		return false;

	/* init window */
	if (!m_window.Init(_hInstance))
		return false;

	/* URD - external Graphics library*/
	//m_pGraphics = nullptr;
	//URD::GetGraphicsInstance(m_pGraphics);

	//if (!m_pGraphics)
	//	return false;

	//if (!m_pGraphics->Initialize(m_window.GetHwnd()))
	//	return false;

	if (!m_pApplication->VInit())
		return false;

#ifdef FDEBUG
	printf("Core: Ok\n");
#endif

	m_pTimer->VUpdate();

	return true;
}

void Core::Close()
{
	//delete in reverse order of init

	/* close window first since it relies on application functionality in order to shut down */
	m_window.Close();
	
	SAFE_DELETE(m_pApplication);
	//FreeGraphicsInstance(m_pGraphics);
	SAFE_DELETE(m_pTimer);
}

void Core::Run()
{
	MSG msg;
	bool run = true;
	ZeroMemory(&msg, sizeof(MSG));

	while (run)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{			
			//if (msg.message == WM_QUIT)
			//{
			//	run = false;
			//}
			//else
			{		
				m_pTimer->VUpdate();
				Time dt = m_pTimer->VGetTime();			/* get time delta */

				run = m_pApplication->VFrame(dt);	/* update application */

				if (!run)
				{
					PrintError(AT, "error encountered during rendering - shutting down");
				}			
			}			
		}

		if (msg.message == WM_QUIT)
		{
			run = false;
		}
		
	}
}