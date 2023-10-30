#ifndef GELLY_IRENDERCONTEXT_H
#define GELLY_IRENDERCONTEXT_H

#include <GellyInterface.h>

#include <cstdint>

#include "IManagedTexture.h"

// Distinction between handles and resources since this context will abstract
// away rendering resources.
enum class RenderAPIResource {
	D3D11Device,
	D3D11DeviceContext,
};

// This should only be used to verify invariants.
// All concrete classes are expected to raise logic errors if they are
// connected to an incompatible context.
enum class ContextRenderAPI {
	D3D11,
};

/**
 * The render context abstracts the host rendering API for fluid renderers.
 * This enables higher compatibility, and also allows for automated resource
 * management. That being said, fluid renderers are still expected not to rely
 * on the underlying rendering API.
 *
 * @note For the critical sections which require a rendering API, the context
 * provides several core resources. These are the device and device context for
 * D3D11.
 */
gelly_interface IRenderContext {
public:
	// All subclasses must destroy their resources here, but also in the event
	// of a device reset.
	virtual ~IRenderContext() = 0;

	virtual void *GetRenderAPIResource(RenderAPIResource resource) = 0;
	virtual ContextRenderAPI GetRenderAPI() = 0;

	/**
	 * Will throw an exception if the texture already exists!
	 * @param name
	 * @param desc
	 * @return
	 */
	virtual IManagedTexture *CreateTexture(
		const char *name, const GellyTextureDesc &desc
	) = 0;

	/**
	 * Will throw an exception if the texture does not exist!
	 * @param name
	 * @return
	 */
	virtual void DestroyTexture(const char *name) = 0;

	/**
	 * Sets the dimensions of the context, which in turn sets the dimensions of
	 * any fluid renderer using this context.
	 */
	virtual void SetDimensions(uint16_t width, uint16_t height) = 0;
};

#endif	// GELLY_IRENDERCONTEXT_H
