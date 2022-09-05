#pragma once
#include "pch.h"

using Microsoft::WRL::ComPtr;

class GpuResource;
class CommandListManager;

class ContextManager {
public: 
	//CommandContext* AssignContext(D3D12_COMMAND_LIST_TYPE type);
};

class CommandContext {
	friend class ContextManager;

public:
	CommandContext(D3D12_COMMAND_LIST_TYPE type);
	void Transition(GpuResource& resource, D3D12_RESOURCE_STATES newState);

	ID3D12GraphicsCommandList* GetCommandList() { return m_pCommandList.Get(); }

private:
	void Initialize();

protected:
	CommandListManager* m_pCmdListManager;

	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	ComPtr<ID3D12CommandAllocator> m_pAllocator;
	ComPtr<ID3D12RootSignature> m_pGraphicsRootSignature;
	ComPtr<ID3D12RootSignature> m_pComputeRootSignature;

	ComPtr<ID3D12PipelineState> m_pPipelineState;

	D3D12_COMMAND_LIST_TYPE m_type;
};