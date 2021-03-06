#version 130

uniform mat4 camera;
uniform mat4 projection;
uniform mat4 model;

in vec3 vert;
in vec2 vert_tex_coord;

out vec2 frag_tex_coord;

void main() {
    frag_tex_coord = vert_tex_coord;

    gl_Position = projection * camera * model * vec4(vert, 1);
}
