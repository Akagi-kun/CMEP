%VULKAN_SDK%/Bin/glslc.exe text_vert.glsl -o text_vert.spv
%VULKAN_SDK%/Bin/glslc.exe text_frag.glsl -o text_frag.spv

%VULKAN_SDK%/Bin/glslc.exe axis_vert.glsl -o axis_vert.spv
%VULKAN_SDK%/Bin/glslc.exe axis_frag.glsl -o axis_frag.spv

%VULKAN_SDK%/Bin/glslc.exe mesh_vert.glsl -o mesh_vert.spv
%VULKAN_SDK%/Bin/glslc.exe mesh_frag.glsl -o mesh_frag.spv

%VULKAN_SDK%/Bin/glslc.exe sprite_vert.glsl -o sprite_vert.spv
%VULKAN_SDK%/Bin/glslc.exe sprite_frag.glsl -o sprite_frag.spv

pause