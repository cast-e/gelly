#ifndef GELLY_GELLY_H
#define GELLY_GELLY_H

#include <GellyEngine.h>
#include <GellyRenderer.h>
#include <d3d9.h>
#include <wrl.h>

#include <mutex>
#include <semaphore>

using namespace Microsoft::WRL;

struct GellyInitParams {
	int maxParticles;
	int maxColliders;
	int width;
	int height;
	SharedTextures sharedTextures;
};

struct GellyMessage {
	enum Type { LoadMap, Update, Render, AddParticle, SetupCamera, SyncCamera };

	Type type;
	union {
		struct {
			const char *mapName;
		} loadMap;
		struct {
			float deltaTime;
		} update;
		struct {
			Vec4 position;
			Vec3 velocity;
		} addParticle;
		struct {
			float fov;
			float width;
			float height;
			float nearZ;
			float farZ;
		} setupCamera;
		struct {
			Vec3 position;
			Vec3 direction;
		} syncCamera;
	};
};

class RendererCompositor {
private:
	IDirect3DDevice9 *device;
	ComPtr<IDirect3DVertexBuffer9> screenQuad;
	// no declaration needed, we use FVF instead

	struct NDCVertex {
		// It's not in viewport space, but in NDC space.

		static const DWORD FVF = D3DFVF_XYZW | D3DFVF_TEX1;
		float x, y, z, w;
		float u, v;
	};

	ComPtr<IDirect3DVertexShader9> vertexShader;
	ComPtr<IDirect3DPixelShader9> pixelShader;

	// We're not using a ComPtr here because this is meant to be owned
	// by Gelly. There should be nothing happening with the ref count.
	IDirect3DTexture9 *depthTexture;

	void CreateScreenQuad();
	void CreateShaders();
	void BindShaderResources();

public:
	explicit RendererCompositor(
		IDirect3DDevice9 *device, IDirect3DTexture9 *depthTexture
	);
	~RendererCompositor() = default;
	void Composite();
};

/**
 * Gelly is a threaded wrapper of GellyScene and GellyRenderer.
 * It integrates both of them together, but uses a separate thread for any
 * of the work due to how Garry's Mod works. (it simply doesn't work on the
 * main thread)
 */
class Gelly {
private:
	GellyScene *scene;
	GellyRenderer *renderer;

	// Semaphores are used to communicate between the main thread and the gelly
	// thread. This is because there really is no multithreading involved which
	// would require something like a mutex for resource locking at critical
	// sections, we just need to do it on a separate thread due to technical
	// limitations.

	// rx
	std::binary_semaphore mainToThread;
	// tx
	std::binary_semaphore threadToMain;

	std::thread thread;
	GellyMessage *message;

	/**
	 * Processes a message sent from the main thread.
	 * This is called by the thread, but note that the message member is usually
	 * only valid in this function.
	 */
	void ProcessMessage();
	void LoadMap(const char *mapName);
	void Update(float deltaTime);
	void AddParticle(Vec4 position, Vec3 velocity);
	void SetupCamera(
		float fov, float width, float height, float nearZ, float farZ
	);
	void SyncCamera(Vec3 position, Vec3 dir);
	void Render();

public:
	RendererCompositor compositor;

	explicit Gelly(
		const GellyInitParams &params,
		IDirect3DDevice9 *device,
		IDirect3DTexture9 *depthTexture
	);
	/**
	 * Initializes Gelly on a separate thread. This is called in the
	 * constructor, but ran on a different thread due to technical limitations.
	 * @param params
	 */
	[[noreturn]] static void InitThreaded(
		Gelly *gelly, const GellyInitParams &params
	);

	void SendGellyMessage(GellyMessage &newMessage);

	~Gelly();
};

#endif	// GELLY_GELLY_H
