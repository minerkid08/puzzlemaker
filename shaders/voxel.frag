#version 330 core

uniform sampler2D whiteTex;
uniform sampler2D blackTex;

in vec2 iuv;
in vec4 itint;
flat in int imatId;

out vec4 color;

void main()
{
  if(imatId == 0)
    color = texture(blackTex, iuv) * itint;
  else
    color = texture(whiteTex, iuv) * itint;
}
