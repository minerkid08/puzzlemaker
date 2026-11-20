#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 tint;
layout(location = 4) in int matId;

uniform mat4 mat;
uniform mat4 cam;

out vec2 iuv;
flat out int imatId;

void main()
{
  imatId = matId;
  iuv = uv;
  gl_Position = mat * cam * vec4(pos, 1);
}
