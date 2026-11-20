#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 tint;
layout(location = 4) in int matId;

uniform mat4 mat;
uniform mat4 cam;

out VSOUT
{
  vec3 pos;
  vec2 uv;
  vec4 tint;
  vec3 normal;
  flat int materialId;
} vertOut;

void main()
{
  vertOut.pos = pos;
  vertOut.uv = uv;
  vertOut.tint = tint;
  vertOut.normal = norm;
  vertOut.materialId = matId;
  gl_Position = mat * cam * vec4(pos, 1);
}
