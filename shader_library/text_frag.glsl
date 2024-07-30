#version 450
#pragma shader_stage(fragment)

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 finalColor = vec4(fragColor.rgb, texture(texSampler, fragTexCoord).a);

    if(finalColor.a < 0.2)
    {
        discard;
    }

    outColor = finalColor;
}