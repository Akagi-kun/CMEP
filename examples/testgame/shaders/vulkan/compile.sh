#!/bin/bash

$VULKAN_SDK/bin/glslc text_vert.glsl -o text_vert.spv
$VULKAN_SDK/bin/glslc text_frag.glsl -o text_frag.spv

$VULKAN_SDK/bin/glslc axis_vert.glsl -o axis_vert.spv
$VULKAN_SDK/bin/glslc axis_frag.glsl -o axis_frag.spv

$VULKAN_SDK/bin/glslc mesh_vert.glsl -o mesh_vert.spv
$VULKAN_SDK/bin/glslc mesh_frag.glsl -o mesh_frag.spv

$VULKAN_SDK/bin/glslc sprite_vert.glsl -o sprite_vert.spv
$VULKAN_SDK/bin/glslc sprite_frag.glsl -o sprite_frag.spv
