#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "shader.h"
#include "camera.h"
#include "deps/lodepng.h"
#include "deps/linmath.h"

#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640
#define SCREEN_BPP 32
#define DEGREES_PER_SECOND 180
#define MOVE_SPEED 5.0f
#define MOUSE_SENS 20.0f
#define FOV_SENS 1.2f

static bool init_gl(void);
static bool init(void);
static void handle_keys(SDL_Keycode key);
static void render(void);
static void load_cube(void);
static void update(float delta);
static GLuint load_texture(const char *filename, GLint min_mag_filt, GLint wrap_mode);
static void flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height);

int direction = -1;
bool running = true;
GLuint vao;
GLuint vbo;
GLuint prog;
GLuint tex;
GLfloat degrees_rotated;
struct camera cam = {
	.fov = 50.0f,
	.near_plane = 0.1f,
	.far_plane = 10.0f,
	.vp_aspect_ratio = SCREEN_WIDTH / SCREEN_HEIGHT,
	.pos = {0, 0, 4},
	.vert_angle = 0.0f,
	.horiz_angle = 0.0f
};
SDL_Window *window;
SDL_GLContext gl_context;

int main(int argc, char **argv)
{
	SDL_Event event;
	int cur_time, prev_time = 0;
	int x, y;

	if (!init())
		return EXIT_FAILURE;

	SDL_SetRelativeMouseMode(true);

	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				handle_keys(event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				SDL_GetRelativeMouseState(&x, &y);
				cam_offset_orientation(&cam, -x / MOUSE_SENS,
							y / MOUSE_SENS);
				break;
			case SDL_MOUSEWHEEL:
				if (event.wheel.y != 0)
					cam.fov = cam.fov +
						(FOV_SENS * -event.wheel.y);
				break;
			}
		}
		cur_time = SDL_GetTicks();
		update((float)(cur_time - prev_time) / 1000);
		render();
		prev_time = cur_time;
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

	tex = load_texture("wooden-crate.png", GL_LINEAR, GL_CLAMP_TO_EDGE);
	load_cube();

	return true;
}

static bool init_gl(void)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	else if (key == SDLK_w)
		direction = FORWARD;
	else if (key == SDLK_s)
		direction = BACKWARD;
	else if (key == SDLK_a)
		direction = LEFT;
	else if (key == SDLK_d)
		direction = RIGHT;
	else if (key == SDLK_z)
		direction = UP;
	else if (key == SDLK_x)
		direction = DOWN;
}

static void render(void)
{
	mat4x4 model, trans, camera;

	mat4x4_identity(trans);
	mat4x4_rotate(model, trans, 0.0f, 1.0f, 0.0f, RADIANS(degrees_rotated));

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(prog);

	cam_get_matrix(&cam, camera);
	glUniformMatrix4fv(glGetUniformLocation(prog, "camera"), 1, GL_FALSE, (GLfloat *) camera);
	glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, (GLfloat *) model);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(prog, "tex"), 0);
	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	SDL_GL_SwapWindow(window);
}

static void load_cube(void)
{
	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLfloat vertex_data[] = {
		/*  X     Y     Z       U     V */
		/* bottom */
		-1.0f, -1.0f, -1.0f,   0.0f, 0.0f,
		1.0f, -1.0f, -1.0f,   1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,   0.0f, 1.0f,
		1.0f, -1.0f, -1.0f,   1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,   1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,   0.0f, 1.0f,

		/* top */
		-1.0f, 1.0f, -1.0f,   0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		1.0f, 1.0f, -1.0f,   1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,   1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,   1.0f, 1.0f,

		/* front */
		-1.0f, -1.0f, 1.0f,   1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,   0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,   0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,

		/* back */
		-1.0f, -1.0f, -1.0f,   0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,   0.0f, 1.0f,
		1.0f, -1.0f, -1.0f,   1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,   1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,   0.0f, 1.0f,
		1.0f, 1.0f, -1.0f,   1.0f, 1.0f,

		/* left */
		-1.0f, -1.0f, 1.0f,   0.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,   1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,   0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,   0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,   1.0f, 0.0f,

		/* right */
		1.0f, -1.0f, 1.0f,   1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,   1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,   0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,   1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,   0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 1.0f
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
	glUseProgram(0);
}

static GLuint load_texture(const char *filename, GLint min_mag_filt, GLint wrap_mode)
{
	unsigned char *data;
	unsigned int width, height, err;
	GLuint texture;

	err = lodepng_decode24_file(&data, &width, &height, filename);
	if (err)
		fprintf(stderr, "Failed to load %s: %s\n", filename, lodepng_error_text(err));

	flip_image_vertical(data, width, height);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_mag_filt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, min_mag_filt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

/*XXX: This has to be updated to accept an RGB image as we have 24 bits of data, not 32.*/
static void flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height)
{
	unsigned int size = width * height * 3;
	unsigned int stride = sizeof(char) * width * 3;
	unsigned int i, j;
	unsigned char *new_data = malloc(sizeof(unsigned char) * size);

	for (i = 0; i < height; i++) {
		j = height - i - 1;
		memcpy(new_data + j * stride, data + i * stride, stride);
	}

	memcpy(data, new_data, size);
	free(new_data);
}

static void update(float delta)
{
	degrees_rotated += delta * DEGREES_PER_SECOND;
	if (degrees_rotated > 360.0f)
		degrees_rotated -= 360.0f;

	if (direction >= 0) {
		cam_move(&cam, direction, delta * MOVE_SPEED);
		direction = -1;
	}
}
