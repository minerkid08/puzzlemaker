#version 330 core

uniform sampler2D textures[10];

in vec2 iuv;
flat in int imat;

out vec4 color;

void main()
{
	switch (imat)
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
	}
}
