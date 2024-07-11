AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Fountain"
ENT.Purpose			= "Functional Gelly fountain!"
ENT.Emitting = false

function ENT:Initialize()
	if CLIENT then return end

	self:SetModel("models/props_c17/fountain_01.mdl")

	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetUseType(SIMPLE_USE)
	self:PhysWake()

	local phys = self:GetPhysicsObject()
	phys:SetMass(1500)
end

function ENT:Use()
	self.Emitting = not self.Emitting
	self:EmitSound("buttons/button1.wav")

	if SERVER then
		self:SetNWBool("Emitting", self.Emitting)
	end
end

function ENT:Think()
	if SERVER then
		return
	end

	local isEmitting = self:GetNWBool("Emitting")
	if isEmitting then
		local pos = self:LocalToWorld(Vector(0, 0, 276))
		local up = self:LocalToWorldAngles(Angle(-90, 0, 0))

		gellyx.emitters.Sphere( {
			center = pos,
			velocity = up:Forward() * 7,
			radius = 20,
			density = 80,
			randomness = 0.5,
		} )
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end