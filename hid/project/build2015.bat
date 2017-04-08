setlocal
pushd %~dp0
set MSBUILD="C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe"

call :CLEAN "windows/VS2015/hid.sln"

call :BUILD "windows/VS2015/hid.sln"

goto :END

:CLEAN
%MSBUILD% %1 /t:clean /p:Configuration=Debug;Platform=x86
%MSBUILD% %1 /t:clean /p:Configuration=Debug;Platform=x64
%MSBUILD% %1 /t:clean /p:Configuration=Release;Platform=x86
%MSBUILD% %1 /t:clean /p:Configuration=Release;Platform=x64
exit /b

:BUILD
%MSBUILD% %1 /t:build /p:Configuration=Debug;Platform=x86 /m
%MSBUILD% %1 /t:build /p:Configuration=Debug;Platform=x64 /m
%MSBUILD% %1 /t:build /p:Configuration=Release;Platform=x86 /m
%MSBUILD% %1 /t:build /p:Configuration=Release;Platform=x64 /m
exit /b

:END
popd

