#pragma once
#include "pch.h"
#include "d3dApp.h"
#include "CommandAllocatorPool.h"
#include <vector>

class CommandQueue {
	friend class CommandListManager;
public:
	CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

	ID3D12CommandAllocator* RequestAllocator();
	void ExecuteCommandList(ID3D12CommandList* list);

	inline D3D12_COMMAND_LIST_TYPE GetType() const { return m_type; }

protected:
	const D3D12_COMMAND_LIST_TYPE m_type;

	CommandAllocatorPool m_AllocatorPool;
	ComPtr<ID3D12CommandQueue> m_pCommandQueue;

	ComPtr<ID3D12Fence> m_pFence;
	UINT64 m_fenceValue;
	HANDLE m_hFenceEvent;
};

class CommandListManager
{
public:
	CommandListManager(ID3D12Device* device);
	void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* allocator, ID3D12CommandList* commandList);

	CommandQueue& GetGraphicsQueue() { return m_GraphicsQueue; }
	CommandQueue& GetComputeQueue() { return m_ComputeQueue; }
	CommandQueue& GetCopyQueue() { return m_CopyQueue; }

protected:
	CommandQueue m_GraphicsQueue;
	CommandQueue m_ComputeQueue;
	CommandQueue m_CopyQueue;

private: 
	ID3D12Device* m_pDevice;
};