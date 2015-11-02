#version 130

uniform sampler2D tex;

in vec2 frag_tex_coord;

out vec4 final_colour;

void main() {
    final_colour = texture(tex, frag_tex_coord);
}
