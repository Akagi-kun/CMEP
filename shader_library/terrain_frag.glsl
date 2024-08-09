#version 450
#pragma shader_stage(fragment)

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 finalColor = texture(texSampler, fragTexCoord).rgba;

    if(finalColor.a < 0.2)
    {
        discard;
    }

	if(fragNormal.y > 0)
	{
		finalColor *= 1.2;
	}

    outColor = finalColor;
}