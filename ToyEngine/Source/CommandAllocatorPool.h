#pragma once
#include <vector>
#include "pch.h"

class CommandAllocatorPool {
public:
	CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12CommandAllocator* CreateAllocator();
	void SetDevice(ID3D12Device* device);
protected:
	Microsoft::WRL::ComPtr<ID3D12Device> m_pDevice;
	D3D12_COMMAND_LIST_TYPE m_type;
	std::vector<ID3D12CommandAllocator*> m_CommandAllocatorPool;
};
