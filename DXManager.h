#pragma once

#include "Globals.h"

#include "Vertex.h"
#include "ConstantBuffers.h"
#include "Material.h"

#include "OBJLoader.h"

class DXManager {

public:
	DXManager();
	~DXManager();

	bool InitDirect3D(const std::string& objFilename);
	void Render();
	bool LoadTextures(const std::unordered_map<std::string, Material>& materials);
	bool CreateTextureArrayAndSRV(const std::vector<ID3D11Texture2D*>& textures, ID3D11Texture2D** textureArray, ID3D11ShaderResourceView** textureSRV);
	bool LoadAndProcessTexture(const std::string& texturePath, std::vector<ID3D11Texture2D*>& textureVector, UINT targetWidth, UINT targetHeight);

	IDXGISwapChain* swapChain;
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dContext;
	ID3D11RenderTargetView* backBufferTarget;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* constantBuffer;
	//ID3D11ShaderResourceView* textureArraySRV;

	//ID3D11ShaderResourceView* textureView = nullptr;
	std::vector<ID3D11ShaderResourceView*> textureViews;
	std::vector<ID3D11ShaderResourceView*> displacementTextureViews;
	std::vector<ID3D11ShaderResourceView*> ambientTextureViews;
	ID3D11SamplerState* samplerState = nullptr;

	D3D11_VIEWPORT viewport;
	HWND hWnd;
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	const float resW = 1280.f;
	const float resH = 720.f;
	const bool isWindowed = true;

	std::vector<Vertex> objVertices;
	std::vector<UINT> objIndices;
	std::unordered_map<std::string, Material> materials;

	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX cameraPosition;

	ID3D11Texture2D* depthStencilBuffer = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;

	ID3D11Texture2D* diffuseArrayTexture = nullptr;
	ID3D11Texture2D* displacementArrayTexture = nullptr;
	ID3D11Texture2D* ambientArrayTexture = nullptr;

	ID3D11ShaderResourceView* diffuseArraySRV = nullptr;
	ID3D11ShaderResourceView* displacementArraySRV = nullptr;
	ID3D11ShaderResourceView* ambientArraySRV = nullptr;


	OBJLoader OBJLoadInstance;
};