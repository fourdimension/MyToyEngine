#include "pch.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "d3dApp.h"

using namespace GameCore;

D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPSO::InitGraphicsPSODesc(const D3D12_INPUT_LAYOUT_DESC& layoutDesc)

{
    m_psoDesc.InputLayout = layoutDesc;
    m_psoDesc.pRootSignature = m_rootSignature->GetRootSignature();
    m_psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    /*m_psoDesc.DepthStencilState.DepthEnable = FALSE;
    m_psoDesc.DepthStencilState.StencilEnable = FALSE;*/
    m_psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
    m_psoDesc.SampleMask = UINT_MAX;
    m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_psoDesc.NumRenderTargets = 1;
    m_psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    m_psoDesc.SampleDesc.Count = 1;
	return m_psoDesc;
}

void GraphicsPSO::Finalize()
{
    ASSERT_SUCCEEDED(m_device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    m_pipelineState->SetName(m_name);
}
