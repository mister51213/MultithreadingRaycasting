#pragma once

namespace Shaders {
	#include "pixel.shh"
	#include "vertex.shh"
}


struct FSQVertex {
	float x, y, z;
	float u, v;
};


class DXHelper {
protected:
	IDXGIFactory			 *pFactory		= nullptr;
	IDXGIAdapter			 *pAdapter		= nullptr;
	IDXGIOutput				 *pModeEnum		= nullptr;
	DXGI_MODE_DESC			 *pModes		= nullptr;

	IDXGISwapChain			 *pSwapChain	= nullptr;
	ID3D11Device			 *pD3Dev		= nullptr;
	ID3D11DeviceContext		 *pDevCtx		= nullptr;
	ID3D11RenderTargetView	 *pRTView		= nullptr;
	ID3D11ShaderResourceView *pSysView		= nullptr;
	ID3D11Texture2D			 *pSysBuff		= nullptr;
	ID3D11PixelShader		 *pShaderPS		= nullptr;
	ID3D11VertexShader		 *pShaderVS		= nullptr;
	ID3D11InputLayout		 *pLayout		= nullptr;
	ID3D11Buffer			 *pVertBuff		= nullptr;
	ID3D11SamplerState		 *pSmpState		= nullptr;
	ID3D11SamplerState		 *pWallSmpState	= nullptr;

	vector<ID3D11ShaderResourceView*> TexViews;

	unsigned ScrWid = 0;
	unsigned ScrHei = 0;

	bool Failed() {
		Release();
		return true;
	}

public:
	void Release() {
		if (pModes)
			delete[] pModes;
		
		if (pModeEnum)
			pModeEnum->Release();

		if (pAdapter)
			pAdapter->Release();

		if (pFactory)
			pFactory->Release();

		// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
		if (pSwapChain)
			pSwapChain->SetFullscreenState(false, NULL);

		if (pRTView)
			pRTView->Release();

		if (pDevCtx)
			pDevCtx->Release();

		if (pD3Dev)
			pD3Dev->Release();

		if (pSwapChain)
			pSwapChain->Release();

		new (this) DXHelper;
	}

	bool Init(const HWND hWnd, const unsigned ScreenWidth, const unsigned ScreenHeight) {
		Release();

		HRESULT result;
		result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
		if (FAILED(result))
			return Failed();

		result = pFactory->EnumAdapters(0, &pAdapter);
		if (FAILED(result))
			return Failed();

		result = pAdapter->EnumOutputs(0, &pModeEnum);
		if (FAILED(result))
			return Failed();

		unsigned numModes;
		result = pModeEnum->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		if (FAILED(result))
			return Failed();

		pModes = new DXGI_MODE_DESC[numModes];
		if (!pModes)
			return Failed();

		result = pModeEnum->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, pModes);
		if (FAILED(result))
			return Failed();

		unsigned numerator = 0, denominator = 1;
		for (unsigned i = 0; i < numModes; i++)
			if (pModes[i].Width == ScreenWidth && pModes[i].Height == ScreenHeight) {
				numerator = pModes[i].RefreshRate.Numerator;
				denominator = pModes[i].RefreshRate.Denominator;
			}

		delete[] pModes;
		pModes = nullptr;

		pModeEnum->Release();
		pModeEnum = nullptr;

		pAdapter->Release();
		pAdapter = nullptr;

		pFactory->Release();
		pFactory = nullptr;

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		ZeroMemory(&swapDesc, sizeof(swapDesc));

		swapDesc.Windowed			= true;
		swapDesc.OutputWindow		= hWnd;
		swapDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;
		swapDesc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.BufferCount		= 1;
		swapDesc.Flags				= 0;

		swapDesc.BufferDesc.Width	= ScreenWidth;
		swapDesc.BufferDesc.Height	= ScreenHeight;
		swapDesc.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;

		swapDesc.SampleDesc.Count	= 1;
		swapDesc.SampleDesc.Quality	= 0;

		swapDesc.BufferDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferDesc.RefreshRate.Numerator   = numerator;
		swapDesc.BufferDesc.RefreshRate.Denominator = denominator;

		D3D_FEATURE_LEVEL feat = D3D_FEATURE_LEVEL_11_0;
		result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &feat, 1, 
											   D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pD3Dev, nullptr, &pDevCtx);
		if (FAILED(result))
			return Failed();

		ID3D11Texture2D *texBackBuff;
		result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texBackBuff);
		if (FAILED(result))
			return Failed();

		result = pD3Dev->CreateRenderTargetView(texBackBuff, nullptr, &pRTView);
		if (FAILED(result))
			return Failed();

		pDevCtx->OMSetRenderTargets(1, &pRTView, nullptr);
		texBackBuff->Release();

		D3D11_VIEWPORT vp;
		vp.Width	= (float)ScreenWidth;
		vp.Height	= (float)ScreenHeight;
		vp.MinDepth	= 0.0f;
		vp.MaxDepth	= 1.0f;
		vp.TopLeftX	= 0.0f;
		vp.TopLeftY	= 0.0f;

		pDevCtx->RSSetViewports(1, &vp);

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width				= ScreenWidth;
		texDesc.Height				= ScreenHeight;
		//texDesc.Width				= ScreenHeight;
		//texDesc.Height				= ScreenWidth;
		texDesc.MipLevels			= 1;
		texDesc.ArraySize			= 1;
		texDesc.Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;// DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.Usage				= D3D11_USAGE_DYNAMIC;
		texDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		texDesc.MiscFlags			= 0;
		texDesc.SampleDesc.Count	= 1;
		texDesc.SampleDesc.Quality	= 0;

		result = pD3Dev->CreateTexture2D(&texDesc, nullptr, &pSysBuff);
		if (FAILED(result))
			return Failed();

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format				= texDesc.Format;
		srvDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels	= 1;

		result = pD3Dev->CreateShaderResourceView(pSysBuff, &srvDesc, &pSysView);
		if (FAILED(result))
			return Failed();

		result = pD3Dev->CreatePixelShader(Shaders::g_FramebufferPS, sizeof(Shaders::g_FramebufferPS), nullptr, &pShaderPS);
		if (FAILED(result))
			return Failed();

		result = pD3Dev->CreateVertexShader(Shaders::g_FramebufferVS, sizeof(Shaders::g_FramebufferVS), nullptr, &pShaderVS);
		if (FAILED(result))
			return Failed();

		const FSQVertex vertices[] = {
			//   X      Y     Z     U     V
			{-1.0f,  1.0f, 0.0f, 0.0f, 0.0f},
			{ 1.0f,  1.0f, 0.0f, 1.0f, 0.0f},
			{ 1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
			{-1.0f,  1.0f, 0.0f, 0.0f, 0.0f},
			{ 1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
			{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
		};

		D3D11_BUFFER_DESC vertDesc = {};
		vertDesc.Usage			= D3D11_USAGE_DEFAULT;
		vertDesc.ByteWidth		= sizeof(FSQVertex) * 6;
		vertDesc.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		vertDesc.CPUAccessFlags	= 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem		= vertices;

		result = pD3Dev->CreateBuffer(&vertDesc, &initData, &pVertBuff);
		if (FAILED(result))
			return Failed();

		const D3D11_INPUT_ELEMENT_DESC ieDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		result = pD3Dev->CreateInputLayout(ieDesc, 2, Shaders::g_FramebufferVS, sizeof(Shaders::g_FramebufferVS), &pLayout);
		if (FAILED(result))
			return Failed();

		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampDesc.AddressU		= D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV		= D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW		= D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD			= 0;
		sampDesc.MaxLOD			= D3D11_FLOAT32_MAX;

		result = pD3Dev->CreateSamplerState(&sampDesc, &pSmpState);
		if (FAILED(result))
			return Failed();

		//D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD			= 0;
		sampDesc.MaxLOD			= D3D11_FLOAT32_MAX;

		result = pD3Dev->CreateSamplerState(&sampDesc, &pWallSmpState);
		if (FAILED(result))
			return Failed();

		ScrWid = ScreenWidth;
		ScrHei = ScreenHeight;

		return false;
	}

	bool NewTexture(Pixel *pImage, int Wid, int Hei) {
		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = pImage;
		subData.SysMemPitch = Wid * sizeof(Pixel);
		
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width				= Wid;
		texDesc.Height				= Hei;
		texDesc.MipLevels			= 1;
		texDesc.ArraySize			= 1;
		texDesc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.Usage				= D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags		= 0;
		texDesc.MiscFlags			= 0;
		texDesc.SampleDesc.Count	= 1;
		texDesc.SampleDesc.Quality	= 0;

		ID3D11Texture2D *pTex;
		HRESULT result = pD3Dev->CreateTexture2D(&texDesc, &subData, &pTex);
		if (FAILED(result))
			return true;

/*		return pTex;
	}

	void BindTexture(ID3D11Texture2D *pTex) {*/
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels	= -1;

		ID3D11ShaderResourceView *pView;
		result = pD3Dev->CreateShaderResourceView(pTex, &srvDesc, &pView);
		if (FAILED(result))
			return true;

		TexViews.push_back(pView);

		return false;
	}

	bool Render(Camera *pCam, const bool VSync) {
		HRESULT result;

		// Clear the back buffer.
		//float color[4] = {1,0,1, 1};
		//pDevCtx->ClearRenderTargetView(pRTView, color);

		D3D11_MAPPED_SUBRESOURCE mapTex;
		result = pDevCtx->Map(pSysBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapTex);
		if (FAILED(result))
			return true;

		char *pDst = (char*)(mapTex.pData);
		const size_t dstPitch = mapTex.RowPitch;
		//const size_t srcPitch = ScrWid * sizeof(Pixel);
		//const size_t srcPitch = ScrHei * sizeof(Pixel);
		const size_t srcPitch = ScrWid * sizeof(float) * 4;

		for (unsigned y = 0; y < ScrHei; y++)
		//for (unsigned y = 0; y < ScrWid; y++)
			//memcpy(pDst + y * dstPitch, (char*)pCam->pImage + y * srcPitch, srcPitch);
			memcpy(pDst + y * dstPitch, (char*)pCam->pFLOATS + y * srcPitch, srcPitch);

		pDevCtx->Unmap(pSysBuff, 0);

		const unsigned stride = sizeof(FSQVertex), offset = 0;
		pDevCtx->IASetInputLayout(pLayout);
		pDevCtx->VSSetShader(pShaderVS, nullptr, 0);
		pDevCtx->PSSetShader(pShaderPS,  nullptr, 0);
		pDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDevCtx->IASetVertexBuffers(0, 1, &pVertBuff, &stride, &offset);
		pDevCtx->PSSetShaderResources(0, 1, &pSysView);

		for (int i = 0; i < TexViews.size(); i++)
			pDevCtx->PSSetShaderResources(i + 1, 1, &TexViews[i]);

		pDevCtx->PSSetSamplers(0, 1, &pSmpState);
		pDevCtx->PSSetSamplers(1, 1, &pWallSmpState);
		pDevCtx->Draw(6, 0);

		pSwapChain->Present(VSync ? 1 : 0, 0);

		return false;
	}
};


static DXHelper DX;