setlocal
set SHADER_COMPILER=%VK_SDK_PATH%\Bin\glslangValidator.exe
%SHADER_COMPILER% -V -l -o simpleVS.spv simpleVS.vert
%SHADER_COMPILER% -V -l -o simplePS.spv simplePS.frag
%SHADER_COMPILER% -V -l -o imguiVS.spv imguiVS.vert
%SHADER_COMPILER% -V -l -o imguiPS.spv imguiPS.frag
%SHADER_COMPILER% -V -l -o colorVS.spv colorVS.vert
%SHADER_COMPILER% -V -l -o colorPS.spv colorPS.frag
endlocal
