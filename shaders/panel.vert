#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 tint;
layout(location = 3) in int matId;

uniform mat4 mat;
uniform mat4 cam;
uniform mat4 transform;

out vec2 iuv;
flat out int imat;

void main()
{
  imat = matId;
  iuv = uv;
  gl_Position = mat * cam * transform * pos;
}
