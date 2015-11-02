#ifndef _camera_h_
#define _camera_h_

#include "deps/linmath.h"

#define PI 3.14159265359
#define DEGREES(radians) ((radians) * 180 / PI)
#define RADIANS(degrees) ((degrees) * PI / 180)
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SIGN(x) (((x) > 0) - ((x) < 0))

enum direction { LEFT, RIGHT, UP, DOWN, FORWARD, BACKWARD };

struct camera {
	vec3 pos;
	float horiz_angle;
	float vert_angle;
	float fov;
	float near_plane;
	float far_plane;
	float vp_aspect_ratio;
};

void cam_get_orientation(struct camera *cam, mat4x4 dest);
void cam_offset_orientation(struct camera *cam, float up, float right);
void cam_look_at(struct camera *cam, vec3 pos);
void cam_move(struct camera *cam, const int d, float distance);
void cam_get_view(struct camera *cam, mat4x4 dest);
void cam_get_projection(struct camera *cam, mat4x4 dest);
void cam_get_matrix(struct camera *cam, mat4x4 dest);
#endif
