
#include "RootSignature.h"
#include "DXSampleHelper.h"
#include "d3dApp.h"

using namespace GameCore;

void RootSignature::Reset(UINT numParameters, UINT numStaticSamplers)
{
	if (numParameters > 0) {
		m_rootParameters.reset(new CD3DX12_ROOT_PARAMETER1[numParameters]);
	}
	else {
		m_rootParameters = nullptr;
	}

	if (numStaticSamplers > 0) {
		m_staticSamplers.reset(new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers]);
	}
	else {
		m_staticSamplers = nullptr;
	}
}

void RootSignature::InitRootParameters()
{
	// TODO: Create a basic effect class to initialize it

	// create a root parameter and fill it out
	CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	m_rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	m_rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	m_rootParameters[2].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL);
}

void RootSignature::InitStaticSamplers() 
{
	// TODO: Create a basic effect class to initialize it
	ASSERT(m_numStaticSamplers != 0);
	D3D12_STATIC_SAMPLER_DESC& samplerDesc = m_staticSamplers[0];

	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void RootSignature::Finalize() 
{
	if (m_finalize)
		return;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	m_rootSignatureDesc.Init_1_1(m_numParameters,
		m_rootParameters.get(), // a pointer to the beginning of our root parameters array
		1,
		&m_staticSamplers[0],
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;

	ASSERT_SUCCEEDED(D3DX12SerializeVersionedRootSignature(&m_rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ASSERT_SUCCEEDED(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

	m_finalize = true;
}



