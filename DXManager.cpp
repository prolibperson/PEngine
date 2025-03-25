#include "DXManager.h"

DXManager::DXManager() : vertexBuffer(nullptr), indexBuffer(nullptr), constantBuffer(nullptr),
vertexShader(nullptr), pixelShader(nullptr), inputLayout(nullptr), depthStencilBuffer(nullptr),
depthStencilView(nullptr), backBufferTarget(nullptr), swapChain(nullptr), d3dContext(nullptr),
d3dDevice(nullptr) {
};
DXManager::~DXManager() {
    if (vertexBuffer) vertexBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
    if (constantBuffer) constantBuffer->Release();
    if (vertexShader) vertexShader->Release();
    if (pixelShader) pixelShader->Release();
    if (inputLayout) inputLayout->Release();
    if (depthStencilBuffer) depthStencilBuffer->Release();
    if (depthStencilView) depthStencilView->Release();
    if (backBufferTarget) backBufferTarget->Release();
    if (swapChain) swapChain->Release();
    if (d3dContext) d3dContext->Release();
    if (d3dDevice) d3dDevice->Release();
};

bool DXManager::InitDirect3D(const std::string& objFilename) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = (UINT)resW;
    swapChainDesc.BufferDesc.Height = (UINT)resH;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 4;
    swapChainDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
    swapChainDesc.Windowed = isWindowed;

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.Width = (UINT)resW;
    depthStencilDesc.Height = (UINT)resH;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 4;
    depthStencilDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, 0, &featureLevel, 1,
        D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &d3dDevice, nullptr, &d3dContext);
    if (FAILED(result)) {
        MessageBox(nullptr, "D3D11CreateDeviceAndSwapChain failed!", "Error", MB_OK);
        return false;
    }

    ID3D11Texture2D* backBuffer;
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "GetBuffer failed!", "Error", MB_OK);
        return false;
    }

    result = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &backBufferTarget);
    if (FAILED(result)) {
        MessageBox(nullptr, "CreateRenderTargetView failed!", "Error", MB_OK);
        return false;
    }

    backBuffer->Release();

    result = d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "CreateTexture2D failed!", "Error", MB_OK);
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = depthStencilDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    result = d3dDevice->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
    if (FAILED(result)) {
        MessageBox(nullptr, "CreateDepthStencilView failed!", "Error", MB_OK);
        return false;
    }

    d3dContext->OMSetRenderTargets(1, &backBufferTarget, depthStencilView);

    viewport.Width = static_cast<float>(resW);
    viewport.Height = static_cast<float>(resH);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    d3dContext->RSSetViewports(1, &viewport);

    ID3DBlob* vsBuffer = nullptr;
    result = D3DReadFileToBlob(L"Shaders/VertexShader.cso", &vsBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "Vertex shader loading failed", "Error", MB_OK);
        return false;
    }

    result = d3dDevice->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(result)) {
        vsBuffer->Release();
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "MATERIALINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };


    result = d3dDevice->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &inputLayout);
    vsBuffer->Release();
    if (FAILED(result)) {
        return false;
    }

    ID3DBlob* psBuffer = nullptr;
    result = D3DReadFileToBlob(L"Shaders/PixelShader.cso", &psBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "Pixel shader loading failed", "Error", MB_OK);
        return false;
    }

    result = d3dDevice->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), nullptr, &pixelShader);
    psBuffer->Release();
    if (FAILED(result)) {
        return false;
    }

    d3dContext->IASetInputLayout(inputLayout);

    if (!OBJLoadInstance.LoadMTL("Assets/sponza.mtl", materials)) {
        MessageBox(nullptr, "Failed to load MTL file", "Error", MB_OK);
        return false;
    }

    if (!LoadTextures(materials)) {
        return false;
    }

    if (!OBJLoadInstance.LoadOBJ(objFilename.c_str(), objVertices, objIndices, materials)) {
        MessageBox(nullptr, "Failed to load OBJ file", "Error", MB_OK);
        return false;
    }

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * objVertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    ZeroMemory(&vertexData, sizeof(vertexData));
    vertexData.pSysMem = objVertices.data();

    result = d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "CreateBuffer failed for vertex buffer!", "Error", MB_OK);
        return false;
    }

    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * objIndices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData;
    ZeroMemory(&indexData, sizeof(indexData));
    indexData.pSysMem = objIndices.data();

    result = d3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "CreateBuffer failed for index buffer!", "Error", MB_OK);
        return false;
    }

    D3D11_BUFFER_DESC constantBufferDesc;
    ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.MiscFlags = 0;

    result = d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
    if (FAILED(result)) {
        MessageBox(nullptr, "CreateBuffer failed for constant buffer!", "Error", MB_OK);
        return false;
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = d3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
    if (FAILED(result)) {
        MessageBox(nullptr, "Failed to create sampler state!", "Error", MB_OK);
        return false;
    }

    worldMatrix = XMMatrixIdentity();
    viewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 3.0f, -40.0f, 0.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, resW / resH, 0.5f, 10000.0f);

    return true;
}

void DXManager::Render() {
    float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    d3dContext->ClearRenderTargetView(backBufferTarget, clearColor);
    d3dContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    d3dContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ConstantBuffer cb;
    cb.mWorld = XMMatrixTranspose(worldMatrix);
    cb.mView = XMMatrixTranspose(viewMatrix);
    cb.mProjection = XMMatrixTranspose(projectionMatrix);
    d3dContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

    d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    d3dContext->VSSetShader(vertexShader, nullptr, 0);
    d3dContext->PSSetShader(pixelShader, nullptr, 0);

    d3dContext->PSSetShaderResources(0, 1, &diffuseArraySRV);
    d3dContext->PSSetSamplers(0, 1, &samplerState);

    d3dContext->PSSetShaderResources(1, 1, &displacementArraySRV);
    d3dContext->PSSetSamplers(1, 1, &samplerState);

    d3dContext->PSSetShaderResources(2, 1, &ambientArraySRV);
    d3dContext->PSSetSamplers(2, 1, &samplerState);

    d3dContext->DrawIndexed(static_cast<UINT>(objIndices.size()), 0, 0);

    swapChain->Present(0, 0);
}


bool DXManager::LoadTextures(const std::unordered_map<std::string, Material>& materials) {
    std::vector<ID3D11Texture2D*> diffuseTextures;
    std::vector<ID3D11Texture2D*> displacementTextures;
    std::vector<ID3D11Texture2D*> ambientTextures;

    UINT textureWidth = 0;
    UINT textureHeight = 0;

    for (const auto& entry : materials) {
        const auto& material = entry.second;

        if (!material.diffuseMap.empty()) {
            DirectX::ScratchImage image;
            HRESULT result = DirectX::LoadFromTGAFile(std::wstring(material.diffuseMap.begin(), material.diffuseMap.end()).c_str(), nullptr, image);
            if (FAILED(result)) {
                MessageBox(nullptr, "Failed to load diffuse TGA texture!", "Error", MB_OK);
                return false;
            }
            textureWidth = image.GetMetadata().width;
            textureHeight = image.GetMetadata().height;
            std::cout << "Reference diffuse texture size: " << textureWidth << "x" << textureHeight << std::endl;
            break;
        }
    }

    for (const auto& entry : materials) {
        const auto& material = entry.second;

        if (!material.diffuseMap.empty()) {
            std::cout << "Loading diffuse texture: " << material.diffuseMap << std::endl;
            if (!LoadAndProcessTexture(material.diffuseMap, diffuseTextures, textureWidth, textureHeight)) {
                MessageBox(nullptr, "Failed to load or process diffuse texture!", "Error", MB_OK);
                return false;
            }
        }

        if (!material.displacementMap.empty()) {
            std::cout << "Loading displacement texture: " << material.displacementMap << std::endl;
            if (!LoadAndProcessTexture(material.displacementMap, displacementTextures, textureWidth, textureHeight)) {
                MessageBox(nullptr, "Failed to load or process displacement texture!", "Error", MB_OK);
                return false;
            }
        }

        if (!material.ambientMap.empty()) {
            std::cout << "Loading ambient texture: " << material.ambientMap << std::endl;
            if (!LoadAndProcessTexture(material.ambientMap, ambientTextures, textureWidth, textureHeight)) {
                MessageBox(nullptr, "Failed to load or process ambient texture!", "Error", MB_OK);
                return false;
            }
        }
    }

    if (!CreateTextureArrayAndSRV(diffuseTextures, &diffuseArrayTexture, &diffuseArraySRV) ||
        !CreateTextureArrayAndSRV(displacementTextures, &displacementArrayTexture, &displacementArraySRV) ||
        !CreateTextureArrayAndSRV(ambientTextures, &ambientArrayTexture, &ambientArraySRV)) {
        return false;
    }

    return true;
}

bool DXManager::LoadAndProcessTexture(const std::string& texturePath, std::vector<ID3D11Texture2D*>& textureVector, UINT targetWidth, UINT targetHeight) {
    DirectX::ScratchImage image;
    HRESULT result = DirectX::LoadFromTGAFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(), nullptr, image);
    if (FAILED(result)) {
        std::cerr << "Failed to load TGA texture: " << texturePath << std::endl;
        return false;
    }

    DirectX::ScratchImage resizedImage;
    const DirectX::TexMetadata& meta = image.GetMetadata();
    if (meta.width != targetWidth || meta.height != targetHeight) {
        std::cout << "Resizing texture: " << texturePath << " from " << meta.width << "x" << meta.height << " to " << targetWidth << "x" << targetHeight << std::endl;
        result = DirectX::Resize(image.GetImages(), image.GetImageCount(), meta, targetWidth, targetHeight, DirectX::TEX_FILTER_DEFAULT, resizedImage);
        if (FAILED(result)) {
            std::cerr << "Failed to resize texture: " << texturePath << std::endl;
            return false;
        }
    }
    else {
        resizedImage = std::move(image);
    }

    ID3D11Texture2D* texture;
    result = DirectX::CreateTexture(d3dDevice, resizedImage.GetImages(), resizedImage.GetImageCount(), resizedImage.GetMetadata(), (ID3D11Resource**)&texture);
    if (FAILED(result)) {
        std::cerr << "Failed to create texture from image: " << texturePath << std::endl;
        return false;
    }

    textureVector.push_back(texture);
    return true;
}

bool DXManager::CreateTextureArrayAndSRV(const std::vector<ID3D11Texture2D*>& textures, ID3D11Texture2D** textureArray, ID3D11ShaderResourceView** textureSRV) {
    if (textures.empty()) {
        return true;
    }

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textures[0]->GetDesc(&textureDesc);
    textureDesc.ArraySize = static_cast<UINT>(textures.size());

    HRESULT result = d3dDevice->CreateTexture2D(&textureDesc, nullptr, textureArray);
    if (FAILED(result)) {
        MessageBox(nullptr, "Failed to create texture array!", "Error", MB_OK);
        return false;
    }

    for (size_t i = 0; i < textures.size(); ++i) {
        d3dContext->CopySubresourceRegion(*textureArray, D3D11CalcSubresource(0, static_cast<UINT>(i), textureDesc.MipLevels), 0, 0, 0, textures[i], 0, nullptr);
        textures[i]->Release();
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = static_cast<UINT>(textures.size());

    result = d3dDevice->CreateShaderResourceView(*textureArray, &srvDesc, textureSRV);
    if (FAILED(result)) {
        MessageBox(nullptr, "Failed to create shader resource view for texture array!", "Error", MB_OK);
        return false;
    }

    return true;
}