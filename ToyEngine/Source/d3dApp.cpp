#include "d3dApp.h"
#include "DDSTextureLoader.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "CommandListManager.h"
#include <sstream>

namespace GameCore 
{
    HWND D3DApp::m_hWnd = nullptr;
    ComPtr<ID3D12Device> D3DApp::m_device = nullptr;

    D3DApp::D3DApp(HINSTANCE hInst)
        : m_hAppInst(hInst),
        m_MainWndCaption(L"Rendering a Cube"),
        m_DisplayWidth(800),
        m_DisplayHeight(600),
        m_AppPaused(false),
        m_viewport(0.0f, 0.0f, static_cast<float>(m_DisplayWidth), static_cast<float>(m_DisplayHeight)),
        m_scissorRect(0, 0, static_cast<LONG>(m_DisplayWidth), static_cast<LONG>(m_DisplayHeight)),
        m_frameCounter(0),
        m_fenceValue{},
        m_rtvDescriptorSize(0),
        m_signature(nullptr)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath)); // How to get path??
        m_assetsPath = assetsPath;

    }

    D3DApp::~D3DApp()
    {
    }

    void D3DApp::SetCustomWindowText(LPCWSTR text)
    {
        std::wstring windowText(text);
        SetWindowText(m_hWnd, windowText.c_str());
    }

    void D3DApp::OnInit()
    {
        UINT dxgiFactoryFlags = 0;

    #if defined(_DEBUG)
        // Enable the D3D12 debug layer.
        // it must be called before the D3D12 device is created.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
    #endif

        // Obtain the DXGI factory
        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
        ASSERT_SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlags, MY_IID_PPV_ARGS(&dxgiFactory)));

        ComPtr<IDXGIAdapter1> pAdapter;
        GetHardwareAdapter(dxgiFactory.Get(), &pAdapter);
        ASSERT_SUCCEEDED(D3D12CreateDevice(
            pAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ));


        // Describe and create the command queue.
        m_commandListManager = new CommandListManager(m_device.Get());

        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;

        swapChainDesc.Width = GetWidth();
        swapChainDesc.Height = GetHeight();
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc = sampleDesc;

        ComPtr<IDXGISwapChain1> swapChain;
        ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
            m_hWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        ASSERT_SUCCEEDED(swapChain.As(&m_swapChain));

        // This sample does not support fullscreen transitions.
        ASSERT_SUCCEEDED(dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ASSERT_SUCCEEDED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        {
            // Describe and create a shader resource view (SRV) heap for the texture.
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.NumDescriptors = 1;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
        }

        {
            // create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.NumDescriptors = 1;
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ASSERT_SUCCEEDED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsDescriptorHeap)));
        }

        {
           // Describe and create a constant buffer view (CBV) descriptor heap.
           // Flags indicate that this descriptor heap can be bound to the pipeline 
           // and that descriptors contained in it can be referenced by a root table.
            D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
            cbvHeapDesc.NumDescriptors = 1;
            cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
        }
        // Create frame resources.
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < FrameCount; n++)
            {
                ASSERT_SUCCEEDED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
                m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }

        // Create the command Allocators 
        {
            for (int i = 0; i < FrameCount; i++) {
                ASSERT_SUCCEEDED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])));
            }
        }
    }

    void D3DApp::OnUpdate()
    {
        m_Timer.Tick(NULL);

        if (m_frameCounter == 500)
        {
            // Update window text with FPS value.
            wchar_t fps[64];
            swprintf_s(fps, L"%ufps", m_Timer.GetFramesPerSecond());
            SetCustomWindowText(fps);
            m_frameCounter = 0;
        }

        m_frameCounter++;

        UpdateCubes();

        m_MainCamera.Update(static_cast<float>(m_Timer.GetElapsedSeconds()));
        XMStoreFloat4x4(&camMatrix.vpMat, XMMatrixTranspose(m_MainCamera.GetViewProjectionMatrix()));
        // copy our camera ConstantBuffer instance to the mapped constant buffer resource
        memcpy(cbvGPUAddress[m_frameIndex] + m_MainCamera.GetOffsetInConstantBuffer(), &camMatrix, sizeof(camMatrix));

        UpdateConstantBuffers();
        //UpdateConstantBuffers(m_MainCamera.GetViewMatrix(), m_MainCamera.GetProjectionMatrix(0.8f, m_aspectRatio));
    }

    void D3DApp::OnKeyDown(UINT8 key)
    {
        m_MainCamera.OnKeyDown(key);
    }

    void D3DApp::OnKeyUp(UINT8 key)
    {
        m_MainCamera.OnKeyUp(key);
    }

    void D3DApp::InitObjects()
    {
        // set starting cubes position
        // first cube
        cube1Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // set cube 1's position
        XMVECTOR posVec = XMLoadFloat4(&cube1Position); // create xmvector for cube1's position

        XMMATRIX tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
        XMStoreFloat4x4(&cube1RotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
        XMStoreFloat4x4(&cube1WorldMat, tmpMat); // store cube1's world matrix

        // second cube
        cube2PositionOffset = XMFLOAT4(1.5f, 0.0f, 0.0f, 0.0f);
        posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cube1Position); // create xmvector for cube2's position
                                                                                    // we are rotating around cube1 here, so add cube2's position to cube1

        tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube2's position offset vector
        XMStoreFloat4x4(&cube2RotMat, XMMatrixIdentity()); // initialize cube2's rotation matrix to identity matrix
        XMStoreFloat4x4(&cube2WorldMat, tmpMat); // store cube2's world matrix
    }

    void D3DApp::UpdateCubes()
    {
        // update app logic, such as moving the camera or figuring out what objects are in view

    // create rotation matrices
        XMMATRIX rotXMat = XMMatrixRotationX(0.0000f);
        XMMATRIX rotYMat = XMMatrixRotationY(0.0000f);
        XMMATRIX rotZMat = XMMatrixRotationZ(0.0000f);

        // add rotation to cube1's rotation matrix and store it
        XMMATRIX rotMat = XMLoadFloat4x4(&cube1RotMat) * rotXMat * rotYMat * rotZMat;
        XMStoreFloat4x4(&cube1RotMat, rotMat);

        // create translation matrix for cube 1 from cube 1's position vector
        XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube1Position));

        // create cube1's world matrix by first rotating the cube, then positioning the rotated cube
        XMMATRIX worldMat = rotMat * translationMat;

        // store cube1's world matrix
        XMStoreFloat4x4(&cube1WorldMat, worldMat);

        // update constant buffer for cube1
        // create the wvp matrix and store in constant buffer
        XMMATRIX wvpMat = XMLoadFloat4x4(&cube1WorldMat); // create wvp matrix
        XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
        XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

        // copy our ConstantBuffer instance to the mapped constant buffer resource
        memcpy(cbvGPUAddress[m_frameIndex], &cbPerObject, sizeof(cbPerObject));

        // now do cube2's world matrix
        // create rotation matrices for cube2
        rotXMat = XMMatrixRotationX(0.0000f);
        rotYMat = XMMatrixRotationY(0.0000f);
        rotZMat = XMMatrixRotationZ(0.0000f);

        // add rotation to cube2's rotation matrix and store it
        rotMat = rotZMat * (XMLoadFloat4x4(&cube2RotMat) * (rotXMat * rotYMat));
        XMStoreFloat4x4(&cube2RotMat, rotMat);

        // create translation matrix for cube 2 to offset it from cube 1 (its position relative to cube1
        XMMATRIX translationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2PositionOffset));

        // we want cube 2 to be half the size of cube 1, so we scale it by .5 in all dimensions
        XMMATRIX scaleMat = XMMatrixScaling(0.5f, 0.5f, 0.5f);

        // reuse worldMat. 
        // first we scale cube2. scaling happens relative to point 0,0,0, so you will almost always want to scale first
        // then we translate it. 
        // then we rotate it. rotation always rotates around point 0,0,0
        // finally we move it to cube 1's position, which will cause it to rotate around cube 1
        worldMat = scaleMat * translationOffsetMat * rotMat * translationMat;

        wvpMat = XMLoadFloat4x4(&cube2WorldMat); // create wvp matrix
        transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
        XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

        // copy our ConstantBuffer instance to the mapped constant buffer resource
        memcpy(cbvGPUAddress[m_frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));

        // store cube2's world matrix
        XMStoreFloat4x4(&cube2WorldMat, worldMat);
    }

    void D3DApp::LoadAssets()
    {


        // Create an empty root signature.
        
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

            // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

            if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
            {
                featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }
            
#pragma region RootSignature
            // create a root parameter and fill it out
            m_signature = new RootSignature(3, 1);

            m_signature->InitRootParameters();
            m_signature->InitStaticSamplers();
            m_signature->Finalize();
        
#pragma endregion RootSignature

        // Create the pipeline state, which includes compiling and loading shaders.
            ComPtr<ID3DBlob> vertexShader;
            ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif

            ASSERT_SUCCEEDED(D3DCompileFromFile(L"Shaders\\Vertexshader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
            ASSERT_SUCCEEDED(D3DCompileFromFile(L"Shaders\\Pixelshader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                //{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            GraphicsPSO gPSO(L"graphicsPSO", m_signature);
            gPSO.InitGraphicsPSODesc({ inputElementDescs, _countof(inputElementDescs) });
            gPSO.BindVS(vertexShader.Get());
            gPSO.BindPS(pixelShader.Get());
            gPSO.Finalize();

        // create a command list 
            //m_commandListManager->CreateCommandList(m_commandListManager->GetGraphicsQueue().GetType(), )
        ASSERT_SUCCEEDED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

        // Create the vertex/index/depth buffer.
        {
            // Define the geometry for a triangle.
            VertexPosColor vertices[] =
            {
                /*{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
                { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
                { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
                { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
                { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
                { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
                { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
                { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }*/

                // front face
                { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f} },
                { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f} },
                { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f} },
                { { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f} },
          
                //{ right side face
                { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f} },
                { { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f} },
                { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f} },
                { { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f} },
          
                //{ left side face
                { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f} },
                { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f} },
                { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f} },
                { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f} },
                                                        
                //{ back face                                   
                { { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f} },
                { {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f} },
                { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f} },
                { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f} },
          
                //{ top face
                { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f} },
                { {0.5f,   0.5f,  0.5f}, {1.0f, 0.0f} },
                { {0.5f,   0.5f, -0.5f}, {0.0f, 0.0f} },
                { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f} },
          
                //{ bottom face
                { { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f} },
                { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f} },
                { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f} },
                { {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f} },
            };

            const UINT vertexBufferSize = sizeof(vertices);

            // Note: using upload heaps to transfer static data like vert buffers is not 
            // recommended. Every time the GPU needs it, the upload heap will be marshalled 
            // over. Please read up on Default Heap usage. An upload heap is used here for 
            // code simplicity and because there are very few verts to actually transfer.

            CD3DX12_HEAP_PROPERTIES vBufferProps(D3D12_HEAP_TYPE_DEFAULT);
            auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
            ThrowIfFailed(m_device->CreateCommittedResource(
                &vBufferProps,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COPY_DEST,// we will start this heap in the copy destination state since we will copy data
                                                // from the upload heap to this heap
                nullptr,
                IID_PPV_ARGS(&m_vertexBuffer)));

            // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
            m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

            // create upload heap
            // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
            // We will upload the vertex buffer using this heap to the default heap
            ID3D12Resource* vBufferUploadHeap;
            CD3DX12_HEAP_PROPERTIES vBufferHeapProp(D3D12_HEAP_TYPE_UPLOAD);
            desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
            m_device->CreateCommittedResource(
                &vBufferHeapProp, // upload heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &desc, // resource description for a buffer
                D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
                nullptr,
                IID_PPV_ARGS(&vBufferUploadHeap));
            vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

            // store  ivertex buffern upload heap
            D3D12_SUBRESOURCE_DATA vertexData = {};
            vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
            vertexData.RowPitch = vertexBufferSize; // size of all our triangle vertex data
            vertexData.SlicePitch = vertexBufferSize; // also the size of our triangle vertex data

            UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &vertexData);

            // transition the vertex buffer data from copy destination state to vertex buffer state
            auto vResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            m_commandList->ResourceBarrier(1, &vResourceBarrier);

            DWORD indices[] = {
                        // ffront face
                0, 1, 2, // first triangle
                0, 3, 1, // second triangle

                // left face
                4, 5, 6, // first triangle
                4, 7, 5, // second triangle

                // right face
                8, 9, 10, // first triangle
                8, 11, 9, // second triangle

                // back face
                12, 13, 14, // first triangle
                12, 15, 13, // second triangle

                // top face
                16, 17, 18, // first triangle
                16, 19, 17, // second triangle

                // bottom face
                20, 21, 22, // first triangle
                20, 23, 21, // second triangle
            };

            int iBufferSize = sizeof(indices);

            numCubeIndices = iBufferSize / sizeof(DWORD);

            CD3DX12_HEAP_PROPERTIES iBufferProp(D3D12_HEAP_TYPE_DEFAULT);
            desc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);
            // create default heap to hold index buffer
            m_device->CreateCommittedResource(
                &iBufferProp, // a default heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &desc, // resource description for a buffer
                D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
                nullptr, // optimized clear value must be null for this type of resource
                IID_PPV_ARGS(&m_indexBuffer));

            m_indexBuffer->SetName(L"Index Buffer Resource Heap");

            ID3D12Resource* iBufferUploadHeap;
            CD3DX12_HEAP_PROPERTIES iBufferHeapProp(D3D12_HEAP_TYPE_UPLOAD);
            desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
            m_device->CreateCommittedResource(
                &iBufferHeapProp, // upload heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &desc, // resource description for a buffer
                D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
                nullptr,
                IID_PPV_ARGS(&iBufferUploadHeap));
            vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

            // store vertex buffer in upload heap
            D3D12_SUBRESOURCE_DATA indexData = {};
            indexData.pData = reinterpret_cast<BYTE*>(indices); // pointer to our index array
            indexData.RowPitch = iBufferSize; // size of all our index buffer
            indexData.SlicePitch = iBufferSize; // also the size of our index buffer

            // we are now creating a command with the command list to copy the data from
            // the upload heap to the default heap
            UpdateSubresources(m_commandList.Get(), m_indexBuffer.Get(), iBufferUploadHeap, 0, 0, 1, &indexData);

            auto iResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            m_commandList->ResourceBarrier(1, &iResourceBarrier);
                

            // Initialize the vertex buffer view.
            m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
            m_vertexBufferView.SizeInBytes = vertexBufferSize;

            m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
            m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
            m_indexBufferView.SizeInBytes = iBufferSize;
        }

        // Create the depth/stencil buffer
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
            depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
            depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

            D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
            depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
            depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
            depthOptimizedClearValue.DepthStencil.Stencil = 0;

            CD3DX12_HEAP_PROPERTIES dsBufferProp(D3D12_HEAP_TYPE_DEFAULT);
            auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, static_cast<LONG>(GetWidth()), static_cast<LONG>(GetHeight()), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            m_device->CreateCommittedResource(
                &dsBufferProp,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &depthOptimizedClearValue,
                IID_PPV_ARGS(&m_depthStencilBuffer)
            );
            NAME_D3D12_OBJECT(m_depthStencilBuffer);

            m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

        // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
        // the command list that references it has finished executing on the GPU.
        // We will flush the GPU at the end of this method to ensure the resource is not
        // prematurely destroyed.
        ComPtr<ID3D12Resource> textureUploadHeap;

        // Create the texture.
        {
            // Copy data to the intermediate upload heap and then schedule a copy 
            // from the upload heap to the Texture2D.
            //std::vector<UINT8> texture = GenerateTextureData();
            std::unique_ptr<uint8_t[]> ddsData;
            CreateDDSTextureFromFile(m_device.Get(), L"Assets/seafloor.dds", 0u, false, m_texture.ReleaseAndGetAddressOf(), m_srvHeap->GetCPUDescriptorHandleForHeapStart(), ddsData);

            // Describe and create a Texture2D.
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.MipLevels = 1;
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.Width = m_texture.Get()->GetDesc().Width;
            textureDesc.Height = m_texture.Get()->GetDesc().Height;
            textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            textureDesc.DepthOrArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            CD3DX12_HEAP_PROPERTIES texBufferProp(D3D12_HEAP_TYPE_DEFAULT);

            ThrowIfFailed(m_device->CreateCommittedResource(
                &texBufferProp,
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_texture)));

            NAME_D3D12_OBJECT(m_texture);

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

            // Create the GPU upload buffer.
            CD3DX12_HEAP_PROPERTIES texheapBufferProp(D3D12_HEAP_TYPE_UPLOAD);
            auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

            ThrowIfFailed(m_device->CreateCommittedResource(
                &texheapBufferProp,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&textureUploadHeap)));
            NAME_D3D12_OBJECT(textureUploadHeap);

            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = ddsData.get();
            textureData.RowPitch = textureDesc.Width * 4;
            textureData.SlicePitch = textureData.RowPitch * textureDesc.Height;

            UpdateSubresources(m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            m_commandList->ResourceBarrier(1, &barrier);

            // Describe and create a SRV for the texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = textureDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
        }

        #pragma region CreateFrameResource
        {
            // create the constant buffer resource heap
            // We will update the constant buffer one or more times per frame, so we will use only an upload heap
            // unlike previously we used an upload heap to upload the vertex and index data, and then copied over
            // to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
            // efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
            // will be modified and uploaded at least once per frame, so we only use an upload heap

            // first we will create a resource heap (upload heap) for each frame for the cubes constant buffers
            // As you can see, we are allocating 64KB for each resource we create. Buffer resource heaps must be
            // an alignment of 64KB. We are creating 3 resources, one for each frame. Each constant buffer is 
            // only a 4x4 matrix of floats in this tutorial. So with a float being 4 bytes, we have 
            // 16 floats in one constant buffer, and we will store 2 constant buffers in each
            // heap, one for each cube, thats only 64x2 bits, or 128 bits we are using for each
            // resource, and each resource must be at least 64KB (65536 bits)
            for (int i = 0; i < FrameCount; ++i)
            {
                CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
                auto desc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
                // create resource for cube 1
                ASSERT_SUCCEEDED(m_device->CreateCommittedResource(
                    &heapProps, // this heap will be used to upload the constant buffer data
                    D3D12_HEAP_FLAG_NONE, // no flags
                    &desc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
                    D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
                    nullptr, // we do not have use an optimized clear value for constant buffers
                    IID_PPV_ARGS(&constantBufferUploadHeaps[i])));
                constantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

                ZeroMemory(&cbPerObject, sizeof(cbPerObject));

                CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

                // map the resource heap to get a gpu virtual address to the beginning of the heap
                ASSERT_SUCCEEDED(constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i])));

                // Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
                // so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
                memcpy(cbvGPUAddress[i], &cbPerObject, sizeof(cbPerObject)); // cube1's constant buffer data
                memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube2's constant buffer data
            }
        }
        #pragma endregion CreateFrameResource

        // Now we execute the command list to upload the initial assets (triangle data)
        m_commandList->Close();
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        {
            ThrowIfFailed(m_device->CreateFence(m_fenceValue[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
            m_fenceValue[m_frameIndex]++;

            // Create an event handle to use for frame synchronization.
            m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForGPU();

        //// set starting cubes position
        //// first cube
        //cube1Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // set cube 1's position
        //XMVECTOR posVec = XMLoadFloat4(&cube1Position); // create xmvector for cube1's position

        //XMMATRIX tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
        //XMStoreFloat4x4(&cube1RotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
        //XMStoreFloat4x4(&cube1WorldMat, tmpMat); // store cube1's world matrix

        //// second cube
        //cube2PositionOffset = XMFLOAT4(1.5f, 0.0f, 0.0f, 0.0f);
        //posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cube1Position); // create xmvector for cube2's position
        //                                                                            // we are rotating around cube1 here, so add cube2's position to cube1

        //tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube2's position offset vector
        //XMStoreFloat4x4(&cube2RotMat, XMMatrixIdentity()); // initialize cube2's rotation matrix to identity matrix
        //XMStoreFloat4x4(&cube2WorldMat, tmpMat); // store cube2's world matrix
    }

    void D3DApp::PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        ASSERT_SUCCEEDED(m_commandAllocator[m_frameIndex]->Reset());

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        ASSERT_SUCCEEDED(m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get()));

        // Set necessary state.
        m_commandList->SetGraphicsRootSignature(m_signature->GetRootSignature());

        ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
        m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        m_commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // Indicate that the back buffer will be used as a render target.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        // clear the depth/stencil buffer
        m_commandList->ClearDepthStencilView(m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->IASetIndexBuffer(&m_indexBufferView);

        // set camera matrix in the constant buffer
        m_commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[m_frameIndex]->GetGPUVirtualAddress() + m_MainCamera.GetOffsetInConstantBuffer());

        // set cube1's constant buffer
        m_commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[m_frameIndex]->GetGPUVirtualAddress());

        // draw first cube
        m_commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

        // second cube

        // set cube2's constant buffer. You can see we are adding the size of ConstantBufferPerObject to the constant buffer
        // resource heaps address. This is because cube1's constant buffer is stored at the beginning of the resource heap, while
        // cube2's constant buffer data is stored after (256 bits from the start of the heap).
        m_commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[m_frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

        // draw second cube
        m_commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

        // Indicate that the back buffer will now be used to present.
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &barrier);

        ASSERT_SUCCEEDED(m_commandList->Close());
    }

    void D3DApp::WaitForGPU()
    {
        // Schedule a Signal command in the queue.
        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_frameIndex]));

        // Wait until the fence has been processed.
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

        // Increment the fence value for the current frame.
        m_fenceValue[m_frameIndex]++;
    }

    void D3DApp::MoveToNextFrame()
    {
        // Schedule a Signal command in the queue.
        const UINT64 currentFenceValue = m_fenceValue[m_frameIndex];
        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

        // Update the frame index.
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_fence->GetCompletedValue() < m_fenceValue[m_frameIndex])
        {
            ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent));
            WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
        }

        // Set the fence value for the next frame.
        m_fenceValue[m_frameIndex] = currentFenceValue + 1;
    }

    void D3DApp::UpdateConstantBuffers()
    {

    }

    // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
    _Use_decl_annotations_
        void D3DApp::GetHardwareAdapter(
            IDXGIFactory1* pFactory,
            IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach();
    }

    std::wstring D3DApp::GetAssetFullPath(LPCWSTR assetName)
    {
        return  m_assetsPath + assetName;;
    }

    HICON hIcon;

    LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	int D3DApp::InitApplication(D3DApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow)
	{
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAINICON));

        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInst;

        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = className;

        wcex.hIcon = hIcon;
        wcex.hIconSm = hIcon;
        ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");

        // Create and display window 
        RECT rc = { 0, 0, static_cast<LONG>(GetWidth()), static_cast<LONG>(GetHeight())};
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        m_hWnd = CreateWindow(
            className,        
            className,          // window caption
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            nullptr,
            nullptr,
            hInst,
            &app);

        ASSERT(m_hWnd != 0);

        app.OnInit();
        ShowWindow(m_hWnd, nCmdShow);

        return app.Run(app);
	}

    int D3DApp::Run(D3DApp& app)
    {
        // Main sample loop.
        MSG msg = {};

        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {
                if (!m_AppPaused) {
                    //CalculateFrameStats();
                    app.OnUpdate();
                    app.OnRender();
                }
                else {
                    Sleep(100);
                }
                
            }
        }

        app.OnDestroy();
        return static_cast<char>(msg.wParam);
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        D3DApp* app = reinterpret_cast<D3DApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (message)
        {
        case WM_CREATE:
        {
            // Save the DXSample* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

        case WM_SIZE:
            //Display::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
            break;

        case WM_KEYDOWN:
            if (app)
            {
                app->OnKeyDown(static_cast<UINT8>(wParam));
            }
            return 0;

        case WM_KEYUP:
            if (app)
            {
                app->OnKeyUp(static_cast<UINT8>(wParam));
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}