#version 330 core

uniform sampler2D whiteTex;
uniform sampler2D blackTex;
uniform sampler2D whiteMiniTex;
uniform sampler2D blackMiniTex;

in vec2 iuv;
in vec4 itint;
flat in int imatId;

out vec4 color;

void main()
{
	color = vec4(1, 0, 0, 1);
	if(imatId == 0)
    color = texture(blackTex, iuv) * itint;
	else if(imatId == 1)
    color = texture(whiteTex, iuv) * itint;
	else if(imatId == 2)
    color = texture(blackMiniTex, iuv) * itint;
	else if(imatId == 3)
    color = texture(whiteMiniTex, iuv) * itint;
}
