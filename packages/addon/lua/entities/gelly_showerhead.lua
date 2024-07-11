AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Shower head"
ENT.Purpose			= "Functional Gelly showerhead!"
ENT.Emitting = false

function ENT:Initialize()
	if CLIENT then return end

	self:SetModel("models/props_wasteland/prison_lamp001a.mdl")
	self:SetSkin(1)
	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetUseType(SIMPLE_USE)
end

function ENT:SpawnFunction(_, tr, class)
	if not tr.Hit then return end

	local ent = ents.Create(class)
	ent:SetModel("models/props_wasteland/prison_lamp001a.mdl")
	ent:SetSkin(1)
	ent:PhysicsInit(SOLID_VPHYSICS)
	ent:SetMoveType(MOVETYPE_VPHYSICS)
	ent:SetSolid(SOLID_VPHYSICS)
	ent:SetUseType(SIMPLE_USE)
	ent:SetPos(tr.HitPos + tr.HitNormal * 10)
	ent:Spawn()
	ent:Activate()

	local phys = ent:GetPhysicsObject()
	if phys:IsValid() then
		phys:SetMass(50)
	end

	return ent
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
		local pos = self:LocalToWorld(Vector(0, 0, -15))
		local ang = self:LocalToWorldAngles(Angle(90, 0, 0))

		gellyx.emitters.Sphere( {
			center = pos,
			velocity = ang:Forward() * 7,
			radius = 10,
			density = 80,
			randomness = 0.5,
		} )
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end