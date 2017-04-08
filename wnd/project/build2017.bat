setlocal
pushd %~dp0
set MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"


call :CLEAN "windows/VS2017/wnd.sln"

call :BUILD "windows/VS2017/wnd.sln"

goto :END

:CLEAN
%MSBUILD% %1 /t:clean /p:Configuration=Debug;Platform=x86
%MSBUILD% %1 /t:clean /p:Configuration=Debug;Platform=x64
%MSBUILD% %1 /t:clean /p:Configuration=Release;Platform=x86
%MSBUILD% %1 /t:clean /p:Configuration=Release;Platform=x64
exit /b

:BUILD
%MSBUILD% %1 /t:rebuild /p:Configuration=Debug;Platform=x86 /m
%MSBUILD% %1 /t:rebuild /p:Configuration=Debug;Platform=x64 /m
%MSBUILD% %1 /t:rebuild /p:Configuration=Release;Platform=x86 /m
%MSBUILD% %1 /t:rebuild /p:Configuration=Release;Platform=x64 /m
exit /b

:END
popd
