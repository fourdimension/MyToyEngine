#pragma once
#include "pch.h"
#include <vector>

class CommandListPool
{
public:
	void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* allocator, ID3D12CommandList** commandList);


protected:
	std::vector<ID3D12GraphicsCommandList*> m_commandListPool;
};