local Game = {}

-- Animation variables
local time = 0

-- Camera variables
local camera_x = 5
local camera_y = 3
local camera_z = 8
local camera_speed = 0.2
local camera_yaw = 0
local camera_pitch = 0
local mouse_sensitivity = 0.002
local last_mouse_x = 400
local last_mouse_y = 300

-- Initialization
function Game.init()
    print("ConX Engine Demo loaded!")
    print("Use WASD to move camera, Q/E to move up/down")
    print("Hold right mouse button and move to look around")
    
    -- Set 3D mode
    ConX.set_3d_mode(true)
    
    -- Set up initial camera
    ConX.set_camera(camera_x, camera_y, camera_z, 0, 0, 0)
end

-- Input handling function
function handle_input(keys, delta_x, delta_y, right_click)
    -- Mouse look (only when right-clicking)
    if right_click then
        camera_yaw = camera_yaw - delta_x * mouse_sensitivity
        camera_pitch = camera_pitch + delta_y * mouse_sensitivity
        
        -- Clamp pitch to prevent flipping
        if camera_pitch > 1.5 then camera_pitch = 1.5 end
        if camera_pitch < -1.5 then camera_pitch = -1.5 end
    end
    
    -- Calculate forward and right vectors
    local forward_x = -math.sin(camera_yaw) * math.cos(camera_pitch)
    local forward_y = math.sin(camera_pitch)
    local forward_z = -math.cos(camera_yaw) * math.cos(camera_pitch)
    local right_x = math.cos(camera_yaw)
    local right_z = -math.sin(camera_yaw)
    
    -- Forward/backward movement
    if ConX.is_key_pressed(keys, ConX.KEY_W) then
        camera_x = camera_x + forward_x * camera_speed
        camera_z = camera_z + forward_z * camera_speed
    end
    if ConX.is_key_pressed(keys, ConX.KEY_S) then
        camera_x = camera_x - forward_x * camera_speed
        camera_z = camera_z - forward_z * camera_speed
    end
    
    -- Strafe left/right
    if ConX.is_key_pressed(keys, ConX.KEY_A) then
        camera_x = camera_x - right_x * camera_speed
        camera_z = camera_z - right_z * camera_speed
    end
    if ConX.is_key_pressed(keys, ConX.KEY_D) then
        camera_x = camera_x + right_x * camera_speed
        camera_z = camera_z + right_z * camera_speed
    end
    
    -- Up/down movement
    if ConX.is_key_pressed(keys, ConX.KEY_Q) then
        camera_y = camera_y - camera_speed
    end
    if ConX.is_key_pressed(keys, ConX.KEY_E) then
        camera_y = camera_y + camera_speed
    end
    
    -- Calculate look target
    local target_x = camera_x + forward_x
    local target_y = camera_y + forward_y
    local target_z = camera_z + forward_z
    
    -- Update camera
    ConX.set_camera(camera_x, camera_y, camera_z, target_x, target_y, target_z)
end

-- Update function
function Game.update()
    ConX.set_clear_color(0.1, 0.1, 0.2, 1.0)
    ConX.clear_screen()

    -- Update animation
    time = time + 0.016
    local sphere_y = math.sin(time * 2) * 2
    local cube_x = math.cos(time) * 2

    -- Draw animated 3D objects
    ConX.draw_cube(cube_x, 0, 0, 1, 1, 1, 1.0, 0.5, 0.2, 1.0)
    ConX.draw_sphere(-3, sphere_y, 0, 0.8, 0.2, 1.0, 0.5, 1.0)
    
    -- Static 3D objects
    ConX.draw_cube(3, 1, -2, 0.5, 2, 0.5, 0.5, 1.0, 0.5, 1.0)
    ConX.draw_sphere(0, -2, 3, 1.2, 1.0, 1.0, 0.2, 1.0)

    ConX.swap_buffers()
end

return Game