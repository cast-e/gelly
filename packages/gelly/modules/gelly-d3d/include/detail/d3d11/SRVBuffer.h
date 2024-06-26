#ifndef GELLY_SRVBUFFER_H
#define GELLY_SRVBUFFER_H

#include <d3d11.h>
#include <wrl.h>

#include "Buffer.h"
#include "ErrorHandling.h"

using namespace Microsoft::WRL;

namespace d3d11 {
template <typename T>
class SRVBuffer {
private:
	ComPtr<ID3D11ShaderResourceView> view;

public:
	SRVBuffer(
		ID3D11Device *device, DXGI_FORMAT format, const Buffer<T> &buffer
	) {
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.BufferEx.FirstElement = 0;
		desc.BufferEx.NumElements = buffer.GetCapacity();

		DX("Failed to create SRV!",
		   device->CreateShaderResourceView(
			   buffer.Get(), &desc, view.GetAddressOf()
		   ));
	}

	SRVBuffer(
		ID3D11Device *device,
		DXGI_FORMAT format,
		ID3D11Buffer *bufferReference,
		int maxCapacity,
		bool isStructured = true
	) {
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		if (isStructured) {
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			desc.BufferEx.FirstElement = 0;
			desc.BufferEx.NumElements = maxCapacity;
		} else {
			desc.Format = format;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			desc.BufferEx.FirstElement = 0;
			desc.BufferEx.NumElements = maxCapacity;
		}

		DX("Failed to create SRV!",
		   device->CreateShaderResourceView(
			   bufferReference, &desc, view.GetAddressOf()
		   ));
	}

	~SRVBuffer() = default;

	[[nodiscard]] ID3D11ShaderResourceView *Get() const { return view.Get(); }
};
}  // namespace splatting
#endif	// GELLY_SRVBUFFER_H
