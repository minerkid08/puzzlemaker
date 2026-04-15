#include "cglm/types.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include <glad/glad.h>

static unsigned va;
static unsigned vb;
static unsigned ib;
static unsigned shader;

void initDebug()
{
	shader = makeShader("debug");

	glCreateVertexArrays(1, &va);
	glBindVertexArray(va);

	glCreateBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 8, 0, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

  int inds[] = {0, 1};
  //int inds[] = {2, 3, 1, 0, 4, 5, 7, 6, 6, 7, 3, 2};

	glCreateBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

	glLineWidth(10);
}

void drawDebugRect(vec3 a, vec3 b)
{
  glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glUseProgram(shader);

	setUniformMat4(shader, "cam", *getCamMat());
	setUniformMat4(shader, "mat", *getProjMat());

  vec3 verts[] = {
    {a[0], a[1], a[2]},
    {b[0], a[1], a[2]},
    {a[0], b[1], a[2]},
    {b[0], b[1], a[2]},
    {a[0], a[1], b[2]},
    {b[0], a[1], b[2]},
    {a[0], b[1], b[2]},
    {b[0], b[1], b[2]}
  };

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * 8, verts);

  glDrawElements(GL_QUADS, 12, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}

void drawDebugLine(vec3 a, vec3 b)
{
  glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glUseProgram(shader);

	setUniformMat4(shader, "cam", *getCamMat());
	setUniformMat4(shader, "mat", *getProjMat());

  vec3 verts[] = {
    {a[0], a[1], a[2]},
    {b[0], b[1], b[2]},
  };

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * 4, verts);

  glDrawArrays(GL_LINES, 0, 2);

  glBindVertexArray(0);
}
