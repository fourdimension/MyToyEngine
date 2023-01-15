
#include "CommandListManager.h"
#include "d3dApp.h"

using namespace GameCore;

CommandListManager::CommandListManager(ID3D12Device* device)
	:m_pDevice(device),
	m_GraphicsQueue(device, D3D12_COMMAND_LIST_TYPE_DIRECT),
	m_ComputeQueue(device, D3D12_COMMAND_LIST_TYPE_COMPUTE),
	m_CopyQueue(device, D3D12_COMMAND_LIST_TYPE_COPY)
{
	ASSERT(m_pDevice != nullptr);
}

//void CommandListManager::Create(ID3D12Device* device)
//{
//	ASSERT(device != nullptr);
//
//	m_pDevice = device;
//
//	m_GraphicsQueue.Create(device);
//	m_ComputeQueue.Create(device);
//	m_CopyQueue.Create(device);
//}

void CommandListManager::CreateCommandList(D3D12_COMMAND_LIST_TYPE type,
	ID3D12CommandAllocator* allocator, ID3D12CommandList* commandList)
{
	// TODO: add commandlist to pool
	switch (type) 
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT: allocator = m_GraphicsQueue.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: allocator = m_ComputeQueue.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
	case D3D12_COMMAND_LIST_TYPE_COPY: allocator = m_CopyQueue.RequestAllocator(); break;

	// TODO: check nodeMask below(firt param)
	ASSERT_SUCCEEDED(m_pDevice->CreateCommandList(0, type, allocator, nullptr, MY_IID_PPV_ARGS(&commandList)));
	commandList->SetName(L"commandList");
	}
}

CommandQueue::CommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	:m_type(type),
	m_pFence(nullptr),
	m_fenceValue(0),
	m_hFenceEvent(nullptr),
	m_AllocatorPool(device, type)
{
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ASSERT_SUCCEEDED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
	m_pCommandQueue->SetName(L"CommandListManager::m_CommandQueue");

	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
	m_fenceValue++;

	m_hFenceEvent = CreateEvent(nullptr, false, false, nullptr);
	ASSERT(m_hFenceEvent != nullptr);

}

ID3D12CommandAllocator* CommandQueue::RequestAllocator()
{
	uint64_t completedFence = m_pFence->GetCompletedValue();
	return m_AllocatorPool.RequestAllocator(completedFence);
}

void CommandQueue::ExecuteCommandList(ID3D12CommandList* list)
{
	((ID3D12GraphicsCommandList*)list)->Close();
	ID3D12CommandList* ppCommandLists[] = { (ID3D12GraphicsCommandList*)list};

	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	{
		// Schedule a Signal command in the queue.
		ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), m_fenceValue));

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_pFence->GetCompletedValue() < m_fenceValue)
		{
			ThrowIfFailed(m_pFence->SetEventOnCompletion(m_fenceValue, m_hFenceEvent));
			WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_fenceValue++;
	}
}
