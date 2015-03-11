// basicsound.frag
#version 330

// shadertoy.com effect.js line 77
//#extension GL_OES_standard_derivatives : enable
//uniform float     iChannelTime[4];
uniform float     iBlockOffset;
//uniform vec4      iDate;
uniform float     iSampleRate;
//uniform vec3      iChannelResolution[4];

out vec4 glFragColor;

vec2 mainSound(float time)
{
    return vec2( sin(6.2831*440.0*time)*exp(-3.0*time) );
}

void main()
{
    float t = iBlockOffset + (gl_FragCoord.x + gl_FragCoord.y*512.0)/44100.0;

    vec2 y = mainSound( t );

    vec2 v  = floor((0.5+0.5*y)*65536.0);
    vec2 vl =   mod(v,256.0)/255.0;
    vec2 vh = floor(v/256.0)/255.0;
    glFragColor = vec4(vl.x,vh.x,vl.y,vh.y);
}
