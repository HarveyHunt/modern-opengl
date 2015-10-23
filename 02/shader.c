#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "shader.h"

char *load_file(const char *path)
{
	FILE *file;
	int length;
	char *data;

	file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	rewind(file);
	data = calloc(length + 1, sizeof(char));
	fread(data, 1, length, file);
	fclose(file);
	return data;
}

GLuint make_shader(GLenum type, const char *source)
{
	GLuint shader;
	GLint status, length;
	GLchar *info;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		info = calloc(length, sizeof(GLchar));
		glGetShaderInfoLog(shader, length, NULL, info);
		fprintf(stderr, "glCompileShader failed:\n%s\n", info);
		free(info);
	}

	return shader;
}

GLuint load_shader(GLenum type, const char *path)
{
	char *data = load_file(path);
	GLuint result = make_shader(type, data);

	free(data);
	return result;
}

GLuint make_program(GLuint shader1, GLuint shader2)
{
	GLint status, length;
	GLchar *info;
	GLuint program = glCreateProgram();

	glAttachShader(program, shader1);
	glAttachShader(program, shader2);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		info = calloc(length, sizeof(GLchar));
		glGetProgramInfoLog(program, length, NULL, info);
		fprintf(stderr, "glLinkProgram failed: %s\n", info);
		free(info);
	}

	glDetachShader(program, shader1);
	glDetachShader(program, shader2);
	glDeleteShader(shader1);
	glDeleteShader(shader2);
	return program;
}

GLuint load_program(const char *path1, const char *path2)
{
	GLuint shader1 = load_shader(GL_VERTEX_SHADER, path1);
	GLuint shader2 = load_shader(GL_FRAGMENT_SHADER, path2);
	GLuint program = make_program(shader1, shader2);

	return program;
}
