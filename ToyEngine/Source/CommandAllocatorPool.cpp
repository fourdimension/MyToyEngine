#include "CommandAllocatorPool.h"

CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type):
	m_pDevice(nullptr),
	m_type(type)
{
}

ID3D12CommandAllocator* CommandAllocatorPool::CreateAllocator()
{
	//ASSERT_SUCCEEDED(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	return nullptr;
}

void CommandAllocatorPool::SetDevice(ID3D12Device* device)
{
	m_pDevice = device;
}
