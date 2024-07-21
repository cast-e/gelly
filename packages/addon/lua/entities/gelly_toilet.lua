AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"

ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Toilet"
ENT.Author			= "Mee & AndrewEathan (with help from PotatoOS)"
ENT.Purpose			= "MOHERFUCKING toilet!"
ENT.Emitting = false

function ENT:Initialize()
	if CLIENT then return end

	self:SetModel("models/props_c17/FurnitureToilet001a.mdl")

	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetUseType(SIMPLE_USE)
	self:PhysWake()

	local phys = self:GetPhysicsObject()
	phys:SetMass(200)
end

function ENT:Use()
	-- notify clients that we're emitting
	self.Emitting = not self.Emitting
	self:EmitSound("buttons/lever1.wav")

	if SERVER then
		self:SetNWBool("Emitting", self.Emitting)
	end
end

function ENT:Think()
	if SERVER then return end

	local isEmitting = self:GetNWBool("Emitting")
	if isEmitting then
		local pos = self:LocalToWorld(Vector(5, 0, -16))
		local ang = self:LocalToWorldAngles(Angle(-70, 0, 0))

		gellyx.emitters.Sphere( {
			center = pos,
			velocity = ang:Forward() * 5,
			radius = 10,
			density = 40,
			randomness = 0,
		} )
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end