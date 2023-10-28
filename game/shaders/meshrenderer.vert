#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 ambient;
layout(location = 4) in vec3 diffuse;
layout(location = 5) in vec3 specular;
layout(location = 6) in uint matid;
layout(location = 7) in float dissolve;
layout(location = 8) in vec3 emission;
layout(location = 9) in vec3 tangent;
layout(location = 10) in vec3 bitangent;

out vec4 Position_worldspace;
out vec4 fragPosition;
out vec2 fragUV;
out vec3 fragNormal;

out float cosTheta;
out vec3 LightDirection_tangentspace;
out vec3 EyeDirection_tangentspace;
out vec3 LightDirection_cameraspace;
out vec3 EyeDirection_cameraspace;

out vec3 fragAmbient;
out vec3 fragDiffuse;
out vec3 fragSpecular;
out uint fragMatid;
out float fragDissolve;
out vec3 fragEmission;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec3 fragLight;

uniform mat4 matV;
uniform mat4 matM;
uniform mat4 matMV;
uniform mat3 matMV3x3;
uniform mat4 matMVP;

uniform vec3 Light;

void main() 
{ 
    gl_Position = matMVP * vec4(pos, 1.0);
    fragPosition = matMVP * vec4(pos, 1.0);

    fragUV = uv;
    fragNormal = normal;

    vec3 normal_cameraspace = matMV3x3 * normalize(normal);
    vec3 tangent_cameraspace = matMV3x3 * normalize(tangent);
    vec3 bitangent_cameraspace = matMV3x3 * normalize(bitangent);

    mat3 matTBN = transpose(mat3(
        tangent_cameraspace,
        bitangent_cameraspace,
        normal_cameraspace
    ));

	vec3 vertexPosition_cameraspace = ( matV * matM * vec4(pos, 1)).xyz;
    EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

    vec3 LightPosition_cameraspace = ( matV * vec4(Light, 1)).xyz;   
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

	EyeDirection_tangentspace =  matTBN * EyeDirection_cameraspace;
	LightDirection_tangentspace = matTBN * LightDirection_cameraspace;
	
    Position_worldspace = matM * vec4(pos, 1.0);

    cosTheta = dot(normal, Light);

    fragLight = Light;

    fragAmbient = ambient;
    fragDiffuse = diffuse;
    fragSpecular = specular;
    fragMatid = matid;
    fragDissolve = dissolve;
    fragEmission = emission;

    fragTangent = tangent;
    fragBitangent = bitangent;
}