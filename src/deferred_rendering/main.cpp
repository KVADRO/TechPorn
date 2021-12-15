#include <SDL.h>
#include <SDL_syswm.h>

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

using namespace DirectX;

#define RETURN_FALSE_IF(hresult) \
if(FAILED(hresult)) { SDL_TriggerBreakpoint(); return false; }

#define COM_RELEASE(com) \
if(com) { com->Release(); com = nullptr; }

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

        // Create DB & DBV

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
        m_Contex->OMSetRenderTargets(1, &m_RTV, m_DSV);

        // Rasterizer

        D3D11_RASTERIZER_DESC rasterDesc;
        ZeroMemory(&rasterDesc, sizeof(rasterDesc));
        rasterDesc.AntialiasedLineEnable = false;
        rasterDesc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_BACK;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
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
        m_Contex->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void Present()
    {
        m_SwapChain->Present(0, 0);
    }

    ID3D11Device* GetDevice() { return m_Device; }
    ID3D11DeviceContext* GetContext() { return m_Contex; }
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

template<typename T>
class Shader
{
public:
    virtual ~Shader()
    {
        COM_RELEASE(m_Shader);
    }

    bool Create(const std::wstring& path, const std::string& entry)
    {
        ID3D10Blob* bytecodeBuffer = nullptr;
        ID3D10Blob* errorBuffer = nullptr;

        HRESULT result = D3DCompileFromFile(path.c_str()
            , nullptr
            , nullptr
            , entry.c_str()
            , GetCompileTarget().c_str()
            , D3D10_SHADER_ENABLE_STRICTNESS
            , 0
            , &bytecodeBuffer
            , &errorBuffer);

        if (errorBuffer)
        {
            PrintMessage(errorBuffer, path);
        }

        if (SUCCEEDED(result))
        {
            result = CreateShaderObject(bytecodeBuffer);
        }

        COM_RELEASE(bytecodeBuffer);
        COM_RELEASE(errorBuffer);

        return SUCCEEDED(result);
    }

protected:
    virtual bool CreateShaderObject(ID3D10Blob* bytecode) = 0;
    virtual std::string GetCompileTarget() const = 0;

    void PrintMessage(ID3D10Blob* buffer, const std::wstring& path)
    {
        std::wcout << L"HLSL [" << path << L"]: " << reinterpret_cast<LPWSTR>(buffer->GetBufferPointer()) << std::endl;
    }

protected:
    T* m_Shader{ nullptr };
};

class VertexShader : public Shader<ID3D11VertexShader>
{
    using Parent_t = Shader<ID3D11VertexShader>;

public:
    struct ConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

public:
    ~VertexShader()
    {
        COM_RELEASE(m_IL);
        COM_RELEASE(m_ConstantBuffer);
    }

    bool SetConstantBuffer(ConstantBuffer buffer)
    {
        buffer.world = XMMatrixTranspose(buffer.world);
        buffer.view = XMMatrixTranspose(buffer.view);
        buffer.projection = XMMatrixTranspose(buffer.projection);

        D3D11_MAPPED_SUBRESOURCE resource;
        
        HRESULT result = DEVICE.GetContext()->Map(m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        RETURN_FALSE_IF(result);

        *reinterpret_cast<ConstantBuffer*>(resource.pData) = buffer;
        DEVICE.GetContext()->Unmap(m_ConstantBuffer, 0);

        return true;
    }

    void Bind()
    {
        DEVICE.GetContext()->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
        DEVICE.GetContext()->IASetInputLayout(m_IL);
        DEVICE.GetContext()->VSSetShader(m_Shader, nullptr, 0);
    }

private:
    bool CreateShaderObject(ID3D10Blob* bytecode) override
    {
        HRESULT result = DEVICE.GetDevice()->CreateVertexShader(bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , nullptr
            , &m_Shader);

        RETURN_FALSE_IF(result);

        D3D11_INPUT_ELEMENT_DESC layoutDesc[2] = {0};
        layoutDesc[0].SemanticName = "POSITION";
        layoutDesc[0].SemanticIndex = 0;
        layoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layoutDesc[0].InputSlot = 0;
        layoutDesc[0].AlignedByteOffset = 0;
        layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[0].InstanceDataStepRate = 0;

        layoutDesc[1].SemanticName = "COLOR";
        layoutDesc[1].SemanticIndex = 0;
        layoutDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        layoutDesc[1].InputSlot = 0;
        layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        layoutDesc[1].InstanceDataStepRate = 0;

        result = DEVICE.GetDevice()->CreateInputLayout(layoutDesc, 2, bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &m_IL);
        RETURN_FALSE_IF(result);

        D3D11_BUFFER_DESC constantDesc;
        ZeroMemory(&constantDesc, sizeof(constantDesc));
        constantDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantDesc.ByteWidth = sizeof(ConstantBuffer);
        constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantDesc.MiscFlags = 0;
        constantDesc.StructureByteStride = 0;

        result = DEVICE.GetDevice()->CreateBuffer(&constantDesc, nullptr, &m_ConstantBuffer);
        RETURN_FALSE_IF(result);

        return true;
    }

    std::string GetCompileTarget() const override { return "vs_5_0"; }

private:
    ID3D11InputLayout* m_IL{ nullptr };
    ID3D11Buffer* m_ConstantBuffer{ nullptr };
};

class PixelShader : public Shader<ID3D11PixelShader>
{
    using Parent_t = Shader<ID3D11PixelShader>;

public:
    void Bind()
    {
        DEVICE.GetContext()->PSSetShader(m_Shader, nullptr, 0);
    }

private:
    bool CreateShaderObject(ID3D10Blob* bytecode)
    {
        HRESULT result = DEVICE.GetDevice()->CreatePixelShader(bytecode->GetBufferPointer()
            , bytecode->GetBufferSize()
            , nullptr
            , &m_Shader);

        return SUCCEEDED(result);
    }

    std::string GetCompileTarget() const override { return "ps_5_0"; }
};

class Mesh
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

public:
    ~Mesh()
    {
        COM_RELEASE(m_VB);
        COM_RELEASE(m_IB);
    }

    bool Create(const std::vector<Vertex>& vertices, const std::vector<unsigned long>& indices)
    {
        if (CreateBuffer(sizeof(Vertex) * static_cast<UINT>(vertices.size()), vertices.data(), &m_VB, D3D11_BIND_VERTEX_BUFFER))
        {
            return CreateBuffer(sizeof(unsigned long) * static_cast<UINT>(indices.size()), indices.data(), &m_IB, D3D11_BIND_INDEX_BUFFER);
        }

        return false;
    }

    bool Create(const std::string& path)
    {
        //#TODO:
        return false;
    }

    void Bind()
    {
        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        DEVICE.GetContext()->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
        DEVICE.GetContext()->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, 0);
        DEVICE.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

private:
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
};

class Camera
{
public:
    void Init(int w, int h)
    {
        m_ViewMTX = XMMatrixIdentity();
        m_ProjectionMTX = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), static_cast<float>(w) / static_cast<float>(h), 0.1f, 1000.0f);
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

        if (m_VS.Create(L"vs_plain_color.hlsl", "ColorVertexShader") && m_PS.Create(L"ps_plain_color.hlsl", "ColorPixelShader"))
        {
            std::vector<Mesh::Vertex> vertices
            {
                Mesh::Vertex{XMFLOAT3{0.0f, 1.0f, 0.0f}, XMFLOAT4{1.0f, 0.0f, 0.0f, 1.0f}},
                Mesh::Vertex{XMFLOAT3{1.0f, -1.0f, 0.0f}, XMFLOAT4{0.0f, 1.0f, 0.0f, 1.0f}},
                Mesh::Vertex{XMFLOAT3{-1.0f, -1.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 1.0f, 1.0f}}
            };

            std::vector<unsigned long> indices
            {
                0, 1, 2
            };

            return m_Triangle.Create(vertices, indices);
        }

        return false;
    }

    void Update(float delta)
    {
        m_Triangle.Bind();

        VertexShader::ConstantBuffer shaderConstant;
        shaderConstant.world = XMMatrixIdentity();
        shaderConstant.view = m_Camera.GetViewMTX();
        shaderConstant.projection = m_Camera.GetProjectionMTX();

        m_VS.SetConstantBuffer(shaderConstant);
        m_VS.Bind();
        m_PS.Bind();

        DEVICE.GetContext()->DrawIndexed(3, 0, 0);
    }

private:
    Camera m_Camera;

    VertexShader m_VS;
    PixelShader m_PS;

    Mesh m_Triangle;
};

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        return -1;
    }

    const int WindowWidth = 1920;
    const int WindowHeight = 1080;

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
        bool isRunning = true;
        while (isRunning)
        {
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
            scene.Update(0.0f);
            DEVICE.Present();
        }
    }

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return 0;
}