if is_mode("debug") then
    binDir = path.join(os.projectdir(),"bin/Debug/Project")
else 
    binDir = path.join(os.projectdir(),"bin/Release/Project")
end
includes("Common")
includes("src")