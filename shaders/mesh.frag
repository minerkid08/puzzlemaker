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

uniform int pass;

in VSOUT
{
  vec2 uv;
  flat int materialId;
} fragIn;

out vec4 color;

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

void main()
{
  Material mat = materials[fragIn.materialId];
  if (mat.texIndex == 0)
    color = mat.tint;
  else
  {
		color = getColor(mat.texIndex);
    color = color * mat.tint;
  }

  if (mat.alphaTex != 0)
  {
		color.a = getColor(mat.alphaTex).a;
    color.a *= mat.tint.a;
  }

  if (color.a <= 0)
    discard;
  if (pass == 0)
  {
    if (color.a < 1)
      discard;
  }
  else
  {
    if (color.a >= 1)
      discard;
  }
}
