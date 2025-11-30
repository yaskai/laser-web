/*
#version 330

in vec3 frag_normal;
out vec4 final_color;

void main() {
	final_color = vec4(normalize(frag_normal) * 0.5 + 0.5, 1.0);
}
*/

#version 100
precision mediump float;

varying vec3 frag_normal;

void main() {
    gl_FragColor = vec4(normalize(frag_normal) * 0.5 + 0.5, 1.0);
}
