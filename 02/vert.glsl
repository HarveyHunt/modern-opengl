#version 130

in vec3 vert;
in vec2 vert_tex_coord;

out vec2 frag_tex_coord;

void main() {
    frag_tex_coord = vert_tex_coord;

    gl_Position = vec4(vert, 1);
}
