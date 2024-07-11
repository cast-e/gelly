-- The sandbox mod really just enables the Gelly Gun and thats about it.

print("Gwater mod loaded")

gellyx.presets.select("Water")

hook.Add("GellyModsShutdown", "gellyx.gwater-mod", function()
	gelly.Reset()

	print("Gwater mod unloaded")
end)