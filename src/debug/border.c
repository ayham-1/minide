#include "minide/debug/border.h"
#include "minide/logger.h"
#include "minide/shaders_util.h"

#include "minide/app.h"

bool did_init = false;
GLuint shader_program;
GLuint attribute_coord;
GLuint uniform_projection;
GLuint vbo;
GLuint ibo;
GLuint vao;

GLuint indices[] = {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
};

#ifndef NDEBUG
void debug_border(GLsizei x1, GLsizei y1, GLsizei x2, GLsizei y2)
{
	if (!did_init) {
		shader_program = shader_program_create("glsl/debug/border.v.glsl", "glsl/debug/border.f.glsl");
		attribute_coord = shader_get_attrib(shader_program, "coord");
		uniform_projection = shader_get_uniform(shader_program, "projection");
		// glGenBuffers(1, &ibo);
		glGenBuffers(1, &vbo);
		glGenVertexArrays(1, &vao);
		did_init = true;
	}

	x1 += 1;
	y1 += 1;
	x2 += 1;
	y2 += 1;

	GLfloat vertices[] = {
	    // Position
	    x1, y2, // Bottom-left corner
	    x2, y2, // Bottom-right corner

	    x2, y1, // Top-right corner
	    x1, y1, // Top-left corner

	    x1, y1, // Top-left corner
	    x1, y2, // Bottom-left corner

	    x2, y1, // Top-right corner
	    x2, y2, // Bottom-right corner
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glBindVertexArray(vao);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glUseProgram(shader_program);
	glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, (GLfloat *)app_config.gl_projection);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_LINES, 0, sizeof(vertices) / sizeof(GLfloat));
}
#else

void debug_border(mat4 projection, GLsizei x1, GLsizei y1, GLsizei x2, GLsizei y2) {}

#endif
