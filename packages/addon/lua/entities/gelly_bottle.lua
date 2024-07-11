AddCSLuaFile()

ENT.Type = "anim"
ENT.Base = "base_gmodentity"
ENT.Category		= "Gelly"
ENT.Spawnable 		= true
ENT.PrintName		= "Bottle"
ENT.Purpose			= "Functional bottle!"
ENT.Emitting		= false

function ENT:Initialize()
	if CLIENT then
		self.FlowSound = CreateSound(self, "ambient/water/leak_1.wav")
		self.FlowSound:Stop()
		return
	end

	self:PrecacheGibs()
	self:SetModel("models/props_junk/garbage_glassbottle003a.mdl")
	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)

	local phys = self:GetPhysicsObject()
	phys:SetMass(20)
end

function ENT:Break(force)
	if SERVER then return end

	self:EmitSound("physics/glass/glass_bottle_break" .. math.random(1, 2) .. ".wav")
	self:GibBreakClient(force * 2)

	gellyx.emitters.Sphere( {
		center = self:GetPos(),
		velocity = force / 3,
		radius = 20,
		density = 40,
		randomness = 2,
	} )
end

function ENT:OnTakeDamage(damage)
	-- have to do some math since the damage force is wayyy too high
	local f = damage:GetDamageForce()
	local mag = f:Length()

	self:Break(f:GetNormalized() * math.Clamp(mag * 0.1, 0, 500))
end

function ENT:PhysicsCollide(col)
	if col.Speed > 100 then
		self:Break(col.OurOldVelocity * 0.5)
	end
end

function ENT:Think()
	if SERVER then return end

	local ang = self:GetAngles():Up()

	local last = self.Emitting
	self.Emitting = ang:Dot(Vector(0, 0, 1)) <= 0

	if self.Emitting then
		local pos = self:LocalToWorld(Vector(0, 0, 13))
		ang = self:LocalToWorldAngles(Angle(90, 0, 0))

		gellyx.emitters.Sphere( {
			center = pos,
			velocity = -ang:Forward() * 2,
			radius = 8,
			density = 4,
			randomness = 0,
		} )
	end

	if last ~= self.Emitting then
		if self.Emitting then
			self.FlowSound:Play()
		else
			self.FlowSound:Stop()
		end
	end

	self:SetNextClientThink(CurTime() + 0.01)
	return true
end

function ENT:OnRemove()
	if CLIENT and self.FlowSound then
		self.FlowSound:Stop()
		self.FlowSound = nil
	end
end