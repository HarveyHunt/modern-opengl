#include <math.h>
#include <stdio.h>
#include "camera.h"
#include "deps/linmath.h"

#define MAX_VERT_ANGLE 85.0f

static void cam_norm_angles(struct camera *cam);
static void print_mat4x4(mat4x4 mat);
static void print_vec4(vec4 v);
static void print_vec3(vec3 v);

void cam_get_orientation(struct camera *cam, mat4x4 dest)
{
	mat4x4_rotate_X(dest, dest, RADIANS(cam->vert_angle));
	mat4x4_rotate_Y(dest, dest, RADIANS(cam->horiz_angle));
}

void cam_offset_orientation(struct camera *cam, float right, float down)
{
	cam->horiz_angle += right;
	cam->vert_angle += down;
	cam_norm_angles(cam);
}

void cam_look_at(struct camera *cam, vec3 pos)
{
	vec3 direction;

	vec3_sub(pos, pos, cam->pos);
	vec3_norm(direction, pos);

	cam->vert_angle = RADIANS(asinf(-direction[1]));
	cam->horiz_angle = -RADIANS(atan2f(-direction[0], -direction[2]));
	cam_norm_angles(cam);
}

void cam_move(struct camera *cam, const int d, float distance)
{
	vec4 dir, displacement;
	mat4x4 orientation;

	mat4x4_identity(orientation);

	cam_get_orientation(cam, orientation);
	mat4x4_invert(orientation, orientation);

	/* Calculate the direction as a unit vector inline
	 *
	 * XXX: mat4x4_mul_vec4(x, m, x) doesn't work...
	 */
	mat4x4_mul_vec4(dir, orientation, (vec4){
			d == LEFT || d == RIGHT ? 1 : 0,
			d == UP || d == DOWN ? 1 : 0,
			d == FORWARD || d == BACKWARD ? 1 : 0,
			1});

	if (d == LEFT || d == DOWN || d == FORWARD)
		vec4_scale(dir, dir, -1);

	/* displacement = dir * distance */
	vec4_scale(displacement, dir, distance);

	/* Displace the old position */
	vec3_add(cam->pos, cam->pos, (vec3){displacement[0],
					displacement[1],
					displacement[2]});
}

void cam_get_view(struct camera *cam, mat4x4 dest)
{
	mat4x4 trans, orientation;

	mat4x4_identity(trans);
	mat4x4_identity(orientation);

	cam_get_orientation(cam, orientation);
	mat4x4_translate(trans, cam->pos[0] * -1,
			cam->pos[1] * -1, cam->pos[2] * -1);

	mat4x4_mul(dest, orientation, trans);
}

void cam_get_projection(struct camera *cam, mat4x4 dest)
{
	mat4x4_perspective(dest, RADIANS(cam->fov), cam->vp_aspect_ratio,
			cam->near_plane, cam->far_plane);
}

void cam_get_matrix(struct camera *cam, mat4x4 dest)
{
	mat4x4 proj, view;

	mat4x4_identity(proj);
	mat4x4_identity(view);

	cam_get_projection(cam, proj);
	cam_get_view(cam, view);

	mat4x4_mul(dest, proj, view);
}

static void cam_norm_angles(struct camera *cam)
{
	cam->horiz_angle = fmodf(cam->horiz_angle, 360.0f);

	if (cam->horiz_angle < 0.0f)
		cam->horiz_angle += 360.0f;

	if (cam->vert_angle > MAX_VERT_ANGLE)
		cam->vert_angle = MAX_VERT_ANGLE;
	else if (cam->vert_angle < -MAX_VERT_ANGLE)
		cam->vert_angle = -MAX_VERT_ANGLE;
}

static void print_mat4x4(mat4x4 m)
{
	int r;

	printf("{");
	for (r = 0; r < 4; r++)
		printf("{%f, %f, %f, %f}%s\n", m[r][0], m[r][1],
						m[r][2], m[r][3],
						r == 3 ? "}" : "");
	printf("\n");
}

static void print_vec4(vec4 v)
{
	printf("vec: {%f, %f, %f, %f}\n", v[0], v[1], v[2], v[3]);
}

static void print_vec3(vec3 v)
{
	printf("vec: {%f, %f, %f}\n", v[0], v[1], v[2]);
}
