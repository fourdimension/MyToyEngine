#pragma once	

#include "pch.h"

namespace GameCore 
{
	class IGameApp
	{
	public:
		IGameApp(HINSTANCE hInstance);
		virtual ~IGameApp();

		// Accessors.
		UINT GetWidth() const { return m_DisplayWidth; }
		UINT GetHeight() const { return m_DisplayHeight; }

	protected:
		HINSTANCE m_hAppInst;
		int m_DisplayWidth;
		int m_DisplayHeight;
		HWND m_hMainWnd;
	};
}

namespace GameCore
{
	int RunApplication(IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow);
}
