#include "CommandAllocatorPool.h"

CommandAllocatorPool::CommandAllocatorPool(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type):
	m_pDevice(device),
	m_type(type)
{
}

ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(uint64_t completedFence)
{
	//ASSERT_SUCCEEDED(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	ID3D12CommandAllocator* pAllocator = nullptr;
	if (!m_ReadyAllocators.empty()) 
	{
		std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocators.front();

		if (AllocatorPair.first < completedFence) 
		{
			pAllocator = AllocatorPair.second;
			ASSERT_SUCCEEDED(pAllocator->Reset());
			m_ReadyAllocators.pop();
		}
	}

	if (pAllocator == nullptr)
	{
		ASSERT_SUCCEEDED(m_pDevice->CreateCommandAllocator(m_type, MY_IID_PPV_ARGS(&pAllocator)));
		wchar_t AllocatorName[32];
		swprintf(AllocatorName, 32, L"CommandAllocator %zu", m_CommandAllocatorPool.size());
		pAllocator->SetName(AllocatorName);
		m_CommandAllocatorPool.push_back(pAllocator);
	}
	return pAllocator;
}

