AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Bathtub"
ENT.Purpose			= "Functional Gelly bathtub!"
ENT.Emitting = false

function ENT:Initialize()
	if CLIENT then return end

	self:SetModel("models/props_interiors/BathTub01a.mdl")
	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetUseType(SIMPLE_USE)
	self:PhysWake()
end

function ENT:Use()
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
		local pos = self:LocalToWorld(Vector(-30, 0, 25))
		local pos1 = self:LocalToWorld(Vector(-30, 12, 25))
		local ang = self:LocalToWorldAngles(Angle(90, 0, 0))

		gellyx.emitters.Sphere( {
			center = pos,
			velocity = ang:Forward() * 4,
			radius = 2,
			density = 8,
			randomness = 0.1,
		} )

		gellyx.emitters.Sphere( {
			center = pos1,
			velocity = ang:Forward() * 4,
			radius = 2,
			density = 8,
			randomness = 0.1,
		} )
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end