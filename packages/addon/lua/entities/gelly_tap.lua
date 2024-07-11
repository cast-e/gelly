AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Tap"
ENT.Purpose			= "Functional Gelly tap!"
ENT.Emitting = false

function ENT:Initialize()
	if CLIENT then return end

	self:SetModel("models/props_wasteland/prison_sinkchunk001e.mdl")

	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetUseType(SIMPLE_USE)
	self:PhysWake()

	local phys = self:GetPhysicsObject()
	phys:SetMass(10)
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
		local pos = self:LocalToWorld(Vector(0, 0, 7))
		local ang = self:LocalToWorldAngles(Angle(90, 0, 0))

		gellyx.emitters.Sphere( {
			center = pos,
			velocity = ang:Forward() * 7,
			radius = 5,
			density = 10,
			randomness = 1,
		} )
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end
