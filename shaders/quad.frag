#version 330 core

uniform sampler2D textures[16];
uniform int pass;

in vec2 iuv;
in vec4 itint;
flat in int imatId;

out vec4 color;

void main()
{
	switch (imatId)
  {
		case 0:  color = texture(textures[0],  iuv); break;
		case 1:  color = texture(textures[1],  iuv); break;
		case 2:  color = texture(textures[2],  iuv); break;
		case 3:  color = texture(textures[3],  iuv); break;
		case 4:  color = texture(textures[4],  iuv); break;
		case 5:  color = texture(textures[5],  iuv); break;
		case 6:  color = texture(textures[6],  iuv); break;
		case 7:  color = texture(textures[7],  iuv); break;
		case 8:  color = texture(textures[8],  iuv); break;
		case 9:  color = texture(textures[9],  iuv); break;
		case 10:  color = texture(textures[10],  iuv); break;
		case 11:  color = texture(textures[11],  iuv); break;
		case 12:  color = texture(textures[12],  iuv); break;
		case 13:  color = texture(textures[13],  iuv); break;
		case 14:  color = texture(textures[14],  iuv); break;
		case 15:  color = texture(textures[15],  iuv); break;
	}
	color *= itint;
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
