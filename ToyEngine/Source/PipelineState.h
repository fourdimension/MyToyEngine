#pragma once
#include "pch.h"

class RootSignature;

class PSO
{
public:
	PSO(const wchar_t* name)
		:m_name(name), m_rootSignature(nullptr), m_pipelineState(nullptr) {}

	void SetRootSignature(const RootSignature& bindSignature)
	{
		m_rootSignature = &bindSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject(void) const { return m_pipelineState.Get(); }

protected:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	const RootSignature* m_rootSignature;
	const wchar_t* m_name;
};

class GraphicsPSO : public PSO 
{
public:
	GraphicsPSO(const wchar_t* name)
		:PSO(name){}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC InitGraphicsPSODesc(const D3D12_INPUT_LAYOUT_DESC& layoutDesc);

	inline void BindVS(ID3DBlob* VS) 
	{
		ASSERT(VS != nullptr);
		m_psoDesc.VS = CD3DX12_SHADER_BYTECODE(VS); 
	}
	inline void BindPS(ID3DBlob* PS) 
	{
		ASSERT(PS != nullptr);
		m_psoDesc.PS = CD3DX12_SHADER_BYTECODE(PS); 
	}

	void Finalize();

	

protected:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
};

class ComputePSO : PSO 
{
	// TODO: same as above

public:
	ComputePSO(const wchar_t* name)
		:PSO(name) {}
protected:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDesc;
};