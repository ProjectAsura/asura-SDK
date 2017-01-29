setlocal
set SHADER_COMPILER=%VK_SDK_PATH%\Bin\glslangValidator.exe
%SHADER_COMPILER% -V -l -o simpleQuadVS.spv simpleQuadVS.vert
%SHADER_COMPILER% -V -l -o simpleQuadPS.spv simpleQuadPS.frag

%SHADER_COMPILER% -V -l -o simplePS.spv simplePS.frag
%SHADER_COMPILER% -V -l -o simpleVS.spv simpleVS.vert

%SHADER_COMPILER% -V -l -o simpleTexPS.spv simpleTexPS.frag
%SHADER_COMPILER% -V -l -o simpleTexVS.spv simpleTexVS.vert

%SHADER_COMPILER% -V -l -o imguiVS.spv imguiVS.vert
%SHADER_COMPILER% -V -l -o imguiPS.spv imguiPS.frag

endlocal
