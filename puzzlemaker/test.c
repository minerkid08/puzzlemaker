#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb/stbimage.h>
#include <stdio.h>
#include <stdlib.h>

#define glErrCheck()                                                           \
  {                                                                            \
    int error = glGetError();                                                  \
    while (error) {                                                            \
      printf("gl error: %d, %d\n", __LINE__, error);                           \
      error = glGetError();                                                    \
    }\
    }                                                                          

int makeShader(int type, const char* src)
{
	int id = glCreateShader(type);
	glShaderSource(id, 1, &src, 0);
	glCompileShader(id);
	int state;
	glGetShaderiv(id, GL_COMPILE_STATUS, &state);
	if (state == GL_FALSE)
	{
		int len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		char* err = malloc(len);
		glGetShaderInfoLog(id, len, &len, err);
		printf("shader err: %s\n", err);
		free(err);
		glDeleteShader(id);
		return -1;
	}
	return id;
}

const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
out vec2 iuv;
void main()
{
  iuv = uv;
  gl_Position = vec4(pos, 1);
}
)";

const char* fragSrc = R"(
#version 330 core
uniform sampler2D tex;
in vec2 iuv;
out vec4 color;
void main()
{
  color = texture(tex, iuv);
}
)";

int makeProgram()
{
	int vert = makeShader(GL_VERTEX_SHADER, vertSrc);
	int frag = makeShader(GL_FRAGMENT_SHADER, fragSrc);

	int prgmId = glCreateProgram();
	glAttachShader(prgmId, vert);
	glAttachShader(prgmId, frag);
	glLinkProgram(prgmId);

	int status;
	glGetProgramiv(prgmId, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("shader linking failed\n");
		glDeleteShader(vert);
		glDeleteShader(frag);
		exit(1);
	}
	return prgmId;
}

int loadTexture()
{
	int channels;
	int width;
	int height;
	unsigned char* data = stbi_load("wall.png", &width, &height, &channels, 0);
	if (data == 0)
	{
		printf("failed to load image: '%s'\n", "wall.png");
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

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(2048, 2048, "source -1", 0, 0);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	int prgmid = makeProgram();

	unsigned int va;
	unsigned int vb;
	unsigned int ib;

	glCreateVertexArrays(1, &va);
	glBindVertexArray(va);

	glCreateBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 5, 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	unsigned int inds[] = {0, 1, 2, 2, 1, 3};

	glCreateBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, inds, GL_STATIC_DRAW);

  glUseProgram(prgmid);

  int texture = loadTexture();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  int loc = glGetUniformLocation(prgmid, "tex");
  if(loc == -1)
    printf("uniform no exist\n");
  glUniform1i(loc, 0);

	int error = glGetError();
	while (error)
	{
		printf("gl error: %d\n", error);
		error = glGetError();
	}

	float verts[] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f, 1.0f
	};

	glClearColor(0.8, 0.8, 0.8, 1);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glBindBuffer(GL_ARRAY_BUFFER, vb);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 4 * 5, verts);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
		glUseProgram(prgmid);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
