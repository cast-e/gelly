SWEP.Category = "Gelly"
SWEP.Spawnable = true
SWEP.AdminOnly = false
SWEP.PrintName = "Piss"

SWEP.ViewModel = "models/props_c17/FurnitureDrawer001a_Shard01.mdl"
SWEP.WorldModel = "models/props_c17/FurnitureDrawer001a_Shard01.mdl"
SWEP.ViewModelFOV = 54
SWEP.UseHands = false

SWEP.Primary.ClipSize = -1
SWEP.Primary.DefaultClip = -1
SWEP.Primary.Automatic = true
SWEP.Primary.Ammo = ""

function SWEP:Initialize()
	self:SetHoldType("magic")
end

function SWEP:PrimaryAttack()
	if SERVER then
		self:CallOnClient("PrimaryAttack")
		return
	end

	local owner = self:GetOwner()
	local jitterDisplacement = VectorRand() * 2.5

	gellyx.emitters.Sphere({
		center = owner:GetBonePosition(0) + owner:GetAimVector() * 5 + jitterDisplacement,
		velocity = owner:GetAimVector() * 7 * ((owner:GetAimVector().z + 3) / 4),
		radius = 20,
		density = 15,
		randomness = 0,
	})

	self:SetNextPrimaryFire(CurTime())
end

function SWEP:SecondaryAttack()
end