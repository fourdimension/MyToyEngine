
#include "CommandListPool.h"
#include "d3dApp.h"

using namespace GameCore;

void CommandListPool::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, 
	ID3D12CommandAllocator* allocator, ID3D12CommandList** commandList)
{
	// TODO: add commandlist to pool
	switch (type) 
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
	case D3D12_COMMAND_LIST_TYPE_COPY: break;

	// TODO: check nodeMask below(firt param)
	ASSERT_SUCCEEDED(D3DApp::m_device->CreateCommandList(0, type, allocator, nullptr, MY_IID_PPV_ARGS(commandList)));
	(*commandList)->SetName(L"commandList");
	}
}
