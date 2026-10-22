#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in int matId;

uniform mat4 mat;
uniform mat4 cam;
uniform mat4 trans;

out vec2 iuv;
flat out int materialId;

void main()
{
	materialId = matId;
  iuv = uv;
  gl_Position = mat * cam * trans * vec4(pos, 1);
}
