set MSBUILD="C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe"

%MSBUILD% "d3d11/a3d.sln" /t:clean /p:Configuration=Debug;Platform=x86
%MSBUILD% "d3d11/a3d.sln" /t:clean /p:Configuration=Debug;Platform=x64
%MSBUILD% "d3d11/a3d.sln" /t:clean /p:Configuration=Release;Platform=x86
%MSBUILD% "d3d11/a3d.sln" /t:clean /p:Configuration=Release;Platform=x64
%MSBUILD% "d3d12/a3d.sln" /t:clean /p:Configuration=Debug;Platform=x86
%MSBUILD% "d3d12/a3d.sln" /t:clean /p:Configuration=Debug;Platform=x64
%MSBUILD% "d3d12/a3d.sln" /t:clean /p:Configuration=Release;Platform=x86
%MSBUILD% "d3d12/a3d.sln" /t:clean /p:Configuration=Release;Platform=x64
%MSBUILD% "vulkan/a3d.sln" /t:clean /p:Configuration=Debug;Platform=x86
%MSBUILD% "vulkan/a3d.sln" /t:clean /p:Configuration=Debug;Platform=x64
%MSBUILD% "vulkan/a3d.sln" /t:clean /p:Configuration=Release;Platform=x86
%MSBUILD% "vulkan/a3d.sln" /t:clean /p:Configuration=Release;Platform=x64

%MSBUILD% "d3d11/a3d.sln" /t:rebuild /p:Configuration=Debug;Platform=x86 /m 
%MSBUILD% "d3d11/a3d.sln" /t:rebuild /p:Configuration=Debug;Platform=x64 /m 
%MSBUILD% "d3d11/a3d.sln" /t:rebuild /p:Configuration=Release;Platform=x86 /m 
%MSBUILD% "d3d11/a3d.sln" /t:rebuild /p:Configuration=Release;Platform=x64 /m 
%MSBUILD% "d3d12/a3d.sln" /t:rebuild /p:Configuration=Debug;Platform=x86 /m 
%MSBUILD% "d3d12/a3d.sln" /t:rebuild /p:Configuration=Debug;Platform=x64 /m 
%MSBUILD% "d3d12/a3d.sln" /t:rebuild /p:Configuration=Release;Platform=x86 /m 
%MSBUILD% "d3d12/a3d.sln" /t:rebuild /p:Configuration=Release;Platform=x64 /m 
%MSBUILD% "vulkan/a3d.sln" /t:rebuild /p:Configuration=Debug;Platform=x86 /m 
%MSBUILD% "vulkan/a3d.sln" /t:rebuild /p:Configuration=Debug;Platform=x64 /m 
%MSBUILD% "vulkan/a3d.sln" /t:rebuild /p:Configuration=Release;Platform=x86 /m 
%MSBUILD% "vulkan/a3d.sln" /t:rebuild /p:Configuration=Release;Platform=x64 /m 
