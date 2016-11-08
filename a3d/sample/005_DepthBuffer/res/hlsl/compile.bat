setlocal
set SHADER_COMPILER="C:\Program Files (x86)\Windows Kits\10\bin\x64\fxc.exe"
%SHADER_COMPILER% /E main /T vs_4_0 /Fo simpleVS.cso simpleVS.hlsl
%SHADER_COMPILER% /E main /T ps_4_0 /Fo simplePS.cso simplePS.hlsl
endlocal
