#include "BasicD3D9Renderer.h"

#include "ErrorHandling.h"
#include "SDL_syswm.h"

struct QuadVertex {
	float x, y, z;
	float u, v;
};

#define MAKE_SHARED_TEXTURE(name, width, height, fmt) \
	HANDLE shared_handle_##name = nullptr;            \
	DX("Failed to create " #name " texture.",         \
	   device->CreateTexture(                         \
		   width,                                     \
		   height,                                    \
		   1,                                         \
		   D3DUSAGE_RENDERTARGET,                     \
		   fmt,                                       \
		   D3DPOOL_DEFAULT,                           \
		   &gbuffer.input_##name,                     \
		   &shared_handle_##name                      \
	   ));                                            \
	gbuffer.input_##name##_shared_handle = new HANDLE(shared_handle_##name);

BasicD3D9Renderer::BasicD3D9Renderer(SDL_Window *window)
	: d3d9(nullptr),
	  device(nullptr),
	  gbuffer({.input_normal = nullptr, .input_normal_shared_handle = nullptr}
	  ) {
	DX("Failed to create D3D9Ex.",
	   Direct3DCreate9Ex(D3D_SDK_VERSION, d3d9.GetAddressOf()));

	D3DPRESENT_PARAMETERS params = {};
	ZeroMemory(&params, sizeof(params));
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.EnableAutoDepthStencil = TRUE;
	params.AutoDepthStencilFormat = D3DFMT_D16;
	params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	params.BackBufferWidth = 840;
	params.BackBufferHeight = 640;
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);
		params.hDeviceWindow = wmInfo.info.win.window;
	}

	DX("Failed to make D3D9Ex device.",
	   d3d9->CreateDeviceEx(
		   D3DADAPTER_DEFAULT,
		   D3DDEVTYPE_HAL,
		   params.hDeviceWindow,
		   D3DCREATE_HARDWARE_VERTEXPROCESSING,
		   &params,
		   nullptr,
		   device.GetAddressOf()
	   ));

	// D3DFMT_A16B16G16R16F is DXGI_FORMAT_R16G16B16A16_FLOAT in D3D11.
	// We're forced to use 16-bit precision as D3D9 doesn't support 32-bit float
	// textures.

	MAKE_SHARED_TEXTURE(normal, 840, 640, D3DFMT_A16B16G16R16F)
	MAKE_SHARED_TEXTURE(depth, 840, 640, D3DFMT_A16B16G16R16F)

	// Screen-aligned quad vertex buffer
	QuadVertex vertices[] = {
		{-0.5f, -0.5f, 0.0f, 0.0f, 0.0f},
		{-0.5f, 0.5f, 0.0f, 0.0f, 1.0f},
		{0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
		{0.5f, 0.5f, 0.0f, 1.0f, 1.0f}};

	// Create vertex buffer and populate it with vertex data
	device->CreateVertexBuffer(
		sizeof(vertices),
		0,
		0,
		D3DPOOL_DEFAULT,
		vertexBuffer.GetAddressOf(),
		nullptr
	);

	void *pVertices;
	vertexBuffer->Lock(0, sizeof(vertices), &pVertices, 0);
	memcpy(pVertices, vertices, sizeof(vertices));
	vertexBuffer->Unlock();
}

HANDLE *BasicD3D9Renderer::GetInputNormalSharedHandle() {
	return gbuffer.input_normal_shared_handle;
}

HANDLE *BasicD3D9Renderer::GetInputDepthSharedHandle() {
	return gbuffer.input_depth_shared_handle;
}

void BasicD3D9Renderer::Render() {
	// TODO: Render what we receive, but for now just clear to blue.
	DX("Failed to begin scene", device->BeginScene());
	DX("Failed to clear backbuffer!",
	   device->Clear(
		   0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0
	   ));
	DX("Failed to set normal texture",
	   device->SetTexture(0, gbuffer.input_normal));
	DX("Failed to null vertex shader", device->SetVertexShader(nullptr));
	DX("Failed to set FVF", device->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1));
	DX("Failed to set vertex buffer",
	   device->SetStreamSource(0, vertexBuffer.Get(), 0, sizeof(QuadVertex)));
	DX("Failed to set render state",
	   device->SetRenderState(D3DRS_LIGHTING, FALSE));
	DX("Failed to set render state",
	   device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	DX("Failed to set render state",
	   device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	DX("Failed to set render state",
	   device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	DX("Failed to set texture stage state",
	   device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
	DX("Failed to draw", device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	DX("Failed to end scene", device->EndScene());
	DX("Failed to present",
	   device->Present(nullptr, nullptr, nullptr, nullptr));
}
