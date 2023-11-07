#!/bin/bash

$VULKAN_SDK/bin/glslc textrenderer.vert -o textrenderer_vert.spv
$VULKAN_SDK/bin/glslc textrenderer.frag -o textrenderer_frag.spv

$VULKAN_SDK/bin/glslc axisrenderer.vert -o axisrenderer_vert.spv
$VULKAN_SDK/bin/glslc axisrenderer.frag -o axisrenderer_frag.spv

$VULKAN_SDK/bin/glslc meshrenderer.vert -o meshrenderer_vert.spv
$VULKAN_SDK/bin/glslc meshrenderer.frag -o meshrenderer_frag.spv

$VULKAN_SDK/bin/glslc default.vert -o default_vert.spv
$VULKAN_SDK/bin/glslc default.frag -o default_frag.spv
