#version 330 core

uniform sampler2D textures[4];

struct Material
{
	int texIndex;
	vec4 tint;
};

layout(std140) uniform MaterialBlock
{
	Material materials[4];
};

uniform int pass;

in vec2 iuv;
flat in int materialId;

out vec4 color;

void main()
{
	int textureIndex = materials[materialId].texIndex;
	if(textureIndex == 0)
		color = materials[materialId].tint;
	else
	{
		switch(textureIndex)
		{
			case 1:
				color = texture(textures[0], iuv);
				break;
			case 2:
				color = texture(textures[1], iuv);
				break;
			case 3:
				color = texture(textures[2], iuv);
				break;
			case 4:
				color = texture(textures[3], iuv);
				break;
		}
		color = color * materials[materialId].tint;
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
}
