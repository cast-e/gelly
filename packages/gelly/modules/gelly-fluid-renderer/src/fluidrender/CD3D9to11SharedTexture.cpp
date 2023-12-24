#include <stdexcept>
#include "fluidrender/CD3D9to11SharedTexture.h"
#include "detail/d3d11/ErrorHandling.h"
#include "fluidrender/IRenderContext.h"

static TextureFormat validFormats[] = {
	TextureFormat::R8G8B8A8_UNORM,
	TextureFormat::R16G16B16A16_FLOAT,
	TextureFormat::R10G10B10A2_UNORM
};

CD3D9to11SharedTexture::CD3D9to11SharedTexture(HANDLE sharedHandle)
	: context(nullptr),
	  texture(nullptr),
	  srv(nullptr),
	  rtv(nullptr),
	  uav(nullptr),
	  sharedHandle(sharedHandle) {}

CD3D9to11SharedTexture::~CD3D9to11SharedTexture() {
	// Unfortunate, but necessary
	// There is UB if a virtual function is called in a destructor
	CD3D9to11SharedTexture::Destroy();
}

void CD3D9to11SharedTexture::AutogenerateDesc() {
	if (texture == nullptr) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::AutogenerateDesc() should not be called "
			"before the texture is created"
		);
	}

	// Pull our texture description from the texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texture->GetDesc(&texDesc);

	autoGeneratedDesc.width = texDesc.Width;
	autoGeneratedDesc.height = texDesc.Height;

	const auto format = GetTextureFormatFromDXGI(texDesc.Format);
	if (std::find(
		std::begin(validFormats), std::end(validFormats), format
	) == std::end(validFormats)) {
		throw std::runtime_error(
			"CD3D9to11SharedTexture::AutogenerateDesc() encountered an "
			"unsupported texture format"
		);
	}

	autoGeneratedDesc.format = format;
	autoGeneratedDesc.access = TextureAccess::READ | TextureAccess::WRITE;
	autoGeneratedDesc.isFullscreen = false;
	autoGeneratedDesc.filter = TextureFilter::POINT;
	autoGeneratedDesc.addressMode = TextureAddressMode::WRAP;
}

void CD3D9to11SharedTexture::SetDesc(const TextureDesc &desc) {
	// There is no control over the texture description.
	throw std::logic_error(
		"CD3D9to11SharedTexture::SetDesc() should not be called"
	);
}

const TextureDesc &CD3D9to11SharedTexture::GetDesc() const {
	return autoGeneratedDesc;
}

bool CD3D9to11SharedTexture::Create() {
	if (!context) {
		return false;
	}

	if (texture) {
		return false;
	}

	auto *device = static_cast<ID3D11Device *>(
		context->GetRenderAPIResource(RenderAPIResource::D3D11Device)
	);

	ID3D11Resource *resource = nullptr;
	HRESULT hr = device->OpenSharedResource(
		sharedHandle, __uuidof(ID3D11Resource), (void **)&resource
	);

	if (FAILED(hr)) {
		return false;
	}

	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&texture);

	if (FAILED(hr)) {
		return false;
	}

	resource->Release();
	AutogenerateDesc();

	const auto format = GetDXGIFormat(autoGeneratedDesc.format);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	DX("Failed to create D3D11 RTV",
	   device->CreateRenderTargetView(texture, &rtvDesc, &rtv));

	if ((autoGeneratedDesc.access & TextureAccess::READ) != 0) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		DX("Failed to create D3D11 SRV",
		   device->CreateShaderResourceView(texture, &srvDesc, &srv));
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = TextureFilterToD3D11(autoGeneratedDesc.filter);
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW =
		TextureAddressModeToD3D11(autoGeneratedDesc.addressMode);

	DX("Failed to create D3D11 sampler",
	   device->CreateSamplerState(&samplerDesc, &sampler));

	DX("Failed to query ID3D11Resource from texture",
	   texture->QueryInterface(
		   __uuidof(ID3D11Resource), reinterpret_cast<void **>(&d3d11Resource)
	   ));

	return true;
}

void CD3D9to11SharedTexture::Destroy() {
	/**
	 * The texture we own is like a proxy to the shared texture. We still need
	 * to release it, but we don't own the shared texture.
	 */
	if (texture) {
		texture->Release();
		texture = nullptr;
	}

	if (srv) {
		srv->Release();
		srv = nullptr;
	}

	if (rtv) {
		rtv->Release();
		rtv = nullptr;
	}

	if (uav) {
		uav->Release();
		uav = nullptr;
	}
}

void CD3D9to11SharedTexture::AttachToContext(IRenderContext *context) {
	if (context->GetRenderAPI() != ContextRenderAPI::D3D11) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::AttachToContext() encountered an "
			"unsupported render API"
		);
	}

	this->context = context;
}

GellyObserverPtr<IRenderContext> CD3D9to11SharedTexture::GetParentContext() {
	return context;
}

void CD3D9to11SharedTexture::SetFullscreenSize() {
	throw std::logic_error(
		"CD3D9to11SharedTexture::SetFullscreenSize() should not be called"
	);
}

void *CD3D9to11SharedTexture::GetSharedHandle() {
	throw std::logic_error(
		"CD3D9to11SharedTexture::GetSharedHandle() should not be called"
	);
}

void *CD3D9to11SharedTexture::GetResource(TextureResource resource) {
	switch (resource) {
		case TextureResource::D3D11_SRV:
			return srv;
		case TextureResource::D3D11_RTV:
			return rtv;
		case TextureResource::D3D11_UAV:
			return uav;
		case TextureResource::D3D11_RESOURCE:
			return d3d11Resource;
		default:
			return nullptr;
	}
}

void CD3D9to11SharedTexture::BindToPipeline(
	TextureBindStage stage, uint8_t slot, OptionalDepthBuffer depthBuffer
) {
	if (!context) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::BindToPipeline() encountered a null "
			"context"
		);
	}

	auto *deviceContext = static_cast<ID3D11DeviceContext *>(
		context->GetRenderAPIResource(RenderAPIResource::D3D11DeviceContext)
	);

	switch (stage) {
		case TextureBindStage::PIXEL_SHADER_READ:
			deviceContext->PSSetShaderResources(slot, 1, &srv);
			deviceContext->PSSetSamplers(slot, 1, &sampler);

			break;
		case TextureBindStage::RENDER_TARGET_OUTPUT: {
			if (rtv == nullptr) {
				throw std::logic_error(
					"CD3D9to11SharedTexture::BindToPipeline: RTV is null."
				);
			}

			ID3D11DepthStencilView *dsv = nullptr;

			if (depthBuffer.has_value()) {
				dsv = static_cast<ID3D11DepthStencilView *>(
					depthBuffer.value()->RequestResource(
						DepthBufferResource::D3D11_DSV
					)
				);

				if (dsv == nullptr) {
					throw std::logic_error(
						"CD3D9to11SharedTexture::BindToPipeline: Failed to "
						"request a D3D11 DSV from the provided depth buffer."
					);
				}
			}

			deviceContext->OMSetRenderTargets(1, &rtv, dsv);
			break;
		}
		default:
			break;
	}
}

void CD3D9to11SharedTexture::Clear(const float color[4]) {
	if (!context) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::Clear() encountered a null context"
		);
	}

	auto *deviceContext = static_cast<ID3D11DeviceContext *>(
		context->GetRenderAPIResource(RenderAPIResource::D3D11DeviceContext)
	);

	deviceContext->ClearRenderTargetView(rtv, color);
}

void CD3D9to11SharedTexture::CopyToTexture(
	GellyInterfaceRef<IManagedTexture> texture
) {
	if (!context) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::CopyToTexture() encountered a null "
			"context"
		);
	}

	if (texture->GetDesc().format != autoGeneratedDesc.format) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::CopyToTexture() encountered a texture "
			"with a different format"
		);
	}

	auto *deviceContext = static_cast<ID3D11DeviceContext *>(
		context->GetRenderAPIResource(RenderAPIResource::D3D11DeviceContext)
	);

	auto *otherResource = static_cast<ID3D11Resource *>(
		texture->GetResource(TextureResource::D3D11_RESOURCE)
	);

	if (otherResource == nullptr) {
		throw std::logic_error(
			"CD3D9to11SharedTexture::CopyToTexture() encountered a null "
			"resource"
		);
	}

	deviceContext->CopyResource(otherResource, d3d11Resource);
}