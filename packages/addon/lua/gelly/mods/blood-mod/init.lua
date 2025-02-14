-- One of the benefits of the mod system is that it'll never be ran on the server, so most conditional realm blocks are unnecessary.
print("Blood mod loaded")

local DAMAGE_TYPE_BLOOD_CONFIGS = {
    {
        DamageFlags = bit.bor(DMG_BULLET, DMG_ALWAYSGIB),
        MinDensity = 200,
        MaxDensity = 400,
        VelocityPower = 15,
        Randomness = 0.3,
        CubeSize = 5,
        DamageMultiplier = 10,
    },
    {
        DamageFlags = DMG_BLAST,
        MinDensity = 100,
        MaxDensity = 800,
        VelocityPower = 10,
        Randomness = 0.3,
        CubeSize = 20,
        DamageMultiplier = 200,
        FromEntity = true,
    },
    {
        DamageFlags = bit.bor(DMG_SLASH, DMG_CLUB),
        MinDensity = 200,
        MaxDensity = 2200,
        VelocityPower = 15,
        Randomness = 0.5,
        CubeSize = 10,
        DamageMultiplier = 5,
    },
    {
        DamageFlags = DMG_CRUSH,
        MinDensity = 500,
        MaxDensity = 700,
        VelocityPower = 10,
        Randomness = 0.7,
        CubeSize = 10,
        DamageMultiplier = 25,
    },
}

local WEAPON_BLOOD_CONFIGS = {
    m9k_dbarrel = {
        MinDensity = 100,
        MaxDensity = 200,
        VelocityPower = 18,
        Randomness = 0.3,
        CubeSize = 52,
        DamageMultiplier = 35,
    },
    weapon_shotgun = {
        MinDensity = 300,
        MaxDensity = 500,
        VelocityPower = 15,
        Randomness = 0.3,
        CubeSize = 7,
        DamageMultiplier = 50,
    },
    weapon_357 = {
        MinDensity = 100,
        MaxDensity = 200,
        VelocityPower = 20,
        Randomness = 0.1,
        CubeSize = 10,
        DamageMultiplier = 20,
    },
}

local BLOOD_COLOR_ABSORPTION = {
    [BLOOD_COLOR_RED] = Vector(0.3, 1.1, 1.1),
    [BLOOD_COLOR_YELLOW] = Vector(0.3, 0.3, 1.1),
    [BLOOD_COLOR_GREEN] = Vector(1.1, 0.3, 1.1),
    [BLOOD_COLOR_MECH] = Vector(100.0, 100.0, 100.0),
    [BLOOD_COLOR_ANTLION] = Vector(0.5, 0.3, 1.1),
    [BLOOD_COLOR_ZOMBIE] = Vector(0.3, 0.7, 1.1),
    [BLOOD_COLOR_ANTLION_WORKER] = Vector(0.5, 0.3, 1.1),
}

local function getDamageTypeConfig(damageType)
    for _, config in ipairs(DAMAGE_TYPE_BLOOD_CONFIGS) do
        if bit.band(damageType, config.DamageFlags) ~= 0 then
            return config
        end
    end
    return nil
end

local function getConfig(attacker, damageType)
    local config = nil

    if attacker:IsValid() and attacker.GetActiveWeapon and IsValid(attacker:GetActiveWeapon()) then
        config = WEAPON_BLOOD_CONFIGS[attacker:GetActiveWeapon():GetClass()]
    end

    if damageType and not config then
        config = getDamageTypeConfig(damageType)
    end

    return config
end

local function getClosestBone(victim, damagePosition)
    local closestBone, closestDistance
    local boneCount = victim:GetBoneCount()

    if boneCount and boneCount > 0 then
        for i = 0, boneCount - 1 do
            local bonePos = victim:GetBonePosition(i)
            if bonePos and bonePos ~= Vector(0, 0, 0) then
                local distance = bonePos:DistToSqr(damagePosition)
                if not closestDistance or distance < closestDistance then
                    closestBone = i
                    closestDistance = distance
                end
            end
        end
    end

    return closestBone
end

local function getImpactPosition(victim, damagePosition)
    local trace = util.TraceLine({
        start = damagePosition,
        endpos = victim:LocalToWorld(victim:OBBCenter()),
        filter = victim,
    })

    return trace.HitPos or damagePosition
end

local function sprayBlood(damageType, victim, attacker, position, force, damage, material)
    local config = getConfig(attacker, damageType)
    if not config then return end

    local normal = (position - attacker:GetPos()):GetNormalized()
    local velocity = normal * config.VelocityPower
    velocity.z = velocity.z * 0.5

    local sprayDensity = math.Clamp(config.MinDensity + (damage * config.DamageMultiplier), config.MinDensity, config.MaxDensity)

    gellyx.emitters.Cube({
        center = position,
        velocity = velocity,
        bounds = Vector(config.CubeSize, config.CubeSize, config.CubeSize),
        density = sprayDensity * 0.5,
        randomness = config.Randomness,
        material = material,
    })

    gellyx.emitters.Cube({
        center = position,
        velocity = -velocity,
        bounds = Vector(config.CubeSize, config.CubeSize, config.CubeSize),
        density = sprayDensity * 0.5,
        randomness = config.Randomness,
        material = material,
    })
end

local function sprayExplosionBlood(damageType, victim, attacker, position, force, damage, material)
    local config = getDamageTypeConfig(damageType)
    if not config then return end

    local boneIndex = getClosestBone(victim, position) or 0
    local sprayOrigin = victim:GetBonePosition(boneIndex) 
        or victim:LocalToWorld(victim:OBBCenter())

    if position.z < victim:GetPos().z + 10 then
        sprayOrigin.z = sprayOrigin.z + 30
    end

    local direction = (sprayOrigin - position):GetNormalized()
    local heightDiff = sprayOrigin.z - position.z
    local verticalBoost = math.Clamp(-heightDiff / 100, -1, 1)
    direction.z = direction.z + verticalBoost
    direction:Normalize()

    local velocity = direction * config.VelocityPower + VectorRand() * config.Randomness

    if position.z < victim:WorldSpaceCenter().z then
        velocity.z = velocity.z + 5
    end

    local explosionIsAbove = position.z > (victim:WorldSpaceCenter().z + 20)
    if explosionIsAbove then
        velocity.z = velocity.z - 15
    end

    local maxRadius = 128
    local distance = (sprayOrigin - position):Length()
    local proximity = 1 - math.Clamp(distance / maxRadius, 0, 1)

    local velocityBoostFactor = 0.5  
    velocity = velocity * (1 + velocityBoostFactor * proximity)

    local explosionDensity = math.Clamp(config.MaxDensity * 2, 200, 2000)
    local densityBoostFactor = 5.0  
    explosionDensity = explosionDensity * (1 + densityBoostFactor * proximity)

    local meshDensity = math.Clamp(explosionDensity / 1500, 0.05, 0.7)

    gellyx.emitters.Mesh({
        entity = victim,
        velocity = velocity,
        density = meshDensity * 0.5,
        material = material,
    })

    gellyx.emitters.Mesh({
        entity = victim,
        velocity = -velocity,
        density = meshDensity * 0.5,
        material = material,
    })
end

local function bleedEffect(damage, victim, position, material)
    local bleedDuration = math.Clamp(damage / 10, 2, 10)
    local boneIndex = getClosestBone(victim, position)
    local relativePosition = position - (boneIndex and victim:GetBonePosition(boneIndex) or Vector(0, 0, 0))

    timer.Create("gelly.builtin.steady-bleed-timer" .. victim:EntIndex(), 0.01, bleedDuration * 100, function()
        if not victim:IsValid() then
            timer.Remove("gelly.builtin.steady-bleed-timer" .. victim:EntIndex())
            return
        end

        local bonePos = boneIndex and victim:GetBonePosition(boneIndex)
        if bonePos then
            gellyx.emitters.Cube({
                center = bonePos + relativePosition,
                velocity = (position - victim:GetPos()):GetNormalized() * 4,
                bounds = Vector(2, 2, 2),
                density = damage,
                randomness = 0.1,
                material = material,
            })
        end
    end)
end

hook.Add("GellyXDamage", "gelly.builtin.blood-mod", function(victim, attacker, position, force, damage, type)
    if not victim:IsValid() then return end

    local absorption = BLOOD_COLOR_ABSORPTION[victim:GetInternalVariable("BloodColor") or BLOOD_COLOR_RED]

    if not absorption then
        return
    end

    if GELLY_ACTIVE_PRESET.Name ~= "Blood" then
        gellyx.presets.select("Blood")
        gelly.Reset()
    end

    if bit.band(type, DMG_BLAST) ~= 0 then
        sprayExplosionBlood(type, victim, attacker, position, force, damage, {
            Roughness = 0,
            IsSpecularTransmission = true,
            RefractiveIndex = 1.373,
            Absorption = absorption * 40,
            DiffuseColor = Vector(0, 0, 0),
        })
    else
        sprayBlood(type, victim, attacker, position, force, damage, {
            Roughness = 0,
            IsSpecularTransmission = true,
            RefractiveIndex = 1.373,
            Absorption = absorption * 40,
            DiffuseColor = Vector(0, 0, 0),
        })

        bleedEffect(damage, victim, position, {
            Roughness = 0,
            IsSpecularTransmission = true,
            RefractiveIndex = 1.373,
            Absorption = absorption * 40,
            DiffuseColor = Vector(0, 0, 0),
        })
    end
end )

gellyx.presets.select("Blood")

hook.Add("GellyModsShutdown", "gelly.builtin.blood-mod", function()
    hook.Remove("GellyXDamage", "gelly.builtin.blood-mod")
    hook.Remove("OnEntityCreated", "gelly.builtin.blood-quantity")
	hook.Remove("CreateEntityRagdoll", "gelly.builtin.blood-quantity-ragdoll")

    -- remove any left over blood
    gelly.Reset()

    print("Blood mod unloaded")
end)
