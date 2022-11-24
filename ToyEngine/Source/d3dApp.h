#pragma once	

#include "pch.h"
#include "../Util/StepTimer.h"
#include "DXSampleHelper.h"
#include "../Util/SimpleCamera.h"


//using Microsoft::WRL::ComPtr;
using namespace DirectX;

class RootSignature;
class CommandListManager;
class CommandContext;
class DynamicUploadHeap;

namespace GameCore 
{
	extern ID3D12Device* m_device;

	struct ConstantBufferPerObject {
		XMFLOAT4X4 wvpMat;
	};

	struct CameraViewProjectionObject {
		XMFLOAT4X4 vpMat;
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

		int InitApplication(D3DApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow);
		int Run(D3DApp& app);

		// Helper function
		void SetCustomWindowText(LPCWSTR text);

		virtual void OnInit();
		virtual void OnUpdate();
		virtual void OnRender() = 0;
		virtual void OnDestroy() = 0;

		// Samples override the event handlers to handle specific messages.
		virtual void OnKeyDown(UINT8 /*key*/);
		virtual void OnKeyUp(UINT8 /*key*/);

		static const UINT FrameCount = 2;
		static const UINT ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;
		static const UINT ConstantUploadAlignedSize = 256 * 1024; // 256kb
	private:
		
		UINT m_frameIndex;
		static HWND m_hWnd;

		// App resources.
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		ComPtr<ID3D12Resource> m_depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
		ComPtr<ID3D12DescriptorHeap> m_dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

		// Synchronization objects.
		
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue[FrameCount];

		// Root assets path.
		std::wstring m_assetsPath;
	protected:

		// object 
		void InitObjects();
		void UpdateCubes();

		void LoadAssets();
		void PopulateCommandList();
		void WaitForGPU();
		void MoveToNextFrame();
		void UpdateConstantBuffers();

		void GetHardwareAdapter(
			_In_ IDXGIFactory1* pFactory,
			_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

		std::wstring GetAssetFullPath(LPCWSTR assetName);

		HINSTANCE m_hAppInst;
		int m_DisplayWidth;
		int m_DisplayHeight;

		DynamicUploadHeap m_dynamicUploadHeap;
		CommandContext* m_context;

		// Pipeline objects.
		RootSignature* m_signature;
		CommandListManager* m_commandListManager;

		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;
		
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator[FrameCount];
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		//ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		UINT m_rtvDescriptorSize;

		StepTimer m_Timer;
		UINT m_frameCounter;

		std::wstring m_MainWndCaption;
		bool      m_AppPaused;       // if the application paused

		SimpleCamera m_MainCamera;


		/*Object decleration for testing*/
#pragma region Object
		ConstantBufferPerObject cbPerObject; // this is the constant buffer data we will send to the gpu
		CameraViewProjectionObject camMatrix; // this is camera's constant buffer data we will send to the gpu
		ID3D12Resource* constantBufferUploadHeaps[FrameCount]; // this is the memory on the gpu where constant buffers for each frame will be placed

		UINT8* cbvGPUAddress[FrameCount]; // this is a pointer to each of the constant buffer resource heaps

		Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_texture;

		XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
		XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
		XMFLOAT4 cube1Position; // our first cubes position in space

		XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
		XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
		XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

		int numCubeIndices; // the number of indices to draw the cube

#pragma endregion Object
	};


}

