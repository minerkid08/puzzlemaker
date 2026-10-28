#include "texture.h"
#include "error.h"
#include "glad/glad.h"
#include <string.h>

#include "packages/package.h"
#include "stb/stbimage.h"

#ifdef __linux__
__attribute__((constructor)) static void textureInit()
#else
void textureInit()
#endif
{
	stbi_set_flip_vertically_on_load(1);
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*textureInitPtr)(void) = textureInit;

#pragma comment(linker, "/include:textureInitPtr")
#endif

unsigned int loadTexture(Package* package, const char* filename, char* flags)
{
	int l = strlen(filename) + 18;
	char* buf = malloc(l);

	snprintf(buf, l, "assets/materials/%s\n", filename);
	buf[l - 1] = 0;

	int channels;
	int width;
	int height;
	unsigned char* data;
	if (package == 0)
		data = stbi_load(filename, &width, &height, &channels, 0);
	else
	{
		int dataLen;
		unsigned char* file = readPackageFile(package, buf, &dataLen);

		data = stbi_load_from_memory(file, dataLen, &width, &height, &channels, 0);
		free(file);
	}
	if (data == 0)
	{
		printf("failed to load image: '%s'\n", buf);
		free(buf);
		return 0;
	}

	if (flags)
	{
		if (channels == 4)
		{
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					unsigned long long index = (y * width + x) * 4 + 3;
					unsigned char a = data[index];
					if (a == 255)
						*flags |= TEX_OPAQUE;
					if (a != 0 && a != 255)
						*flags |= TEX_TRANSPARENT;
				}
			}
		}
		else
			*flags = TEX_OPAQUE;
	}

	int format;
	int intFormat = GL_RGBA8;
	switch (channels)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		errorf("invalid number of channels: got %d, expected 1 or 3 or 4\n");
	}

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
