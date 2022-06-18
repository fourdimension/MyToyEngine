#include "GameApp.h"


namespace GameCore
{
	GameApp::GameApp(HINSTANCE hInst)
		:D3DApp(hInst)
	{

	}

	GameApp::~GameApp()
	{
	}

	void GameApp::OnInit()
	{
		D3DApp::OnInit();
		D3DApp::LoadAssets();
	}

	void GameApp::OnUpdate()
	{
	}

	void GameApp::OnRender()
	{
		// Record all the commands we need to render the scene into the command list.
		D3DApp::PopulateCommandList();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		ASSERT_SUCCEEDED(m_swapChain->Present(1, 0));

		WaitForPreviousFrame();
	}

	void GameApp::OnDestroy()
	{
		// Wait for the GPU to be done with all resources.
		D3DApp::WaitForPreviousFrame();

		CloseHandle(GetFenceEvent());
	}

	void GameApp::LoadAssets()
	{
	}

	
}

