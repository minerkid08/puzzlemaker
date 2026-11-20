#version 330 core

uniform sampler2D textures[8];

struct Material
{
  int texIndex;
  int alphaTex;
  vec4 tint;
};

layout(std140) uniform MaterialBlock
{
  Material materials[8];
};

in VSOUT
{
  vec2 uv;
  flat int materialId;
} fragIn;

vec4 getColor(int tex)
{
  switch (tex)
  {
    case 1:
    return texture(textures[0], fragIn.uv);
    case 2:
    return texture(textures[1], fragIn.uv);
    case 3:
    return texture(textures[2], fragIn.uv);
    case 4:
    return texture(textures[3], fragIn.uv);
    case 5:
    return texture(textures[4], fragIn.uv);
    case 6:
    return texture(textures[5], fragIn.uv);
    case 7:
    return texture(textures[6], fragIn.uv);
    case 8:
    return texture(textures[7], fragIn.uv);
  }
  return vec4(1, 1, 1, 1);
}

out vec4 color;

void main()
{
  Material mat = materials[fragIn.materialId];
  vec4 texColor = vec4(0, 0, 0, 1);
  if (mat.texIndex == 0)
    texColor = mat.tint;
  else
  {
    texColor = getColor(mat.texIndex);
    texColor = texColor * mat.tint;
  }

  if (mat.alphaTex != 0)
  {
    texColor.a = getColor(mat.alphaTex).a;
    texColor.a *= mat.tint.a;
  }

  if (texColor.a < 1)
    discard;

  float nearPlane = 0.1;
  float farPlane = 20;
  float z = gl_FragCoord.z * 2.0 - 1.0;
  color = vec4((2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane)) / farPlane);
  color.a = 1;
}
