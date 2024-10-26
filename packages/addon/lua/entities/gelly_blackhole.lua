AddCSLuaFile()

ENT.Type			= "anim"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Black Hole"
ENT.Purpose			= "Gelly Blackhole!"

function ENT:Initialize()
	if CLIENT then
		self.ForceField = gellyx.forcefield.create( {
			Position = self:GetPos(),
			Radius = 1000,
			Strength = -25,
			LinearFalloff = true,
			Mode = gellyx.forcefield.Mode.Force,
		} )
	end
	self:SetModel("models/hunter/misc/sphere075x075.mdl")
	self:SetMaterial("lights/white")
	self:SetColor(Color(0,0,0))
	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
end

function ENT:SpawnFunction(ply, tr, class)
	if not tr.Hit then return end
	local ent = ents.Create(class)
	ent:SetPos(tr.HitPos + tr.HitNormal * 50)
	ent:Spawn()
	ent:Activate()

	return ent
end

function ENT:Think()
	if SERVER then return end

	if self.ForceField then
		self.ForceField:SetPos(self:GetPos())
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end

function ENT:OnRemove()
	if self.ForceField then
		self.ForceField:Remove()
	end
end