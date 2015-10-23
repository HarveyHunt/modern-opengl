#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "shader.h"
#include "deps/lodepng.h"

#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640
#define SCREEN_BPP 32

static bool init_gl(void);
static bool init(void);
static void handle_keys(SDL_Keycode key);
static void render(void);
static void load_triangle(void);
static GLuint load_texture(const char *filename, GLint min_mag_filt, GLint wrap_mode);
static void flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height);

bool running = true;
GLuint vao;
GLuint vbo;
GLuint prog;
GLuint tex;

SDL_Window *window;
SDL_GLContext gl_context;

int main(int argc, char **argv)
{
	SDL_Event event;

	if (!init())
		return EXIT_FAILURE;

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				running = false;
			else if (event.type == SDL_KEYDOWN)
				handle_keys(event.key.keysym.sym);
		}
		render();
	}

	SDL_Quit();

	return EXIT_SUCCESS;
}

static bool init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return false;

	window = SDL_CreateWindow("OpenGL Test", SDL_WINDOWPOS_UNDEFINED,
			 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
			 SDL_WINDOW_OPENGL);
	if (!window) {
		printf("SDL error: %s", SDL_GetError());
		return false;
	}

	gl_context = SDL_GL_CreateContext(window);
	if (!gl_context) {
		printf("SDL error: %s", SDL_GetError());
		return false;
	}

	if (!init_gl())
		return false;

	prog = load_program("vert.glsl", "frag.glsl");
	if (!prog)
		return false;

	tex = load_texture("hazard.png", GL_LINEAR, GL_CLAMP_TO_EDGE);

	load_triangle();

	return true;
}

static bool init_gl(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	GLenum err = glGetError();

	if (err != GL_NO_ERROR) {
		printf("Error initialising OpenGL: %s\n", gluErrorString(err));
		return false;
	}

	err = glewInit();
	if (err != GLEW_OK)
		return false;

	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));

	return true;
}

static void handle_keys(SDL_Keycode key)
{
	if (key == SDLK_ESCAPE)
		running = false;
}

static void render(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(prog);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(prog, "tex"), 0);

	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glUseProgram(0);

	SDL_GL_SwapWindow(window);
}

static void load_triangle(void)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLfloat vertex_data[] = {
		/* X Y Z -- U V */
		0.0f, 0.8f, 0.0f, 0.5f, 1.0f,
		-0.8f, -0.8f, 0.0f, 0.0f, 0.0f,
		0.8f, -0.8f, 0.0f, 1.0f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
			GL_STATIC_DRAW);

	glEnableVertexAttribArray(glGetAttribLocation(prog, "vert"));
	glVertexAttribPointer(glGetAttribLocation(prog, "vert"), 3,
			GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), NULL);

	glEnableVertexAttribArray(glGetAttribLocation(prog, "vert_tex_coord"));
	glVertexAttribPointer(glGetAttribLocation(prog, "vert_tex_coord"), 2,
			GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat),
			(const GLvoid *)(3 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static GLuint load_texture(const char *filename, GLint min_mag_filt, GLint wrap_mode)
{
	unsigned char *data;
	unsigned int width, height, err;
	GLuint texture;

	err = lodepng_decode32_file(&data, &width, &height, filename);
	if (err)
		fprintf(stderr, "Failed to load %s: %s\n", filename, lodepng_error_text(err));

	flip_image_vertical(data, width, height);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_mag_filt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, min_mag_filt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

static void flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height)
{
	unsigned int size = width * height * 4;
	unsigned int stride = sizeof(char) * width * 4;
	unsigned int i, j;
	unsigned char *new_data = malloc(sizeof(unsigned char) * size);

	for (i = 0; i < height; i++) {
		j = height - i - 1;
		memcpy(new_data + j * stride, data + i * stride, stride);
	}

	memcpy(data, new_data, size);
	free(new_data);
}
