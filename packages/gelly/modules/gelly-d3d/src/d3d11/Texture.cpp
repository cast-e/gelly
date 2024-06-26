#include "detail/d3d11/Texture.h"

#include "detail/d3d11/ErrorHandling.h"
#include "detail/d3d9/Texture.h"

namespace d3d11 {
Texture::Texture(const d3d9::Texture &d3d9Texture, ID3D11Device *device)
	: texture(nullptr), rtv(nullptr) {
	// Open shared handle from the D3D9 texture we received
	{
		ComPtr<ID3D11Resource> resource;
		DX("Failed to open shared handle",
		   device->OpenSharedResource(
			   d3d9Texture.sharedHandle,
			   __uuidof(ID3D11Resource),
			   (void **)resource.GetAddressOf()
		   ));

		DX("Failed to get a 2D texture from the received D3D9 shared handle!",
		   resource.As(&texture));
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	DX("Failed to create render target view",
	   device->CreateRenderTargetView(texture.Get(), &rtvDesc, &rtv));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	DX("Failed to create shader resource view",
	   device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv));

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	DX("Failed to create sampler state",
	   device->CreateSamplerState(&samplerDesc, &sampler));
}

Texture::Texture(
	int width, int height, DXGI_FORMAT format, ID3D11Device *device
)
	: texture(nullptr), rtv(nullptr), srv(nullptr), sampler(nullptr) {
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET |
							D3D11_BIND_SHADER_RESOURCE |
							D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	DX("Failed to create texture",
	   device->CreateTexture2D(&textureDesc, nullptr, texture.GetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	DX("Failed to create render target view",
	   device->CreateRenderTargetView(
		   texture.Get(), &rtvDesc, rtv.GetAddressOf()
	   ));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	DX("Failed to create shader resource view",
	   device->CreateShaderResourceView(
		   texture.Get(), &srvDesc, srv.GetAddressOf()
	   ));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	DX("Failed to create unordered access view",
	   device->CreateUnorderedAccessView(texture.Get(), &uavDesc, &uav));

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	DX("Failed to create sampler state",
	   device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf()));
}

}  // namespace splatting

void d3d11::Texture::SetAsRT(
	ID3D11DeviceContext *context, ID3D11DepthStencilView *dsv
) const {
	context->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv);
}

void d3d11::Texture::SetAsSR(ID3D11DeviceContext *context, int slot) const {
	context->PSSetShaderResources(slot, 1, srv.GetAddressOf());
}

void d3d11::Texture::SetSampler(ID3D11DeviceContext *context, int slot) const {
	context->PSSetSamplers(slot, 1, sampler.GetAddressOf());
}

void d3d11::Texture::Clear(ID3D11DeviceContext *context, const float color[4])
	const {
	context->ClearRenderTargetView(rtv.Get(), color);
}

ID3D11RenderTargetView *d3d11::Texture::GetRTV() const { return rtv.Get(); }

ID3D11ShaderResourceView *d3d11::Texture::GetSRV() const { return srv.Get(); }

ID3D11UnorderedAccessView *d3d11::Texture::GetUAV() const { return uav.Get(); }

void d3d11::SetMRT(
	ID3D11DeviceContext *context,
	int numTextures,
	d3d11::Texture **textures,
	ID3D11DepthStencilView *dsv
) {
	// 8 is just an arbitrary constant since we never have more than 8 textures.
	ID3D11RenderTargetView *rtViews[8] = {};
	for (int i = 0; i < numTextures; i++) {
		rtViews[i] = textures[i]->GetRTV();
	}

	context->OMSetRenderTargets(numTextures, rtViews, dsv);
}