#pragma once
#include <vector>
#include <queue>
#include "pch.h"

class CommandAllocatorPool {
public:
	CommandAllocatorPool(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12CommandAllocator* RequestAllocator(uint64_t completedFence);

protected:
	Microsoft::WRL::ComPtr<ID3D12Device> m_pDevice;
	D3D12_COMMAND_LIST_TYPE m_type;
	std::vector<ID3D12CommandAllocator*> m_CommandAllocatorPool;
	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
};
