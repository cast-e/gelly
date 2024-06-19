#include "shader.h"

#include <helpers/throw-informative-exception.h>

#include <stdexcept>

namespace gelly {
namespace renderer {
template <typename ShaderType>
Shader<ShaderType>::Shader(const ShaderCreateInfo &createInfo)
	: createInfo(createInfo) {
	if constexpr (is_vertex_shader<ShaderType>) {
		shader = CreateVertexShader();
	} else if constexpr (is_pixel_shader<ShaderType>) {
		shader = CreatePixelShader();
	} else if constexpr (is_geometry_shader<ShaderType>) {
		shader = CreateGeometryShader();
	}
}

template <typename ShaderType>
auto Shader<ShaderType>::GetRawShader() -> ComPtr<ShaderType> {
	return shader;
}

template <typename ShaderType>
auto Shader<ShaderType>::GetBlob() -> void * {
	return createInfo.shaderBlob;
}

template <typename ShaderType>
auto Shader<ShaderType>::GetBlobSize() -> unsigned int {
	return createInfo.shaderBlobSize;
}

template <typename ShaderType>
std::enable_if_t<is_vertex_shader<ShaderType>, auto>
Shader<ShaderType>::CreateVertexShader() -> ComPtr<ID3D11VertexShader> {
	ComPtr<ID3D11VertexShader> vertexShader;
	const auto result = createInfo.device->GetRawDevice()->CreateVertexShader(
		createInfo.shaderBlob,
		createInfo.shaderBlobSize,
		nullptr,
		vertexShader.GetAddressOf()
	);

	GELLY_RENDERER_THROW_ON_FAIL(
		result, std::invalid_argument, "Failed to create vertex shader"
	);

	return vertexShader;
}

template <typename ShaderType>
std::enable_if_t<is_pixel_shader<ShaderType>, auto>
Shader<ShaderType>::CreatePixelShader() -> ComPtr<ID3D11PixelShader> {
	ComPtr<ID3D11PixelShader> pixelShader;
	const auto result = createInfo.device->GetRawDevice()->CreatePixelShader(
		createInfo.shaderBlob,
		createInfo.shaderBlobSize,
		nullptr,
		pixelShader.GetAddressOf()
	);

	GELLY_RENDERER_THROW_ON_FAIL(
		result, std::invalid_argument, "Failed to create pixel shader"
	);

	return pixelShader;
}

template <typename ShaderType>
std::enable_if_t<is_geometry_shader<ShaderType>, auto>
Shader<ShaderType>::CreateGeometryShader() -> ComPtr<ID3D11GeometryShader> {
	ComPtr<ID3D11GeometryShader> geometryShader;
	const auto result = createInfo.device->GetRawDevice()->CreateGeometryShader(
		createInfo.shaderBlob,
		createInfo.shaderBlobSize,
		nullptr,
		geometryShader.GetAddressOf()
	);

	GELLY_RENDERER_THROW_ON_FAIL(
		result, std::invalid_argument, "Failed to create geometry shader"
	);

	return geometryShader;
}
}  // namespace renderer
}  // namespace gelly