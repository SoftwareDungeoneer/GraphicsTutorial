#include "RenderWindow.h"

#include <tchar.h>

#include <array>
#include <map>
#include <random>
#include <vector>

#include "util.h"

LPCTSTR RenderWindow::kWindowClassName{ _T("Render Window") };

#define RGBA_FLT(r, g, b) { r, g, b, 1.0f }
constexpr float kBackgroundColors[][4]{
	RGBA_FLT(0.f, 0.f, 0.f),
	RGBA_FLT(1.f, 0.f, 0.f),
	RGBA_FLT(1.f, 1.f, 0.f),
	RGBA_FLT(0.f, 1.f, 0.f),
	RGBA_FLT(0.f, 1.f, 1.f),
	RGBA_FLT(0.f, 0.f, 1.f),
	RGBA_FLT(1.f, 0.f, 1.f),
	RGBA_FLT(1.f, 1.f, 1.f),
};

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };

	struct Vertex {
		float pos[2];
		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};
	const D3D11_INPUT_ELEMENT_DESC Vertex::desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
}

RenderWindow::RenderWindow()
{}

RenderWindow::~RenderWindow()
{}

HRESULT RenderWindow::Create()
{
	RegisterWindowClass();
	return CreateUIWindow();
}

void RenderWindow::RegisterWindowClass()
{
	WNDCLASSEX wndClass{
		sizeof(WNDCLASSEX),
		0,
		&RenderWindow::WndProc,
		0,
		0,
		GetModuleHandle(NULL),
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		nullptr,
		nullptr,
		kWindowClassName,
		nullptr
	};

	RegisterClassEx(&wndClass);
}

HRESULT RenderWindow::CreateUIWindow()
{
	DWORD window_styles = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT requested{ 0, 0, 800, 600 };
	AdjustWindowRect(&requested, window_styles, FALSE);
	int width = requested.right - requested.left;
	int height = requested.bottom - requested.top;

	hWnd = CreateWindow(
		kWindowClassName,
		kWindowClassName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		(LPVOID)this
	);

	if (!hWnd)
		return MAKE_HRESULT(1, 0x20FF, 1);

	return S_OK;
}

LRESULT CALLBACK RenderWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RenderWindow* Wnd{ nullptr };
	Wnd = static_cast<RenderWindow*>(GetProp(hWnd, kClassPointerProp));

	if (!Wnd && WM_CREATE == msg)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		Wnd = static_cast<RenderWindow*>(lpcs->lpCreateParams);
		SetProp(hWnd, kClassPointerProp, static_cast<HANDLE>(Wnd));
		Wnd->hWnd = hWnd;
	}

	if (Wnd)
	{
		switch (msg)
		{
		case WM_CREATE:
			return Wnd->OnCreate();

		case WM_SIZE:
			return Wnd->OnSize();

		case WM_DESTROY:
			return Wnd->OnDestroy();
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT RenderWindow::OnCreate()
{
	CreateD3DDevice();

	HRESULT hr{ S_OK };
	ComPtr<IDXGIDevice1> DXGIDevice;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory1> factory;
	ComPtr<IDXGIOutput> primaryOutput;
	std::map<DXGI_FORMAT, std::vector<DXGI_MODE_DESC>> outputFormats;

	pDevice->QueryInterface(&*DXGIDevice);
	DXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void**)&*adapter);
	if (adapter)
	{
		adapter->GetParent(__uuidof(IDXGIFactory1), (void**)&*factory);
		adapter->EnumOutputs(0, &*primaryOutput);
		if (primaryOutput)
		{
			for (int format{ 0 }; format < DXGI_FORMAT_V408; ++format)
			{
				UINT numModes{ 0 };
				DXGI_FORMAT fmt = static_cast<DXGI_FORMAT>(format);
				primaryOutput->GetDisplayModeList(fmt, 0, &numModes, NULL);
				if (!numModes) continue;
				auto er = outputFormats.emplace(fmt, numModes);
				primaryOutput->GetDisplayModeList(fmt, 0, &numModes, er.first->second.data());
			}
		}

		if (factory)
		{
			DXGI_SWAP_CHAIN_DESC swap_chain_desc;
			ZeroInitialize(swap_chain_desc);
			swap_chain_desc.BufferDesc.RefreshRate = DXGI_RATIONAL{ 60, 1 };
			swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swap_chain_desc.SampleDesc.Count = 1;
			swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swap_chain_desc.BufferCount = 2;
			swap_chain_desc.OutputWindow = hWnd;
			swap_chain_desc.Windowed = TRUE;
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			hr = factory->CreateSwapChain(*pDevice, &swap_chain_desc, &*pSwapChain);
		}
	}
	
	if (FAILED(hr))
		return -1;
	
	InitializeRendering();
	InitializeTriangles();

	enableUpdate = true;
	return 0;
}

LRESULT RenderWindow::OnDestroy()
{
	hWnd = nullptr;
	PostQuitMessage(0);
	return 0;
}

LRESULT RenderWindow::OnSize()
{
	pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	return 0;
}

void RenderWindow::CreateD3DDevice()
{
	std::vector<D3D_FEATURE_LEVEL> featureLevels{
		D3D_FEATURE_LEVEL_11_1,
		// D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL actualFeatureLevel;

	D3D_DRIVER_TYPE driverType{ D3D_DRIVER_TYPE_HARDWARE };
	auto deviceFlags{ D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG /* | D3D11_CREATE_DEVICE_DEBUGGABLE */ };

	HRESULT hr = D3D11CreateDevice(
		NULL,
		driverType,
		NULL,
		deviceFlags,
		featureLevels.data(),
		static_cast<UINT>(featureLevels.size()),
		D3D11_SDK_VERSION,
		&*pDevice,
		&actualFeatureLevel,
		&*pDeviceContext
	);
}

D3D11_VIEWPORT RenderWindow::ViewportFromTexture(ComPtr<ID3D11Texture2D>& tex)
{
	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	UINT nViewports{ 1 };
	D3D11_VIEWPORT viewport{
		0, 0,
		desc.Width * 1.f,
		desc.Height * 1.f,
		0, 1
	};
	return viewport;
}

void RenderWindow::InitializeRendering()
{
	auto vsBytes = LoadFile(_T("ClientToNdc.cso"));
	auto psBytes = LoadFile(_T("SolidFill.cso"));

	pDevice->CreateVertexShader(vsBytes.data(), vsBytes.size(), nullptr, &*vertexShader);
	pDevice->CreatePixelShader(psBytes.data(), psBytes.size(), nullptr, &*pixelShader);

	constexpr unsigned short indices[] = { 0, 2, 1 };
	constexpr D3D11_BUFFER_DESC indexBufferDesc{
		sizeof(indices),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0, // No CPU access
		0, // Mo misc flags
		0, // Not a structured buffer
	};
	const D3D11_SUBRESOURCE_DATA indexBufferSrd{ &indices, 0, 0 };
	pDevice->CreateBuffer(&indexBufferDesc, &indexBufferSrd, &*indexBuffer);

	const Vertex verts[] = {
		{ -8, 0 },
		{  8, 0 },
		{  0, 12 }
	};
	pDevice->CreateInputLayout(Vertex::desc, countof(Vertex::desc), vsBytes.data(), vsBytes.size(), &*inputLayout);

	constexpr D3D11_BUFFER_DESC vertexBufferDesc{
		sizeof(verts),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, // No CPU access
		0, // No Misc flags
		0, // Not a structured buffer
	};
	const D3D11_SUBRESOURCE_DATA vertexBufferSrd{ &verts, 0, 0 };
	pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferSrd, &*vertexBuffer);

	D3D11_BUFFER_DESC cbufferDesc{
		sizeof(vscbViewport),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_CONSTANT_BUFFER,
		0,
		0, // No Misc flags
		0, // Not a structured buffer
	};
	pDevice->CreateBuffer(&cbufferDesc, nullptr, &*viewportCBuffer);

	cbufferDesc.ByteWidth = sizeof(vscbInstanceBuffer);
	pDevice->CreateBuffer(&cbufferDesc, nullptr, &*instanceCBuffer);
}

void RenderWindow::InitializeTriangles()
{
	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_real_distribution<float> dist;

	ZeroInitialize(triangles);
	for (auto& tri : triangles)
	{
		tri.position.x = dist(engine) * 800;
		tri.position.y = dist(engine) * 600;
		tri.veloicty.x = 60 * (2 * dist(engine) - 1);
		tri.veloicty.y = 60 * (2 * dist(engine) - 1);
		tri.color[0] = dist(engine);
		tri.color[1] = dist(engine);
		tri.color[2] = dist(engine);
		tri.color[3] = 1.f;
	}
}

void RenderWindow::Update(double elapsed)
{
	if (!enableUpdate)
		return;

	for (auto& tri : triangles)
	{
		tri.position += tri.veloicty * float(elapsed);
		if (tri.position.x < 0.f)
			tri.position.x += 800.f;
		if (tri.position.x > 800.f)
			tri.position.x -= 800.f;
		if (tri.position.y > 600.f)
			tri.position.y -= 600.f;
		if (tri.position.y < 0.f)
			tri.position.y += 600.f;
	}
}

void RenderWindow::Render()
{
	static unsigned bgIndex{ 0 };

	if (!enableUpdate)
		return; 
	
	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, kBackgroundColors[0]);

	// Get back buffer description and build viewport
	D3D11_VIEWPORT viewport{ ViewportFromTexture(pBackBuffer) };

	// Update vertex shader constants
	vscbViewport.viewportWidth = viewport.Width;
	vscbViewport.viewportHeight = viewport.Height;
	pDeviceContext->UpdateSubresource(*viewportCBuffer, 0, nullptr, &vscbViewport, 0, 0);

	// Set up input assembler
	unsigned stride = sizeof(Vertex);
	unsigned offset = 0;
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetIndexBuffer(*indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pDeviceContext->IASetInputLayout(*inputLayout);
	pDeviceContext->IASetVertexBuffers(0, 1, &*vertexBuffer, &stride, &offset);

	// Set up vertex shadr
	std::array<ID3D11Buffer*, 2> vsConstantBuffers = {
		*instanceCBuffer,
		*viewportCBuffer,
	};
	pDeviceContext->VSSetShader(*vertexShader, nullptr, 0);
	pDeviceContext->VSSetConstantBuffers(0, 2, vsConstantBuffers.data());

	// Set up rasterizer
	pDeviceContext->RSSetViewports(1, &viewport);

	// Set up Output Merger
	pDeviceContext->OMSetRenderTargets(1, &*pRenderTarget, nullptr);

	// Set up pixel shader
	pDeviceContext->PSSetShader(*pixelShader, nullptr, 0);

	// Dispatch draw
	for (const auto& tri : triangles)
	{
		pDeviceContext->UpdateSubresource(*instanceCBuffer, 0, nullptr, &tri, 0, 0);
		pDeviceContext->DrawIndexed(3, 0, 0);
	}

	pSwapChain->Present(1, 0);
}
