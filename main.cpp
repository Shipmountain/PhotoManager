
#include "ui/ui_interface.h"
#include "business/BusinessInterface.h"

#include <atlbase.h>
#include <atlwin.h>

static CComModule k_Module;

#if defined(WIN32) && !defined(UNDER_CE)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
#endif
{
	::CoInitialize(NULL);
	::OleInitialize(NULL);
	k_Module.Init(0, hInstance);

	//数据初始化
    if (theBusiness::instance().init())
    {
        ui::init((void*)hInstance);
    }
    theBusiness::instance().uninit();
	
	k_Module.Term();

	::OleUninitialize();
	::CoUninitialize();

	return 0;
}