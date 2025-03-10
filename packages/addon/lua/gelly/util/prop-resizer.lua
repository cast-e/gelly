local logging = include("gelly/logging.lua")

local function isPropResizerUninstalled()
	return scripted_ents.Get("sizehandler") == nil
end

if isPropResizerUninstalled() then
	logging.warn("Prop Resizer was not detected. Not loading prop resizer utilities.")
	return
else
	logging.info("Prop Resizer detected. Loading prop resizer utilities.")
end

-- we want to wrap the horrible addon API into a better one so
-- that Gelly can hook into it

local originalAPI = scripted_ents.GetStored("sizehandler").t
local originalThink = originalAPI.Think

function originalAPI:Think()
	originalThink(self)
	local scale = Vector(self:GetActualPhysicsScale())

	if scale ~= self.Gelly_LastScale then
		self.Gelly_LastScale = scale
		hook.Run("GellyXPropResized", self:GetParent(), scale)
	end
end
