#version 330 core

uniform sampler2D tex;

in vec2 iuv;
in vec4 itint;

out vec4 color;

void main()
{
  color = texture(tex, iuv) * itint;
}
