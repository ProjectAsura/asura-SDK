setlocal
set SHADER_COMPILER="C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\fxc.exe"

%SHADER_COMPILER% /E main /T vs_4_0 /Fo simpleQuadVS.cso simpleQuadVS.hlsl
%SHADER_COMPILER% /E main /T ps_4_0 /Fo simpleQuadPS.cso simpleQuadPS.hlsl

%SHADER_COMPILER% /E main /T vs_4_0 /Fo simpleVS.cso simpleVS.hlsl
%SHADER_COMPILER% /E main /T ps_4_0 /Fo simplePS.cso simplePS.hlsl

%SHADER_COMPILER% /E main /T vs_4_0 /Fo simpleTexVS.cso simpleTexVS.hlsl
%SHADER_COMPILER% /E main /T ps_4_0 /Fo simpleTexPS.cso simpleTexPS.hlsl

%SHADER_COMPILER% /E main /T vs_4_0 /Fo imguiVS.cso imguiVS.hlsl
%SHADER_COMPILER% /E main /T ps_4_0 /Fo imguiPS.cso imguiPS.hlsl

endlocal
