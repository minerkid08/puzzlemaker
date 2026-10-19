#version 330 core

uniform sampler2D tex;
uniform int pass;

in vec2 iuv;

out vec4 color;

void main()
{
  color = texture(tex, iuv);
	if(pass == 0)
	{
		if(color.a != 1)
			discard;
	}
	else
	{
		if(color.a == 1)
			discard;
	}
}
