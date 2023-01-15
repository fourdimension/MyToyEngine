#pragma once
#include "pch.h"

class RootSignature {
public:
	RootSignature(UINT numParameters = 0, UINT numStaticSamplers = 0)
		:m_finalize(false),
		m_numParameters(numParameters),
		m_numStaticSamplers(numStaticSamplers) 
	{
		Reset(numParameters, numStaticSamplers);
	}

	ID3D12RootSignature* GetRootSignature() const{ return m_rootSignature.Get(); }

	void Reset(UINT numParameters = 0, UINT numStaticSamplers = 0);
	void InitRootParameters();
	void InitStaticSamplers();
	void Finalize();

protected:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	//CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	bool m_finalize;
	UINT m_numParameters;
	UINT m_numStaticSamplers;
	std::unique_ptr<CD3DX12_ROOT_PARAMETER1[]> m_rootParameters;
	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> m_staticSamplers;
};



