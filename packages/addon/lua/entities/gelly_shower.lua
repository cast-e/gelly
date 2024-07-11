AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Shower"
ENT.Purpose			= "Functional Gelly shower!"
ENT.Emitting = false

function ENT:Initialize()
	if CLIENT then
		self.FlowAreas = {
			{ Vector( 10,  72, 7), 	Angle( 44, -69, -180) },
			{ Vector(-24,  43, 8), 	Angle( 44, -37, -180) },
			{ Vector(-38,  0,  7), 	Angle( 44,  0,  -180) },
			{ Vector(-24, -44, 7), 	Angle( 44,  32, -180) },
			{ Vector( 10, -73, 7), 	Angle( 44,  68, -180) }
		}
		return
	end

	self:SetModel("models/props_wasteland/shower_system001a.mdl")

	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetUseType(SIMPLE_USE)

	local phys = self:GetPhysicsObject()
	phys:SetMass(50)
end

function ENT:Use()
	self.Emitting = not self.Emitting
	self:EmitSound("buttons/button1.wav")

	if SERVER then
		self:SetNWBool("Emitting", self.Emitting)
	end
end

function ENT:Think()
	if SERVER then return end

	local isEmitting = self:GetNWBool("Emitting")
	if isEmitting then
		for k,v in pairs(self.FlowAreas) do
			local pos = self:LocalToWorld(v[1])
			local ang = self:LocalToWorldAngles(v[2]) + Angle(0, 180, 0)

			gellyx.emitters.Sphere( {
				center = pos,
				velocity = ang:Forward() * 5,
				radius = 10,
				density = 20,
				randomness = 0.2,
			} )
		end
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end