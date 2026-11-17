#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in int matId;

uniform mat4 mat;
uniform mat4 cam;
uniform mat4 trans;

out VSOUT
{
	vec3 pos;
	vec2 uv;
	vec3 normal;
	flat int materialId;
} vertOut;

void main()
{
	vec4 p = trans * vec4(pos, 1);
	vertOut.pos = vec3(p.x, p.y, p.z);
	vertOut.normal = normal;
	vertOut.materialId = matId;
  vertOut.uv = uv;
  gl_Position = mat * cam * p;
}
