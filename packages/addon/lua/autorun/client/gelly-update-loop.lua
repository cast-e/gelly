local ENVBALLS_MODEL_PATH = "models/shadertest/envballs.mdl"

GELLY_SIM_TIMESCALE = 10
GELLY_SIM_RATE_HZ = 60
SIMULATE_GELLY = true

local lastTimescale = GELLY_SIM_TIMESCALE
local lastRate = GELLY_SIM_RATE_HZ

local function isGellyActive()
	return gelly.GetStatus().ActiveParticles > 0
end

hook.Add("GellyLoaded", "gelly.update-loop", function()
	-- forces the engine to compute cubemaps for us
	local envballsModel = ClientsideModel(ENVBALLS_MODEL_PATH)

	-- by this point GellyX has loaded already
	gellyx.settings.registerOnChange("simulation_rate", function()
		GELLY_SIM_RATE_HZ = gellyx.settings.get("simulation_rate"):GetInt()
	end)

	GELLY_SIM_RATE_HZ = gellyx.settings.get("simulation_rate"):GetInt()

	timer.Create("gelly.flex-update-timer", 1 / GELLY_SIM_RATE_HZ, 0, function()
		if lastRate ~= GELLY_SIM_RATE_HZ then
			lastRate = GELLY_SIM_RATE_HZ
			timer.Adjust("gelly.flex-update-timer", 1 / GELLY_SIM_RATE_HZ, 0)
		end

		if SIMULATE_GELLY then -- we don't check if gelly is active because we do need to update deferred particles (to prevent flicker)
			if lastTimescale ~= GELLY_SIM_TIMESCALE then
				lastTimescale = GELLY_SIM_TIMESCALE
				gelly.SetTimeStepMultiplier(GELLY_SIM_TIMESCALE)
			end

			gelly.Simulate(1 / 60) -- flex is programmed to assume a fixed timestep, normally 60hz
		end
	end)

	hook.Add("RenderScene", "gelly.render", function()
		if not isGellyActive() then return end
		gelly.Render()
	end)

	hook.Add("PostDrawOpaqueRenderables", "gelly.composite", function()
		if not isGellyActive() then return end

		render.Model({
			model = ENVBALLS_MODEL_PATH,
			pos = LocalPlayer():EyePos() - LocalPlayer():GetAimVector() * 40,
			angle = LocalPlayer():EyeAngles(),
		}, envballsModel)
		envballsModel:SetNoDraw(true)

		gelly.Composite()
	end)
end)
