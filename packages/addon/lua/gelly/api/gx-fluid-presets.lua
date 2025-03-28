---@module "gelly.logging"
local logging = include("gelly/logging.lua")
gellyx = gellyx or {}
gellyx.presets = gellyx.presets or {}

GELLY_PRESETS = GELLY_PRESETS or {}
GELLY_CUSTOM_PRESETS = GELLY_CUSTOM_PRESETS or {}
GELLY_ACTIVE_PRESET = GELLY_ACTIVE_PRESET or nil

local function loadPreset(path)
	return include(path)
end

--- Loads or reloads all presets from the filesystem.
---
--- Not particularly useful for production, but useful for development of presets.
---
--- To expand on that, it's probably not a good idea to use this function each frame or something similar as it
--- will cause a lot of unnecessary disk reads.
function gellyx.presets.loadPresetFiles()
	local files, _ = file.Find("gelly/api/presets/immutable/*.lua", "LUA")

	for _, fileName in ipairs(files) do
		local path = "gelly/api/presets/immutable/" .. fileName
		local preset = loadPreset(path)

		logging.info("Loading immutable preset %s", path)
		if preset then
			GELLY_PRESETS[preset.Name] = preset
			logging.info("Loaded immutable preset %s", path)
		end
	end
end

local function selectPreset(preset)
	preset.SolverParams.RestDistanceRatio = preset.SolverParams.RestDistanceRatio or 0.73
	preset.UseWhitewater = preset.UseWhitewater or false
	preset.Material.IsMetal = preset.Material.IsMetal or false
	preset.Material.IsScatter = preset.Material.IsScatter or false

	gelly.ChangeParticleRadius(preset.Radius)
	gelly.SetDiffuseScale(gellyx.settings.get("spray_scale"):GetFloat())
	gelly.SetFluidProperties(preset.SolverParams)
	gelly.SetFluidMaterial(preset.Material)

	if gellyx.settings.get("whitewater_enabled"):GetBool() then
		local currentGellySettings = gelly.GetGellySettings()
		currentGellySettings.EnableWhitewater = preset.UseWhitewater
		gelly.SetGellySettings(currentGellySettings)
	end

	GELLY_ACTIVE_PRESET = preset
	logging.info("Selected preset %s", preset.Name)
end

--- Selects a loaded preset to be used for all future particles.
--- # Warning
--- This function will affect all the current particles beha
---@param name string The name of the preset to select.
function gellyx.presets.select(name)
	local preset = GELLY_PRESETS[name]
	if not preset then
		logging.error("Preset %s does not exist", name)
	end

	selectPreset(preset)

	-- synchronize the fluid settings
	gellyx.settings.get("fluid_radius"):SetFloat(preset.Radius)
	gellyx.settings.get("fluid_viscosity"):SetFloat(preset.SolverParams.Viscosity)
	gellyx.settings.get("fluid_cohesion"):SetFloat(preset.SolverParams.Cohesion)
	gellyx.settings.get("fluid_adhesion"):SetFloat(preset.SolverParams.Adhesion)
	gellyx.settings.get("fluid_friction"):SetFloat(preset.SolverParams.DynamicFriction)
	gellyx.settings.get("fluid_rest_distance_ratio"):SetFloat(preset.SolverParams.RestDistanceRatio)
	gellyx.settings.get("fluid_surface_tension"):SetFloat(preset.SolverParams.SurfaceTension or 0)

	if GELLY_CUSTOMIZATION then
		-- tell the UI that we have new settings
		GELLY_CUSTOMIZATION:ForceSettingUpdate()
	end
end

function gellyx.presets.selectEphemeralPreset(preset)
	if type(preset) ~= "table" then
		logging.error("Ephemeral preset must be a table")
	end

	selectPreset(preset)
end

function gellyx.presets.getActivePreset()
	return table.Copy(GELLY_ACTIVE_PRESET)
end

function gellyx.presets.getAllPresets()
	return table.Copy(GELLY_PRESETS)
end

function gellyx.presets.getCustomPresets()
	return table.Copy(GELLY_CUSTOM_PRESETS)
end

function gellyx.presets.getEffectiveRadius()
	return GELLY_ACTIVE_PRESET.Radius
end

--- Copies a preset's material.
---@param name string The name of the preset to copy the material from.
---@return table The copied material, this is an actual copy and not a reference.
function gellyx.presets.copyPresetMaterial(name)
	local preset = GELLY_PRESETS[name]
	if not preset then
		logging.error("Preset %s does not exist", name)
	end

	local material = table.Copy(preset.Material)
	return material
end

--- Copies a preset.
---@param name string The name of the preset to copy.
---@return table The copied preset, this is an actual copy and not a reference.
function gellyx.presets.copyPreset(name)
	local preset = GELLY_PRESETS[name]
	if not preset then
		logging.error("Preset %s does not exist", name)
	end

	local copiedPreset = table.Copy(preset)
	return copiedPreset
end

function gellyx.presets.isPresetUsingWhitewater()
	return GELLY_ACTIVE_PRESET and GELLY_ACTIVE_PRESET.UseWhitewater
end

local EPHEMERAL_FLUID_SETTING_NAMES = {
	"fluid_radius",
	"fluid_viscosity",
	"fluid_cohesion",
	"fluid_adhesion",
	"fluid_friction",
	"fluid_rest_distance_ratio",
	"fluid_surface_tension"
}

gellyx.settings.registerMultipleOnChange(EPHEMERAL_FLUID_SETTING_NAMES, function()
	if not GELLY_ACTIVE_PRESET then
		return
	end

	local newPreset = gellyx.presets.getActivePreset()
	newPreset.Radius = gellyx.settings.get("fluid_radius"):GetFloat()
	newPreset.SolverParams.Viscosity = gellyx.settings.get("fluid_viscosity"):GetFloat()
	newPreset.SolverParams.Cohesion = gellyx.settings.get("fluid_cohesion"):GetFloat()
	newPreset.SolverParams.Adhesion = gellyx.settings.get("fluid_adhesion"):GetFloat()
	newPreset.SolverParams.DynamicFriction = gellyx.settings.get("fluid_friction"):GetFloat()
	newPreset.SolverParams.RestDistanceRatio = gellyx.settings.get("fluid_rest_distance_ratio"):GetFloat()
	newPreset.SolverParams.SurfaceTension = gellyx.settings.get("fluid_surface_tension"):GetFloat()

	selectPreset(newPreset)
end)

local function getValidFluidColorHex()
	local settingValue = gellyx.settings.get("fluid_color_hex"):GetString()

	if #settingValue ~= 7 then
		return "#000000"
	end

	return settingValue
end

gellyx.settings.registerMultipleOnChange(
	{ "fluid_color_hex", "fluid_color_scale", "fluid_roughness", "fluid_opaque", "fluid_metal", "fluid_scatter",
		"fluid_use_whitewater" },
	function()
		if not GELLY_ACTIVE_PRESET then
			return
		end

		-- CSS-style: #rrggbb
		local colorHex = getValidFluidColorHex()
		local colorScale = gellyx.settings.get("fluid_color_scale"):GetFloat()

		local numericR = tonumber(colorHex:sub(2, 3), 16) / 255
		local numericG = tonumber(colorHex:sub(4, 5), 16) / 255
		local numericB = tonumber(colorHex:sub(6, 7), 16) / 255

		-- invert and multiply by the scale to hopefully approach the absorption that the user wants
		local color = Vector(1 - numericR, 1 - numericG, 1 - numericB) * colorScale
		local diffuseColor = Vector(numericR, numericG, numericB)

		local newPreset = gellyx.presets.getActivePreset()
		newPreset.Material.Absorption = color
		newPreset.Material.DiffuseColor = diffuseColor
		newPreset.Material.Roughness = gellyx.settings.get("fluid_roughness"):GetFloat()
		newPreset.Material.IsSpecularTransmission = not gellyx.settings.get("fluid_opaque"):GetBool()
		newPreset.Material.IsMetal = gellyx.settings.get("fluid_metal"):GetBool()
		newPreset.Material.IsScatter = gellyx.settings.get("fluid_scatter"):GetBool()
		newPreset.UseWhitewater = gellyx.settings.get("fluid_use_whitewater"):GetBool()
		selectPreset(newPreset)
	end)
