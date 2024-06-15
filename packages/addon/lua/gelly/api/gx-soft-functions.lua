gellyx = gellyx or {}

local function maxParticles()
	return gelly.GetStatus().MaxParticles
end

local function activeParticles()
	return gelly.GetStatus().ActiveParticles
end

--- Structure representing an individual particle to be spawned.
---@alias gx.ParticleSpawnData {pos: Vector, vel: Vector}

--- Adds the given particle data to the simulation, returning true if they could be added, false otherwise.
---@param particles table<number, gx.ParticleSpawnData>
---@param material table|nil The material to use for the particles, if not provided the active preset's material will be used.
---@return boolean
function gellyx.AddParticles(particles, material)
	material = material or GELLY_ACTIVE_PRESET.Material

	local rawParticles = {}
	if activeParticles() + #particles > maxParticles() then
		return false
	end

	for _, spawnData in ipairs(particles) do
		rawParticles[#rawParticles + 1] = spawnData.pos
		rawParticles[#rawParticles + 1] = spawnData.vel
	end

	gelly.AddParticles(rawParticles, material.Absorption)
	return true
end
