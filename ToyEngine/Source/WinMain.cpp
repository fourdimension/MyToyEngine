#include "GameApp.h"


WCHAR       WindowClass[MAX_NAME_STRING];

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) 
{ 
    /*Initialize Global Variable*/
    LoadString(hInstance, IDS_WINDOWCLASS, WindowClass, MAX_NAME_STRING);

    GameCore::GameApp app(hInstance);
	return app.InitApplication(app, WindowClass, hInstance, nCmdShow);
}