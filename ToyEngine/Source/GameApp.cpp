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
		m_MainCamera.Init({ 8, 8, 30 });
		D3DApp::OnInit();
		D3DApp::LoadAssets();

		D3DApp::InitObjects();
	}

	void GameApp::OnUpdate()
	{
		D3DApp::OnUpdate();
	}

	void GameApp::OnRender()
	{
		PIXBeginEvent(m_commandQueue.Get(), 0, L"Render");
		// Record all the commands we need to render the scene into the command list.
		D3DApp::PopulateCommandList();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		PIXEndEvent(m_commandQueue.Get());

		// Present the frame.
		ASSERT_SUCCEEDED(m_swapChain->Present(0, 0));

		D3DApp::WaitForGPU();
	}

	void GameApp::OnDestroy()
	{
		// Wait for the GPU to be done with all resources.
		D3DApp::WaitForGPU();

		CloseHandle(GetFenceEvent());
	}

	void GameApp::LoadAssets()
	{
	}

	
}

