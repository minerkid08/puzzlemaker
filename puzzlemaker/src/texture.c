#include "texture.h"
#include "glad/glad.h"
#include "stb/stbimage.h"

unsigned int loadTexture(const char* filename)
{
	int channels;
	int width;
	int height;
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
	if (data == 0)
	{
		printf("failed to load image: '%s'\n", filename);
		return 0;
	}
	int format = (channels == 4 ? GL_RGBA : GL_RGB);
	int intFormat = (channels == 4 ? GL_RGBA8 : GL_RGBA8);
	unsigned int id;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTextureStorage2D(id, 1, (channels == 4 ? GL_RGBA8 : GL_RGB8), width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureSubImage2D(id, 0, 0, 0, width, height, (channels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
  return id;
}
