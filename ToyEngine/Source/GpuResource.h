#pragma once
#include "d3d12.h"

class GpuResource {
	friend class CommandContext;
public:
	GpuResource() 
		: m_pResource(nullptr),
		m_usageState(D3D12_RESOURCE_STATE_COMMON),
		m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL) {}

	GpuResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES state)
		: m_pResource(pResource),
		m_usageState(state),
		m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL) {}

	ID3D12Resource* operator->() { return m_pResource.Get(); }
	const ID3D12Resource* operator->() const { return m_pResource.Get(); }

	ID3D12Resource* GetResource() { return m_pResource.Get(); }
	const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }


protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
	D3D12_RESOURCE_STATES m_usageState;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};