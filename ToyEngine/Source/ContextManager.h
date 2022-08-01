#pragma once
#include "pch.h"

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class GpuResource;

class ContextManager {
public: 
	//CommandContext* AssignContext(D3D12_COMMAND_LIST_TYPE type);
};

class CommandContext {
	//friend class ContextManager;
private:
	CommandContext(D3D12_COMMAND_LIST_TYPE type);

public:
	void Transition(GpuResource& resource, D3D12_RESOURCE_STATES newState);

protected:
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	ComPtr<ID3D12CommandAllocator> m_pAllocator;
	ComPtr<ID3D12RootSignature> m_pGraphicsRootSignature;
	ComPtr<ID3D12RootSignature> m_pComputeRootSignature;

	ComPtr<ID3D12PipelineState> m_pPipelineState;

	D3D12_COMMAND_LIST_TYPE m_type;
};