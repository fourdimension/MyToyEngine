#pragma once	

#include "pch.h"
#include "DXSampleHelper.h"


using namespace Microsoft::WRL;
using namespace DirectX;

namespace GameCore 
{
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	class D3DApp
	{
	public:
		D3DApp(HINSTANCE hInstance);
		virtual ~D3DApp();


		// Accessors.
		UINT GetWidth() const { return m_DisplayWidth; }
		UINT GetHeight() const { return m_DisplayHeight; }
		static HWND GetHWnd() { return m_hWnd; }
		inline HANDLE GetFenceEvent() { return m_fenceEvent; }

		int RunApplication(D3DApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow);

		virtual void OnInit();
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnDestroy() = 0;

	private:
		

		static const UINT FrameCount = 2;
		static HWND m_hWnd;

		// App resources.
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		// Synchronization objects.
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;

		float m_aspectRatio;

		// Root assets path.
		std::wstring m_assetsPath;
	protected:
		void LoadAssets();
		void PopulateCommandList();
		void WaitForPreviousFrame();

		void GetHardwareAdapter(
			_In_ IDXGIFactory1* pFactory,
			_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

		std::wstring GetAssetFullPath(LPCWSTR assetName);

		HINSTANCE m_hAppInst;
		int m_DisplayWidth;
		int m_DisplayHeight;

		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		UINT m_rtvDescriptorSize;

	
	};


}

