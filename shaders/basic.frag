// basic.frag
#version 330

out vec4 glFragColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor = vec4(1.);//vec4(fragCoord, 0., 1.);
}

void main()
{
    vec4 fragcol = vec4(0.);
    mainImage(fragcol, gl_FragCoord.xy);
    glFragColor = fragcol;
}
