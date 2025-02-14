
hook.Add("OnEntityCreated", "gelly.blood-quantity", function(entity)
	if
		not entity:IsValid() or
		(not entity:IsPlayer() and
			not entity:IsNPC() and
			not entity:IsRagdoll() )
	then
		return
	end

	timer.Simple(0, function()
		if not entity:IsValid() then
			return
		end

		entity.MaxBloodQuantity = entity.MaxBloodQuantity or entity:GetMaxHealth() or 100
		entity.BloodQuantity = entity.BloodQuantity or entity:Health() or entity.MaxBloodQuantity
		entity.BloodColor = entity.BloodColor or entity:GetBloodColor() or BLOOD_COLOR_RED
	end )
end)

hook.Add("CreateEntityRagdoll", "gelly.blood-quantity-ragdoll", function(owner, ragdoll)
	ragdoll.MaxBloodQuantity = owner.MaxBloodQuantity
	ragdoll.BloodQuantity = owner.BloodQuantity
	ragdoll.BloodColor = owner.BloodColor or owner:GetBloodColor()
end)