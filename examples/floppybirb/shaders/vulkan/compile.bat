call cleanup.bat

C:/VulkanSDK/1.3.261.1/Bin/glslc.exe textrenderer.vert -o textrenderer_vert.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe textrenderer.frag -o textrenderer_frag.spv

C:/VulkanSDK/1.3.261.1/Bin/glslc.exe axisrenderer.vert -o axisrenderer_vert.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe axisrenderer.frag -o axisrenderer_frag.spv

C:/VulkanSDK/1.3.261.1/Bin/glslc.exe meshrenderer.vert -o meshrenderer_vert.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe meshrenderer.frag -o meshrenderer_frag.spv

C:/VulkanSDK/1.3.261.1/Bin/glslc.exe default.vert -o default_vert.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe default.frag -o default_frag.spv

pause