targetName = "src"
target(targetName)
    set_group("Project")
    set_kind("binary")
    set_targetdir(path.join(binDir,targetName))
    add_dx_sdk_options()
    add_deps("Common")
    add_headerfiles("**.h")
    add_files("**.cpp")
    -- shader
    add_rules("hlsl_shader_copy")
    add_headerfiles("Shaders/**.hlsl|Shaders/**.hlsli")
    add_files("Shaders/**.hlsl")
    add_files("Shaders/**.hlsli")
    -- assert
    add_rules("asset_file")
target_end() 