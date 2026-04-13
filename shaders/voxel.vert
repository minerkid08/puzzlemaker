#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 tint;

uniform mat4 mat;
uniform mat4 cam;

out vec2 iuv;
out vec4 itint;

void main()
{
  iuv = uv;
  itint = tint;
  gl_Position = mat * cam * pos;
}
