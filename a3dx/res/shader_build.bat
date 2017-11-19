setlocal
set BIN_CONVERTER="../tool/Bin2Hex/Bin2Hex.exe"
set HLSL_COMPILER="C:\Program Files (x86)\Windows Kits\10\bin\x64\fxc.exe"
set GLSL_COMPILER=%VK_SDK_PATH%\Bin\glslangValidator.exe

mkdir Compiled

%HLSL_COMPILER% /E main /T vs_4_0 /Fo ./Compiled/imguiVS.cso imguiVS.hlsl
%HLSL_COMPILER% /E main /T ps_4_0 /Fo ./Compiled/imguiPS.cso imguiPS.hlsl

%GLSL_COMPILER% -V -l -o ./Compiled/imguiVS.spv imguiVS.vert
%GLSL_COMPILER% -V -l -o ./Compiled/imguiPS.spv imguiPS.frag

%BIN_CONVERTER% -i ./Compiled/imguiVS.cso -o ../src/imguiVS_cso.h
%BIN_CONVERTER% -i ./Compiled/imguiPS.cso -o ../src/imguiPS_cso.h
%BIN_CONVERTER% -i ./Compiled/imguiVS.spv -o ../src/imguiVS_spv.h
%BIN_CONVERTER% -i ./Compiled/imguiPS.spv -o ../src/imguiPS_spv.h

endlocal
