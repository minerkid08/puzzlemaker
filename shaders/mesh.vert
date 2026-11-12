#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in int matId;

uniform mat4 mat;
uniform mat4 cam;
uniform mat4 trans;

out VSOUT
{
	vec2 uv;
	flat int materialId;
} vertOut;

void main()
{
	vertOut.materialId = matId;
  vertOut.uv = uv;
  gl_Position = mat * cam * trans * vec4(pos, 1);
}
