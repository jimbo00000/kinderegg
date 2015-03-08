// basic.frag
#version 330

uniform vec3 iResolution; // viewport resolution (in pixels)
uniform float iGlobalTime; // shader playback time (in seconds)

out vec4 glFragColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    fragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
}

void main()
{
    vec4 fragcol = vec4(0.);
    mainImage(fragcol, gl_FragCoord.xy);
    glFragColor = fragcol;
}
