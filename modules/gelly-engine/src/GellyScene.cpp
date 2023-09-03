#include "GellyScene.h"

#include <memory>

#include "MeshConvert.h"

GellyScene::GellyScene(
	NvFlexLibrary *library, int maxParticles, int maxColliders
)
	: maxParticles(maxParticles),
	  currentParticleCount(0),
	  solver(nullptr),
	  library(library),
	  params(nullptr),
	  positions(library, maxParticles),
	  velocities(library, maxParticles),
	  phases(library, maxParticles),
	  activeIndices(library, maxParticles),
	  colliders(library, maxColliders),
	  computeDeviceName(NvFlexGetDeviceName(library)) {
	gpuWork = false;

	NvFlexSolverDesc solverDesc{};
	NvFlexSetSolverDescDefaults(&solverDesc);
	solverDesc.maxParticles = maxParticles;
	solverDesc.maxNeighborsPerParticle = 64;
	solverDesc.maxContactsPerParticle = 6;

	solver = NvFlexCreateSolver(library, &solverDesc);

	auto defaultParams = new NvFlexParams();
	memset(defaultParams, 0, sizeof(NvFlexParams));
	params = defaultParams;

	params->gravity[0] = 0.0f;
	params->gravity[1] = 0.0f;
	params->gravity[2] = -10.0f;

	params->wind[0] = 0.0f;
	params->wind[1] = 0.0f;
	params->wind[2] = 0.0f;

	params->radius = 5.15f;
	params->viscosity = 0.01f;
	params->dynamicFriction = 0.1f;
	params->staticFriction = 0.1f;
	params->particleFriction =
		0.1f;  // scale friction between particles by default
	params->freeSurfaceDrag = 0.0f;
	params->drag = 0.0f;
	params->lift = 0.0f;
	params->numIterations = 3;
	params->fluidRestDistance = 3.8f;
	params->solidRestDistance = 11.f;

	params->anisotropyScale = 1.0f;
	params->anisotropyMin = 0.1f;
	params->anisotropyMax = 2.0f;
	params->smoothing = 1.0f;

	params->dissipation = 0.0f;
	params->damping = 0.0f;
	params->particleCollisionMargin = 1.f;
	params->shapeCollisionMargin = 1.0f;
	params->collisionDistance = params->fluidRestDistance *
								3.f;  // Needed for tri-particle intersection
	params->sleepThreshold = 0.0f;
	params->shockPropagation = 0.0f;
	params->restitution = 1.0f;

	params->maxSpeed = FLT_MAX;
	params->maxAcceleration = 100.0f;  // approximately 10x gravity

	params->relaxationMode = eNvFlexRelaxationLocal;
	params->relaxationFactor = 1.0f;
	params->solidPressure = 1.0f;
	params->adhesion = 0.0f;
	params->cohesion = 0.05f;
	params->surfaceTension = 0.0f;
	params->vorticityConfinement = 85.0f;
	params->buoyancy = 1.0f;
	params->diffuseThreshold = 100.0f;
	params->diffuseBuoyancy = 1.0f;
	params->diffuseDrag = 0.8f;
	params->diffuseBallistic = 16;
	params->diffuseLifetime = 2.0f;
	NvFlexSetParams(solver, params);
}

GellyScene::~GellyScene() {
	NvFlexDestroySolver(solver);
	if (d3dParticleBuffer) NvFlexUnregisterD3DBuffer(d3dParticleBuffer);
	delete params;
}

void GellyScene::EnterGPUWork() {
	if (gpuWork) {
		return;
	}

	gpuWork = true;

	positions.map();
	velocities.map();
	phases.map();
	activeIndices.map();
	colliders.EnterGPUWork();
}

void GellyScene::ExitGPUWork() {
	if (!gpuWork) {
		return;
	}

	gpuWork = false;

	positions.unmap();
	velocities.unmap();
	phases.unmap();
	activeIndices.unmap();
	colliders.ExitGPUWork();
}

void GellyScene::Update(float deltaTime) {
	NvFlexCopyDesc copyDesc{};
	copyDesc.dstOffset = 0;
	copyDesc.srcOffset = 0;
	copyDesc.elementCount = currentParticleCount;

	NvFlexSetParticles(solver, positions.buffer, &copyDesc);
	NvFlexSetVelocities(solver, velocities.buffer, &copyDesc);
	NvFlexSetPhases(solver, phases.buffer, &copyDesc);
	NvFlexSetActive(solver, activeIndices.buffer, &copyDesc);
	NvFlexSetActiveCount(solver, currentParticleCount);
	NvFlexSetShapes(
		solver,
		colliders.geometries.buffer,
		colliders.positions.buffer,
		colliders.rotations.buffer,
		colliders.prevPositions.buffer,
		colliders.prevRotations.buffer,
		colliders.flags.buffer,
		colliders.GetEntityCount()
	);
	NvFlexSetParams(solver, params);
	NvFlexUpdateSolver(solver, deltaTime, 2, false);

	NvFlexGetParticles(solver, positions.buffer, &copyDesc);
	if (d3dParticleBuffer)
		NvFlexGetParticles(solver, d3dParticleBuffer, &copyDesc);
	NvFlexGetVelocities(solver, velocities.buffer, &copyDesc);
	NvFlexGetPhases(solver, phases.buffer, &copyDesc);
}

void GellyScene::AddParticle(Vec4 position, Vec3 velocity) {
	if (!gpuWork) {
		return;
	}

	if (currentParticleCount >= maxParticles) {
		return;
	}

	positions[currentParticleCount] = position;
	velocities[currentParticleCount] = velocity;
	phases[currentParticleCount] =
		NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid);
	activeIndices[currentParticleCount] = currentParticleCount++;
}

Vec4 *GellyScene::GetPositions() const {
	if (!gpuWork) {
		return nullptr;
	}

	return positions.mappedPtr;
}

Vec3 *GellyScene::GetVelocities() const {
	if (!gpuWork) {
		return nullptr;
	}

	return velocities.mappedPtr;
}

int GellyScene::GetCurrentParticleCount() const { return currentParticleCount; }

void GellyScene::AddBSP(
	const std::string &mapName, uint8_t *data, size_t dataSize
) {
	if (!gpuWork) {
		return;
	}

	auto info = MeshConvert_LoadBSP(data, dataSize);
	colliders.AddTriangleMesh(mapName, info);
	MeshConvert_FreeBSP(info);

	colliders.AddEntity(
		{.position = Vec3{0, 0, 0},
		 .rotation = Quat{0, 0, 0, 1},
		 .modelPath = mapName}
	);
}

void GellyScene::RegisterD3DBuffer(
	void *buffer, int elementCount, int elementStride
) {
	d3dParticleBuffer =
		NvFlexRegisterD3DBuffer(library, buffer, elementCount, elementStride);
}