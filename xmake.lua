add_rules("mode.debug", "mode.release")
set_defaultmode("debug")
set_toolchains("msvc")
set_languages("c99", "cxx17")
set_encodings("utf-8")

if is_os("windows") then 
    add_defines("UNICODE")
    add_defines("_UNICODE")
end

function add_dx_sdk_options()
    if has_config("WIN7_SYSTEM_SUPPORT") then
        add_defines("_WIN32_WINNT=0x601")
    end
    add_syslinks("d3d11","dxgi","dxguid","D3DCompiler","d2d1","dwrite","winmm","user32","gdi32","ole32")
end

includes("scripts.lua")
--ImGui
includes("ImGui")
-- Assimp
add_requires("assimp",{system = false })

-- ////////////////
targetName = "HelloDX"
target(targetName)
    set_kind("binary")
    set_targetdir(path.join(binDir,targetName))
    set_rundir("$(projectdir)")
    add_dx_sdk_options()
    add_deps("ImGui")
    add_headerfiles("**.h")
    add_files("**.cpp")
    -- Shader
    add_rules("hlsl_shader_complier")
    add_headerfiles("HLSL/**.hlsl|HLSL/**.hlsli")
    add_headerfiles("HLSL/**/**.hlsl|HLSL/**/**.hlsli")
    add_files("HLSL/**.hlsl|HLSL/**.hlsli")
    add_files("HLSL/**/**.hlsl|HLSL/**/**.hlsli")

target_end()
-- ////////////////
