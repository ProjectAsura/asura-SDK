set DEVENV="C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe"
%DEVENV% "d3d12/a3d.sln" /clean "Debug|x86"
%DEVENV% "d3d12/a3d.sln" /clean "Debug|x64"
%DEVENV% "d3d12/a3d.sln" /clean "Release|x86"
%DEVENV% "d3d12/a3d.sln" /clean "Release|x64"
%DEVENV% "vulkan/a3d.sln" /clean "Debug|x86"
%DEVENV% "vulkan/a3d.sln" /clean "Debug|x64"
%DEVENV% "vulkan/a3d.sln" /clean "Release|x86"
%DEVENV% "vulkan/a3d.sln" /clean "Release|x64"

%DEVENV% "d3d12/a3d.sln" /rebuild "Debug|x86"
%DEVENV% "d3d12/a3d.sln" /rebuild "Debug|x64"
%DEVENV% "d3d12/a3d.sln" /rebuild "Release|x86"
%DEVENV% "d3d12/a3d.sln" /rebuild "Release|x64"
%DEVENV% "vulkan/a3d.sln" /rebuild "Debug|x86"
%DEVENV% "vulkan/a3d.sln" /rebuild "Debug|x64"
%DEVENV% "vulkan/a3d.sln" /rebuild "Release|x86"
%DEVENV% "vulkan/a3d.sln" /rebuild "Release|x64"