#include "texture.h"
#include "glad/glad.h"
#include <string.h>

#include "stb/stbimage.h"

static __attribute__((constructor)) void init()
{
	stbi_set_flip_vertically_on_load(1);
}

unsigned int loadTexture(const char* filename)
{
	int l = strlen(filename) + 18;
	char* buf = malloc(l);

	snprintf(buf, l, "assets/materials/%s\n", filename);
	buf[l - 1] = 0;

	int channels;
	int width;
	int height;
	unsigned char* data = stbi_load(buf, &width, &height, &channels, 0);
	if (data == 0)
	{
		printf("failed to load image: '%s'\n", buf);
		free(buf);
		return 0;
	}

	int format = (channels == 4 ? GL_RGBA : GL_RGB);
	int intFormat = (channels == 4 ? GL_RGBA8 : GL_RGBA8);
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	free(buf);
	return id;
}
