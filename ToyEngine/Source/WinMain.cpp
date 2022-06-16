#include "d3dApp.h"

#define MAX_NAME_STRING 256

WCHAR       WindowClass[MAX_NAME_STRING];

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) 
{ 
    /*Initialize Global Variable*/
    wcscpy_s(WindowClass, TEXT("D3DWndClassName"));

    GameCore::IGameApp app(hInstance);
	return GameCore::RunApplication(app, WindowClass, hInstance, nCmdShow);
}