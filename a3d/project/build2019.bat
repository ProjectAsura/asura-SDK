setlocal
pushd %~dp0
set MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"


call :CLEAN "d3d11/VS2019/a3d.sln"
call :CLEAN "d3d12/VS2019/a3d.sln"
call :CLEAN "vulkan/VS2019/a3d.sln"

call :BUILD "d3d11/VS2019/a3d.sln"
call :BUILD "d3d12/VS2019/a3d.sln"
call :BUILD "vulkan/VS2019/a3d.sln"

goto :END

:CLEAN
%MSBUILD% %1 /t:clean /p:Configuration=Debug;Platform=x64
%MSBUILD% %1 /t:clean /p:Configuration=Release;Platform=x64
exit /b

:BUILD
%MSBUILD% %1 /t:rebuild /p:Configuration=Debug;Platform=x64 /m
%MSBUILD% %1 /t:rebuild /p:Configuration=Release;Platform=x64 /m
exit /b

:END
popd
