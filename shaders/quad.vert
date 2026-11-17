#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 tint;
layout(location = 4) in int matId;

uniform mat4 mat;
uniform mat4 cam;

out vec3 fragPos;
out vec2 iuv;
out vec4 itint;
out vec3 inorm;
flat out int imatId;

void main()
{
  imatId = matId;
  iuv = uv;
  itint = tint;
	inorm = norm;
	fragPos = pos;
  gl_Position = mat * cam * vec4(pos, 1);
}
