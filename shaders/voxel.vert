#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 tint;
layout(location = 3) in int matId;

uniform mat4 mat;
uniform mat4 cam;

out vec2 iuv;
out vec4 itint;
flat out int imatId;

void main()
{
  imatId = matId;
  iuv = uv;
  itint = tint;
  gl_Position = mat * cam * pos;
}
