// basic.frag
#version 330

uniform vec3 iResolution; // viewport resolution (in pixels)

out vec4 glFragColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor = vec4(fragCoord, 0., 1.);
}

void main()
{
    vec4 fragcol = vec4(0.);
    mainImage(fragcol, gl_FragCoord.xy/iResolution.xy);
    glFragColor = fragcol;
}
