gellymp = gellymp or {}

gellymp.CUBE_EMITTER_NETMSG = "GellyCubeEmitter"
gellymp.SPHERE_EMITTER_NETMSG = "GellySphereEmitter"
gellymp.MESH_EMITTER_NETMSG = "GellyMeshEmitter"

gellymp.RESET_CLIENT_NETMSG = "GellyClientReset"
gellymp.RESET_NETMSG = "GellyReset"

if SERVER then
    util.AddNetworkString(gellymp.CUBE_EMITTER_NETMSG)
    util.AddNetworkString(gellymp.SPHERE_EMITTER_NETMSG)
    util.AddNetworkString(gellymp.MESH_EMITTER_NETMSG)

    util.AddNetworkString(gellymp.RESET_CLIENT_NETMSG)
    util.AddNetworkString(gellymp.RESET_NETMSG)

    function gellymp.CubeEmitter(params, ply)
        net.Start(gellymp.CUBE_EMITTER_NETMSG)
            net.WriteVector(params.center)
            net.WriteVector(params.velocity)
            net.WriteVector(params.bounds)
            net.WriteFloat(params.density)
            net.WriteMatrix(params.transform or Matrix())
            net.WriteTable(params.material or {})
            net.WriteFloat(params.invMass)
        net.SendOmit(ply)
    end

    function gellymp.SphereEmitter(params, ply)
        net.Start(gellymp.SPHERE_EMITTER_NETMSG)
            net.WriteVector(params.center)
            net.WriteFloat(params.radius)
            net.WriteVector(params.velocity)
            net.WriteFloat(params.density)
            net.WriteFloat(params.randomness)
            net.WriteTable(params.material or {})
        net.SendOmit(ply)
    end

    function gellymp.MeshEmitter(params, ply)
        net.Start(gellymp.MESH_EMITTER_NETMSG)
            net.WriteEntity(params.entity)
            net.WriteFloat(params.density)
            net.WriteTable(params.material or {})
        net.SendOmit(ply)
    end

    net.Receive(gellymp.RESET_CLIENT_NETMSG, function(_, ply)
        net.Start(gellymp.RESET_NETMSG)
        net.Broadcast()
    end)
end

if CLIENT then
    net.Receive(gellymp.CUBE_EMITTER_NETMSG, function(_, _)
        local center = net.ReadVector()
        local velocity = net.ReadVector()
        local bounds = net.ReadVector()
        local density = net.ReadFloat()
        local transform = net.ReadMatrix()
        local material = net.ReadTable()
        local invMass = net.ReadFloat()
        gellyx.emitters.Cube({
            center = center,
            velocity = velocity,
            bounds = bounds,
            density = density,
            transform = transform,
            material = material,
            invMass = invMass,
        })
    end)

    net.Receive(gellymp.SPHERE_EMITTER_NETMSG, function(_, _)
        local center = net.ReadVector()
        local radius = net.ReadFloat()
        local velocity = net.ReadVector()
        local density = net.ReadFloat()
        local randomness = net.ReadFloat()
        local material = net.ReadTable()
        gellyx.emitters.Sphere({
            center = center,
            radius = radius,
            velocity = velocity,
            density = density,
            randomness = randomness,
            material = material,
        })
    end)

    net.Receive(gellymp.MESH_EMITTER_NETMSG, function(_, _)
        local entity = net.ReadEntity()
        local density = net.ReadFloat()
        local material = net.ReadTable()
        gellyx.emitters.Mesh({
            entity = entity,
            density = density,
            material = material,
        })
    end)

    net.Receive(gellymp.RESET_NETMSG, function(_, _)
        gelly.Reset()
    end)

    function gellymp.Reset()
        net.Start(gellymp.RESET_CLIENT_NETMSG)
        net.SendToServer()
    end
end