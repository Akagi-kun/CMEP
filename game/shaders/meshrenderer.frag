#version 460 core

out vec4 color;

in vec4 fragPosition;
in vec2 fragUV;
in vec3 fragNormal;

in float cosTheta;
in vec4 Position_worldspace;
in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

in vec3 fragAmbient;
in vec3 fragDiffuse;
in vec3 fragSpecular;
flat in uint fragMatid;
in float fragDissolve;
in vec3 fragEmission;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec3 fragLight;

uniform sampler2D textureDiffuse[16];
uniform int used_textureDiffuse[16];

uniform sampler2D textureNormal0;
uniform sampler2D textureNormal1;
uniform sampler2D textureNormal2;
uniform sampler2D textureNormal3;
uniform sampler2D textureNormal4;
uniform sampler2D textureNormal5;
uniform sampler2D textureNormal6;
uniform sampler2D textureNormal7;
uniform sampler2D textureNormal8;
uniform sampler2D textureNormal9;
uniform sampler2D textureNormal10;
uniform sampler2D textureNormal11;
uniform sampler2D textureNormal12;
uniform sampler2D textureNormal13;
uniform sampler2D textureNormal14;
uniform sampler2D textureNormal15;

uniform int used_textureNormal0;
uniform int used_textureNormal1;
uniform int used_textureNormal2;
uniform int used_textureNormal3;
uniform int used_textureNormal4;
uniform int used_textureNormal5;
uniform int used_textureNormal6;
uniform int used_textureNormal7;
uniform int used_textureNormal8;
uniform int used_textureNormal9;
uniform int used_textureNormal10;
uniform int used_textureNormal11;
uniform int used_textureNormal12;
uniform int used_textureNormal13;
uniform int used_textureNormal14;
uniform int used_textureNormal15;

vec3 getTextureDiffuseOrDiffuse(vec3 textureDiffuse, int useTexture)
{
    vec3 colorResult;

    if(useTexture == 1)
    {
        colorResult = textureDiffuse;
    }
    else
    {
        colorResult = vec3(normalize(fragDiffuse));
    }

    return colorResult;
}

vec3 getTextureNormalOrNormal(vec3 textureNormal, int useTexture)
{
    vec3 normalResult;

    if(useTexture == 1)
    {
        normalResult = normalize(textureNormal.rgb * 2.0 - 1.0);
    }
    else
    {
        normalResult = vec3(fragNormal);
    }

    return normalResult;
}
void main()
{
    vec3 colorResult = vec3(normalize(fragDiffuse));
    vec3 normalResult = fragNormal;

    vec3 diffuseResult[16];

    for(int i = 0; i < 16; i++)
    {
        diffuseResult[i] = texture(textureDiffuse[i], fragUV).rgb; 
    }
/*
    vec3 normalResult0 = texture(textureNormal0, fragUV).rgb;
    vec3 normalResult1 = texture(textureNormal1, fragUV).rgb;
    vec3 normalResult2 = texture(textureNormal2, fragUV).rgb;
    vec3 normalResult3 = texture(textureNormal3, fragUV).rgb;
    vec3 normalResult4 = texture(textureNormal4, fragUV).rgb;
    vec3 normalResult5 = texture(textureNormal5, fragUV).rgb;
    vec3 normalResult6 = texture(textureNormal6, fragUV).rgb;
    vec3 normalResult7 = texture(textureNormal7, fragUV).rgb;
    vec3 normalResult8 = texture(textureNormal8, fragUV).rgb;
    vec3 normalResult9 = texture(textureNormal9, fragUV).rgb;
    vec3 normalResult10 = texture(textureNormal10, fragUV).rgb;
    vec3 normalResult11 = texture(textureNormal11, fragUV).rgb;
    vec3 normalResult12 = texture(textureNormal12, fragUV).rgb;
    vec3 normalResult13 = texture(textureNormal13, fragUV).rgb;
    vec3 normalResult14 = texture(textureNormal14, fragUV).rgb;
    vec3 normalResult15 = texture(textureNormal15, fragUV).rgb;
*/
    colorResult = getTextureDiffuseOrDiffuse(diffuseResult[fragMatid], used_textureDiffuse[fragMatid]).rgb;
/* 
    switch(fragMatid)
    {
        case 0:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[0], used_textureDiffuse[0]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult0, used_textureNormal0).rgb;
            break;
        case 1:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[1], used_textureDiffuse[1]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult1, used_textureNormal1).rgb;
            break;
        case 2:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[2], used_textureDiffuse[2]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult2, used_textureNormal2).rgb;
            break;
        case 3:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[3], used_textureDiffuse[3]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult3, used_textureNormal3).rgb;
            break;
        case 4:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[4], used_textureDiffuse[4]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult4, used_textureNormal4).rgb;
            break;
        case 5:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[5], used_textureDiffuse[5]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult5, used_textureNormal5).rgb;
            break;
        case 6:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[6], used_textureDiffuse[6]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult6, used_textureNormal6).rgb;
            break;
        case 7:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[7], used_textureDiffuse[7]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult7, used_textureNormal7).rgb;
            break;
        case 8:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[8], used_textureDiffuse[8]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult8, used_textureNormal8).rgb;
            break;
        case 9:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[9], used_textureDiffuse[9]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult9, used_textureNormal9).rgb;
            break;
        case 10:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[10], used_textureDiffuse[10]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult10, used_textureNormal10).rgb;
            break;
        case 11:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[11], used_textureDiffuse[11]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult11, used_textureNormal11).rgb;
            break;
        case 12:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[12], used_textureDiffuse[12]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult12, used_textureNormal12).rgb;
            break;
        case 13:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[13], used_textureDiffuse[13]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult13, used_textureNormal13).rgb;
            break;
        case 14:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[14], used_textureDiffuse[14]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult14, used_textureNormal14).rgb;
            break;
        case 15:
            colorResult = getTextureDiffuseOrDiffuse(diffuseResult[15], used_textureDiffuse[15]).rgb;
            normalResult = getTextureNormalOrNormal(normalResult15, used_textureNormal15).rgb;
            break;
    }
     */
    float cosThetaMod = cosTheta;

    //vec3 TextureNormal_tangentspace = normalize(normalResult * 2.0 - 1.0);
    //cosThetaMod = clamp(dot(TextureNormal_tangentspace, fragLight), 0, 1);

    float alpha = 1.0;
    if(fragDissolve < 0.9)
    {
        alpha = 0.1;
    }

    color = vec4((colorResult + fragAmbient * 0.1 + fragSpecular * 0.3) * (cosThetaMod), alpha);
    //color = vec4(normalResult, alpha);

    if(fragEmission.r > 0.0 || fragEmission.g > 0.0 || fragEmission.b > 0.0)
    {
        color = vec4(fragEmission, alpha);
    }
}
