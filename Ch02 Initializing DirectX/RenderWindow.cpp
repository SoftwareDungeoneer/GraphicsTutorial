#include "RenderWindow.h"

#include <tchar.h>

#include <map>
#include <vector>

#include "util.h"

LPCTSTR RenderWindow::kWindowClassName{ _T("Render Window") };

namespace
{
	LPCTSTR kClassPointerProp{ _T("Class Pointer") };
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
	
	enableUpdate = true;
	return 0;
}

LRESULT RenderWindow::OnDestroy()
{
	hWnd = nullptr;
	PostQuitMessage(0);
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

void RenderWindow::Update(double elapsed)
{
	if (!enableUpdate)
		return;

	elapsedInterval += elapsed;
	if (elapsedInterval < 0.2)
		return;

	elapsedInterval = 0.0;

#define RGBA_FLT(r, g, b) { r, g, b, 1.0f }

	static unsigned bgIndex{ 0 };
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

	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11RenderTargetView> pRenderTarget;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&*pBackBuffer);
	pDevice->CreateRenderTargetView(*pBackBuffer, nullptr, &*pRenderTarget);
	pDeviceContext->ClearRenderTargetView(*pRenderTarget, kBackgroundColors[bgIndex++]);
	bgIndex %= countof(kBackgroundColors);
	pSwapChain->Present(1, 0);
}
