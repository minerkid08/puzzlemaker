#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 mat;
uniform mat4 cam;

void main()
{
  gl_Position = mat * cam * vec4(pos, 1);
}
