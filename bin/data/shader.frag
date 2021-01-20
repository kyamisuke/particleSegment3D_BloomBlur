#version 150

uniform float time;
uniform vec2 resolution;
uniform sampler2DRect tex;
uniform float y_noise[720];

out vec4 outputColor;

void main() {

    float drift = 360.0;
    vec4 t = texture(tex, vec2(gl_FragCoord.x + drift * y_noise[int(gl_FragCoord.y)], (resolution.y - gl_FragCoord.y)));
    outputColor = t;
}
