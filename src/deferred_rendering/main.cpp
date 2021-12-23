#include <SDL.h>
#include <SDL_syswm.h>

#include <tiny_obj_loader.h>

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <DirectXMath.h>

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <functional>

using namespace DirectX;

#define RETURN_FALSE_IF(hresult) \
if(FAILED(hresult)) { SDL_TriggerBreakpoint(); return false; }

#define COM_RELEASE(com) \
if(com) { com->Release(); com = nullptr; }

/*template<typename T>
class UniqueComPtr
{
public:
    UniqueComPtr() = default;
    UniqueComPtr(T* com)
    : m_Com{ com }
    {

    }

    ~UniqueComPtr()
    {
        COM_RELEASE(m_Com);
    }

    T* operator -> ()
    {
        return m_Com;
    }

    bool operator ! ()
    {
        return !m_Com;
    }

    T* Get() 
    { 
        return m_Com; 
    }

    void Reset(T* com)
    {
        COM_RELEASE(m_Com);
        m_Com = com;
    }

private:
    T* m_Com{ nullptr };
};*/

#pragma region Device 

class Device
{
public:
    ~Device()
    {
        COM_RELEASE(m_RasterState);
        COM_RELEASE(m_DSV);
        COM_RELEASE(m_DSS);
        COM_RELEASE(m_DSB);
        COM_RELEASE(m_RTV);
        COM_RELEASE(m_Contex);
        COM_RELEASE(m_Device);
        COM_RELEASE(m_SwapChain);
    }

    bool Init(HWND hwnd, int width, int height)
    {
        // Create swap chain

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = true;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
        HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr
            , D3D_DRIVER_TYPE_HARDWARE
            , nullptr
            , 0
            , featureLevels
            , 1
            , D3D11_SDK_VERSION
            , &swapChainDesc
            , &m_SwapChain
            , &m_Device
            , nullptr
            , &m_Contex);

        RETURN_FALSE_IF(result);

        // Create RT & RTV
        ID3D11Texture2D* backBuffer = nullptr;
        
        result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        RETURN_FALSE_IF(result);

        result = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RTV);
        RETURN_FALSE_IF(result);

        COM_RELEASE(backBuffer);

        /*// Create DB & DBV

        D3D11_TEXTURE2D_DESC depthBufferDesc;
        ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
        depthBufferDesc.Width = width;
        depthBufferDesc.Height = height;
        depthBufferDesc.MipLevels = 1;
        depthBufferDesc.ArraySize = 1;
        depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthBufferDesc.SampleDesc.Count = 1;
        depthBufferDesc.SampleDesc.Quality = 0;
        depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthBufferDesc.CPUAccessFlags = 0;
        depthBufferDesc.MiscFlags = 0;

        result = m_Device->CreateTexture2D(&depthBufferDesc, nullptr, &m_DSB);
        RETURN_FALSE_IF(result);

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;

        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        result = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DSS);
        RETURN_FALSE_IF(result);

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        result = m_Device->CreateDepthStencilView(m_DSB, &depthStencilViewDesc, &m_DSV);
        RETURN_FALSE_IF(result);

        m_Contex->OMSetDepthStencilState(m_DSS, 1);
        m_Contex->OMSetRenderTargets(1, &m_RTV, m_DSV);*/

        // Rasterizer

        D3D11_RASTERIZER_DESC rasterDesc;
        ZeroMemory(&rasterDesc, sizeof(rasterDesc));
        rasterDesc.AntialiasedLineEnable = false;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.MultisampleEnable = false;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;

        result = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterState);
        RETURN_FALSE_IF(result);

        m_Contex->RSSetState(m_RasterState);

        // Viewport

        m_Viewport.Width = (float)width;
        m_Viewport.Height = (float)height;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;

        m_Contex->RSSetViewports(1, &m_Viewport);
        return true;
    }

    void BeforeFrame()
    {
        float clearColor[] =
        {
            0.0f,
            0.0f,
            0.0f,
            1.0f
        };

        m_Contex->ClearRenderTargetView(m_RTV, clearColor);
        //m_Contex->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void Present()
    {
        m_SwapChain->Present(0, 0);
    }

    ID3D11Device* GetDevice() { return m_Device; }
    ID3D11DeviceContext* GetContext() { return m_Contex; }
    ID3D11RenderTargetView* GetRTV() const { return m_RTV; }

    const D3D11_VIEWPORT& GetViewport() const { return m_Viewport; }

private:
    IDXGISwapChain* m_SwapChain{ nullptr };

    ID3D11Device* m_Device{ nullptr };
    ID3D11DeviceContext* m_Contex{ nullptr };

    ID3D11RenderTargetView* m_RTV{ nullptr };

    ID3D11Texture2D* m_DSB{ nullptr };
    ID3D11DepthStencilState* m_DSS{ nullptr };
    ID3D11DepthStencilView* m_DSV{ nullptr };

    ID3D11RasterizerState* m_RasterState{ nullptr };

    D3D11_VIEWPORT m_Viewport;
};

Device& GetGlobalDevice()
{
    static Device instance;
    return instance;
}

#define DEVICE GetGlobalDevice()

#pragma endregion

class DeferredBuffers
{
public:
    enum BufferType
    {
        NORMAL = 0,
        COLOR,
        COUNT
    };

public:
    ~DeferredBuffers()
    {
        COM_RELEASE(m_DSV);
        COM_RELEASE(m_DSS);
        COM_RELEASE(m_Depth);

        for (int i = 0; i < COUNT; ++i)
        {
            COM_RELEASE(m_SRV[i]);
            COM_RELEASE(m_RTV[i]);
            COM_RELEASE(m_Textures[i]);
        }
    }

    bool Create()
    {
        D3D11_TEXTURE2D_DESC textureDesc;
        ZeroMemory(&textureDesc, sizeof(textureDesc));
        textureDesc.Width = DEVICE.GetViewport().Width;
        textureDesc.Height = DEVICE.GetViewport().Height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        ZeroMemory(&rtvDesc, sizeof(rtvDesc));
        rtvDesc.Format = textureDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        for (int i = 0; i < COUNT; ++i)
        {
            HRESULT result = DEVICE.GetDevice()->CreateTexture2D(&textureDesc, nullptr, &m_Textures[i]);
            RETURN_FALSE_IF(result);

            result = DEVICE.GetDevice()->CreateRenderTargetView(m_Textures[i], &rtvDesc, &m_RTV[i]);
            RETURN_FALSE_IF(result);

            result = DEVICE.GetDevice()->CreateShaderResourceView(m_Textures[i], &srvDesc, &m_SRV[i]);
            RETURN_FALSE_IF(result);
        }

        D3D11_TEXTURE2D_DESC depthTextureDesc;
        ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
        depthTextureDesc.Width = DEVICE.GetViewport().Width;
        depthTextureDesc.Height = DEVICE.GetViewport().Height;
        depthTextureDesc.MipLevels = 1;
        depthTextureDesc.ArraySize = 1;
        depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthTextureDesc.SampleDesc.Count = 1;
        depthTextureDesc.SampleDesc.Quality = 0;
        depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthTextureDesc.CPUAccessFlags = 0;
        depthTextureDesc.MiscFlags = 0;

        HRESULT result = DEVICE.GetDevice()->CreateTexture2D(&depthTextureDesc, nullptr, &m_Depth);
        RETURN_FALSE_IF(result);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        ZeroMemory(&dsvDesc, sizeof(dsvDesc));
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        result = DEVICE.GetDevice()->CreateDepthStencilView(m_Depth, &dsvDesc, &m_DSV);
        RETURN_FALSE_IF(result);

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;

        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        result = DEVICE.GetDevice()->CreateDepthStencilState(&depthStencilDesc, &m_DSS);
        RETURN_FALSE_IF(result);

        return true;
    }

    void Bind()
    {
        DEVICE.GetContext()->OMSetDepthStencilState(m_DSS, 1);
        DEVICE.GetContext()->OMSetRenderTargets(COUNT, m_RTV.data(), m_DSV);
    }

    void Clear()
    {
        float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        for (ID3D11RenderTargetView* rtv : m_RTV)
        {
            DEVICE.GetContext()->ClearRenderTargetView(rtv, color);
        }

        DEVICE.GetContext()->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    ID3D11ShaderResourceView* GetSRV(BufferType type) { return m_SRV[type]; }

private:
    std::array<ID3D11Texture2D*, COUNT> m_Textures;
    std::array<ID3D11RenderTargetView*, COUNT> m_RTV;
    std::array<ID3D11ShaderResourceView*, COUNT> m_SRV;

    ID3D11Texture2D* m_Depth{ nullptr };
    ID3D11DepthStencilView* m_DSV{ nullptr };
    ID3D11DepthStencilState* m_DSS{ nullptr };
};

#pragma region Shaders

namespace shader_utils
{
    ID3D10Blob* Compile(const std::wstring& path, const std::string& entry, const std::string& target)
    {
        ID3D10Blob* bytecodeBuffer = nullptr;
        ID3D10Blob* errorBuffer = nullptr;

        HRESULT result = D3DCompileFromFile(path.c_str()
            , nullptr
            , nullptr
            , entry.c_str()
            , target.c_str()
            , D3D10_SHADER_ENABLE_STRICTNESS
            , 0
            , &bytecodeBuffer
            , &errorBuffer);

        if (errorBuffer)
        {
            char buf[1024] = { 0 };
            std::memcpy(buf, errorBuffer->GetBufferPointer(), std::min(1024, (int)errorBuffer->GetBufferSize()));
            buf[1023] = 0;

            std::wcout << L"HLSL [" << path << L"]: " << buf << std::endl;
        }

        COM_RELEASE(errorBuffer);
        return bytecodeBuffer;
    }
}

struct TransformConstantBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

template<typename T>
class ScopedCOM
{
public:
    ScopedCOM(T* com)
        : m_Com{ com }
    {

    }

    ~ScopedCOM()
    {
        COM_RELEASE(m_Com);
    }

private:
    T* m_Com{ nullptr };
};

class GeometryPass
{
public:
    bool Create()
    {
        if (!CreateVS() || !CreatePS())
        {
            return false;
        }

        D3D11_BUFFER_DESC constantDesc;
        ZeroMemory(&constantDesc, sizeof(constantDesc));
        constantDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantDesc.ByteWidth = sizeof(TransformConstantBuffer);
        constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantDesc.MiscFlags = 0;
        constantDesc.StructureByteStride = 0;

        HRESULT result = DEVICE.GetDevice()->CreateBuffer(&constantDesc, nullptr, &m_ConstantBuffer);
        return SUCCEEDED(result);
    }

    void Bind(DeferredBuffers& buffers)
    {
        buffers.Clear();
        buffers.Bind();

        DEVICE.GetContext()->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
        DEVICE.GetContext()->IASetInputLayout(m_IL);
        DEVICE.GetContext()->VSSetShader(m_VS, nullptr, 0);
        DEVICE.GetContext()->PSSetShader(m_PS, nullptr, 0);
    }

    bool SetConstantBuffer(TransformConstantBuffer buffer)
    {
        buffer.world = XMMatrixTranspose(buffer.world);
        buffer.view = XMMatrixTranspose(buffer.view);
        buffer.projection = XMMatrixTranspose(buffer.projection);

        D3D11_MAPPED_SUBRESOURCE resource;

        HRESULT result = DEVICE.GetContext()->Map(m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        RETURN_FALSE_IF(result);

        *reinterpret_cast<TransformConstantBuffer*>(resource.pData) = buffer;
        DEVICE.GetContext()->Unmap(m_ConstantBuffer, 0);

        return true;
    }

private:
    bool CreateVS()
    {
        ID3D10Blob* bytecode = shader_utils::Compile(L"vs_g_pass.hlsl", "VSGMain", "vs_5_0");
        if (!bytecode)
        {
            return false;
        }

        HRESULT result = DEVICE.GetDevice()->CreateVertexShader(bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , nullptr
            , &m_VS);

        if (FAILED(result))
        {
            COM_RELEASE(bytecode);
            return false;
        }

        std::array<D3D11_INPUT_ELEMENT_DESC, 4> layoutDesc;
        ZeroMemory(layoutDesc.data(), layoutDesc.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));

        layoutDesc[0].SemanticName = "POSITION";
        layoutDesc[0].SemanticIndex = 0;
        layoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layoutDesc[0].InputSlot = 0;
        layoutDesc[0].AlignedByteOffset = 0;
        layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[0].InstanceDataStepRate = 0;

        layoutDesc[1].SemanticName = "NORMAL";
        layoutDesc[1].SemanticIndex = 0;
        layoutDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layoutDesc[1].InputSlot = 0;
        layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[1].InstanceDataStepRate = 0;

        layoutDesc[2].SemanticName = "TEXCOORD";
        layoutDesc[2].SemanticIndex = 0;
        layoutDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
        layoutDesc[2].InputSlot = 0;
        layoutDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[2].InstanceDataStepRate = 0;

        layoutDesc[3].SemanticName = "COLOR";
        layoutDesc[3].SemanticIndex = 0;
        layoutDesc[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        layoutDesc[3].InputSlot = 0;
        layoutDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[3].InstanceDataStepRate = 0;

        result = DEVICE.GetDevice()->CreateInputLayout(layoutDesc.data()
            , layoutDesc.size()
            , bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , &m_IL);

        COM_RELEASE(bytecode);
        return SUCCEEDED(result);
    }

    bool CreatePS()
    {
        ID3D10Blob* bytecode = shader_utils::Compile(L"ps_g_pass.hlsl", "PSGMain", "ps_5_0");
        if (!bytecode)
        {
            return false;
        }

        HRESULT result = DEVICE.GetDevice()->CreatePixelShader(bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , nullptr
            , &m_PS);

        COM_RELEASE(bytecode);
        return SUCCEEDED(result);
    }

private:
    ID3D11VertexShader* m_VS;
    ID3D11PixelShader* m_PS;
    ID3D11InputLayout* m_IL;
    ID3D11Buffer* m_ConstantBuffer;
};

class LightPass
{
public:
    struct LightConstantBuffer
    {
        XMVECTOR direction;
    };

public:
    bool Create()
    {
        if (!CreateVS() || !CreatePS())
        {
            return false;
        }

        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory(&samplerDesc, sizeof(samplerDesc));
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0;
        samplerDesc.BorderColor[1] = 0;
        samplerDesc.BorderColor[2] = 0;
        samplerDesc.BorderColor[3] = 0;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        HRESULT result = DEVICE.GetDevice()->CreateSamplerState(&samplerDesc, &m_Sampler);
        RETURN_FALSE_IF(result);

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(LightConstantBuffer);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        result = DEVICE.GetDevice()->CreateBuffer(&desc, nullptr, &m_ConstantBuffer);
        RETURN_FALSE_IF(result);

        D3D11_DEPTH_STENCIL_DESC dssDesc;
        ZeroMemory(&dssDesc, sizeof(dssDesc));
        dssDesc.DepthEnable = false;
        dssDesc.StencilEnable = false;

        result = DEVICE.GetDevice()->CreateDepthStencilState(&dssDesc, &m_DSS);
        return SUCCEEDED(result);
    }

    void Bind(DeferredBuffers& buffers)
    {
        DEVICE.GetContext()->OMSetDepthStencilState(m_DSS, 1);

        ID3D11RenderTargetView* rtv = DEVICE.GetRTV();
        DEVICE.GetContext()->OMSetRenderTargets(1, &rtv, nullptr);

        DEVICE.GetContext()->VSSetShader(m_VS, nullptr, 0);
        DEVICE.GetContext()->PSSetShader(m_PS, nullptr, 0);

        DEVICE.GetContext()->PSSetConstantBuffers(0, 1, &m_ConstantBuffer);
        DEVICE.GetContext()->PSSetSamplers(0, 1, &m_Sampler);

        ID3D11ShaderResourceView* srvNormal = buffers.GetSRV(DeferredBuffers::NORMAL);
        DEVICE.GetContext()->PSSetShaderResources(0, 1, &srvNormal);

        ID3D11ShaderResourceView* srvColor = buffers.GetSRV(DeferredBuffers::COLOR);
        DEVICE.GetContext()->PSSetShaderResources(1, 1, &srvColor);

        DEVICE.GetContext()->IASetInputLayout(m_IL);
    }

    bool SetConstantBuffer(LightConstantBuffer buffer)
    {
        D3D11_MAPPED_SUBRESOURCE resource;

        HRESULT result = DEVICE.GetContext()->Map(m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        RETURN_FALSE_IF(result);

        *reinterpret_cast<LightConstantBuffer*>(resource.pData) = buffer;
        DEVICE.GetContext()->Unmap(m_ConstantBuffer, 0);

        return true;
    }

private:
    bool CreateVS()
    {
        ID3D10Blob* bytecode = shader_utils::Compile(L"vs_l_pass.hlsl", "VSLMain", "vs_5_0");
        if (!bytecode)
        {
            return false;
        }

        HRESULT result = DEVICE.GetDevice()->CreateVertexShader(bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , nullptr
            , &m_VS);

        if (FAILED(result))
        {
            COM_RELEASE(bytecode);
            return false;
        }

        std::array<D3D11_INPUT_ELEMENT_DESC, 4> layoutDesc;
        ZeroMemory(layoutDesc.data(), layoutDesc.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));

        layoutDesc[0].SemanticName = "POSITION";
        layoutDesc[0].SemanticIndex = 0;
        layoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layoutDesc[0].InputSlot = 0;
        layoutDesc[0].AlignedByteOffset = 0;
        layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[0].InstanceDataStepRate = 0;

        layoutDesc[1].SemanticName = "NORMAL";
        layoutDesc[1].SemanticIndex = 0;
        layoutDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layoutDesc[1].InputSlot = 0;
        layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[1].InstanceDataStepRate = 0;

        layoutDesc[2].SemanticName = "TEXCOORD";
        layoutDesc[2].SemanticIndex = 0;
        layoutDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
        layoutDesc[2].InputSlot = 0;
        layoutDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[2].InstanceDataStepRate = 0;

        layoutDesc[3].SemanticName = "COLOR";
        layoutDesc[3].SemanticIndex = 0;
        layoutDesc[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        layoutDesc[3].InputSlot = 0;
        layoutDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[3].InstanceDataStepRate = 0;

        result = DEVICE.GetDevice()->CreateInputLayout(layoutDesc.data()
            , layoutDesc.size()
            , bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , &m_IL);

        COM_RELEASE(bytecode);
        return SUCCEEDED(result);
    }

    bool CreatePS()
    {
        ID3D10Blob* bytecode = shader_utils::Compile(L"ps_l_pass.hlsl", "PSLMain", "ps_5_0");
        if (!bytecode)
        {
            return false;
        }

        HRESULT result = DEVICE.GetDevice()->CreatePixelShader(bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , nullptr
            , &m_PS);

        COM_RELEASE(bytecode);
        return SUCCEEDED(result);
    }

private:
    ID3D11VertexShader* m_VS{ nullptr };
    ID3D11PixelShader* m_PS{ nullptr };
    ID3D11InputLayout* m_IL{ nullptr };
    ID3D11DepthStencilState* m_DSS{ nullptr };
    ID3D11Buffer* m_ConstantBuffer{ nullptr };
    ID3D11SamplerState* m_Sampler{ nullptr };

};

#pragma endregion

class Mesh
{
public:
    using IndexType_t = unsigned long;
    using HashType_t = unsigned long long;

public:
    struct Vertex
    {
        XMFLOAT3 pos = {0.0f, 0.0f, 0.0f};
        XMFLOAT3 normal = {0.0f, 0.0f, 0.0f};
        XMFLOAT2 texCoord = {0.0f, 0.0f};
        XMFLOAT4 color = {1.0f, 0.0f, 0.0f, 1.0f};
    };

public:
    ~Mesh()
    {
        COM_RELEASE(m_VB);
        COM_RELEASE(m_IB);
    }

    bool Create(const std::vector<Vertex>& vertices, const std::vector<IndexType_t>& indices)
    {
        if (CreateBuffer(sizeof(Vertex) * static_cast<UINT>(vertices.size()), vertices.data(), &m_VB, D3D11_BIND_VERTEX_BUFFER))
        {
            if (CreateBuffer(sizeof(unsigned long) * static_cast<UINT>(indices.size()), indices.data(), &m_IB, D3D11_BIND_INDEX_BUFFER))
            {
                m_IndexCount = indices.size();
                return true;
            }
        }

        return false;
    }

    bool Create(const std::string& path)
    {
        tinyobj::attrib_t attrib;
        
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;
        
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            std::cout << "tinyobj ERROR: " << err << std::endl;
            return false;
        }

        if (!warn.empty())
        {
            std::cout << "tinyobj WARNING: " << warn << std::endl;
        }

        struct HashedVertex
        {
            HashType_t hash;
            Vertex vtx;
        };

        std::vector<HashedVertex> vertices;
        std::vector<IndexType_t> indices;

        for (const tinyobj::shape_t& shape : shapes)
        {
            for (const tinyobj::index_t& index : shape.mesh.indices)
            {
                int vIdx = index.vertex_index;
                int nIdx = index.normal_index;

                // Hashing: https://stackoverflow.com/questions/682438/hash-function-providing-unique-uint-from-an-integer-coordinate-pair
                // Hashing: http://szudzik.com/ElegantPairing.pdf

                const HashType_t hash = Hash(vIdx, nIdx);

                auto it = std::find_if(vertices.begin(), vertices.end(), [hash](const HashedVertex& vtx) { return vtx.hash == hash; });
                if (it == vertices.end())
                {
                    XMFLOAT3 vertex
                    {
                        attrib.vertices[3 * vIdx],
                        attrib.vertices[3 * vIdx + 1],
                        attrib.vertices[3 * vIdx + 2]
                    };

                    XMFLOAT3 normal
                    {
                        attrib.normals[3 * nIdx],
                        attrib.normals[3 * nIdx + 1],
                        attrib.normals[3 * nIdx + 2]
                    };

                    HashedVertex vtx;
                    vtx.hash = hash;
                    vtx.vtx.pos = vertex;
                    vtx.vtx.normal = normal;

                    vertices.push_back(vtx);
                    it = std::prev(vertices.end());
                }

                indices.push_back(std::distance(vertices.begin(), it));
            }
        }

        std::vector<Vertex> finalizedVertices;
        finalizedVertices.reserve(vertices.size());

        for (const HashedVertex& vtx : vertices)
        {
            finalizedVertices.push_back(vtx.vtx);
        }

        return Create(finalizedVertices, indices);
    }

    void Bind()
    {
        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        DEVICE.GetContext()->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
        DEVICE.GetContext()->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, 0);
        DEVICE.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    unsigned long GetIndexCount() const { return m_IndexCount; }

private:
    unsigned long Hash(int a, int b)
    {
        return a >= b ? a * a + a + b : a + b * b;
    }
    bool CreateBuffer(UINT bytes, const void* data, ID3D11Buffer** buffer, D3D11_BIND_FLAG bindFlags) const
    {
        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = bytes;
        desc.BindFlags = bindFlags;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA bufferData;
        bufferData.pSysMem = data;
        bufferData.SysMemPitch = 0;
        bufferData.SysMemSlicePitch = 0;

        return SUCCEEDED(DEVICE.GetDevice()->CreateBuffer(&desc, &bufferData, buffer));
    }

private:
    ID3D11Buffer* m_VB{ nullptr };
    ID3D11Buffer* m_IB{ nullptr };

    unsigned long m_IndexCount{ 0 };
};

class Camera
{
public:
    void Init(int w, int h)
    {
        m_ViewMTX = XMMatrixIdentity();
        m_ProjectionMTX = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), static_cast<float>(w) / static_cast<float>(h), 0.1f, 100.0f);
    }

    void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target)
    {
        XMVECTOR ssePos = XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
        XMVECTOR sseTarget = XMVectorSet(target.x, target.y, target.z, 1.0f);
        XMVECTOR sseUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        
        m_ViewMTX = XMMatrixLookAtLH(ssePos, sseTarget, sseUp);
    }

    const XMFLOAT3& GetPos() const { return m_Pos; }
    const XMFLOAT3& GetView() const { return m_View; }

    const XMMATRIX& GetViewMTX() const { return m_ViewMTX; }
    const XMMATRIX& GetProjectionMTX() const { return m_ProjectionMTX; }

private:
    XMFLOAT3 m_Pos;
    XMFLOAT3 m_View;

    XMMATRIX m_ViewMTX;
    XMMATRIX m_ProjectionMTX;
};

class Scene
{
public:
    bool Init()
    {
        m_Camera.Init(DEVICE.GetViewport().Width, DEVICE.GetViewport().Height);
        m_Camera.LookAt(XMFLOAT3{ 3.0f, 3.0f, 3.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });

        std::vector<Mesh::Vertex> vertices
        {
            {XMFLOAT3{-1.0f, 1.0f, 0.0f}, XMFLOAT3{0.0f, 0.0f, 0.0f}, XMFLOAT2{0.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}},
            {XMFLOAT3{1.0f, 1.0f, 0.0f}, XMFLOAT3{0.0f, 0.0f, 0.0f}, XMFLOAT2{1.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}},
            {XMFLOAT3{1.0f, -1.0f, 0.0f}, XMFLOAT3{0.0f, 0.0f, 0.0f}, XMFLOAT2{1.0f, 1.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}},
            {XMFLOAT3{-1.0f, -1.0f, 0.0f}, XMFLOAT3{0.0f, 0.0f, 0.0f}, XMFLOAT2{0.0f, 1.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}}
        };

        std::vector<Mesh::IndexType_t> indices
        {
            0, 1, 2,
            0, 2, 3
        };

        if (m_GPass.Create()
            && m_LPass.Create()
            && m_Buffers.Create()
            && m_Mesh.Create("torus.obj")
            && m_FullscreenQuad.Create(vertices, indices))
        {
            TransformConstantBuffer gPassShaderConstant;
            gPassShaderConstant.world = XMMatrixIdentity();
            gPassShaderConstant.view = m_Camera.GetViewMTX();
            gPassShaderConstant.projection = m_Camera.GetProjectionMTX();
            m_GPass.SetConstantBuffer(gPassShaderConstant);

            LightPass::LightConstantBuffer lPassShaderConstant;
            lPassShaderConstant.direction = XMVectorSet(-1.0, 1.0, -1.0, 0.0);
            m_LPass.SetConstantBuffer(lPassShaderConstant);

            return true;
        }

        return false;
    }

    void Update(float delta)
    {
        // Geometry pass

        m_Mesh.Bind();
        m_GPass.Bind(m_Buffers);

        DEVICE.GetContext()->DrawIndexed(m_Mesh.GetIndexCount(), 0, 0);

        // Light pass

        m_FullscreenQuad.Bind();
        m_LPass.Bind(m_Buffers);

        DEVICE.GetContext()->DrawIndexed(m_FullscreenQuad.GetIndexCount(), 0, 0);
    }

private:
    Camera m_Camera;

    DeferredBuffers m_Buffers;
    GeometryPass m_GPass;
    LightPass m_LPass;

    Mesh m_Mesh;
    Mesh m_FullscreenQuad;
};

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        return -1;
    }

    const int WindowWidth = 1920 * 0.5f;
    const int WindowHeight = 1080 * 0.5f;

    SDL_Window* sdlWindow = SDL_CreateWindow("deferred_rendering"
        , SDL_WINDOWPOS_CENTERED
        , SDL_WINDOWPOS_CENTERED
        , WindowWidth
        , WindowHeight
        , 0);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdlWindow, &wmInfo);

    Scene scene;
    if (DEVICE.Init(wmInfo.info.win.window, WindowWidth, WindowHeight) && scene.Init())
    {
        const float FrameTime = 1.0f / 60.0f;
        float delta = 0.0f;

        bool isRunning = true;
        while (isRunning)
        {
            Uint64 start = SDL_GetTicks64();

            SDL_Event event;
            if (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                {
                    isRunning = false;
                }
                break;

                default:
                    break;
                }
            }

            DEVICE.BeforeFrame();
            scene.Update(delta);
            DEVICE.Present();

            float epleased = static_cast<float>(SDL_GetTicks64() - start) / 1000.0f;
            if (epleased < FrameTime)
            {
                Sleep((FrameTime - epleased) * 1000.0f);
            }

            delta = static_cast<float>(SDL_GetTicks64() - start) / 1000.0f;
        }
    }

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return 0;
}