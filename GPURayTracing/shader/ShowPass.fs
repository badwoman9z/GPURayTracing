#version 430 core

in vec3 pix;
out vec4 fragColor;

uniform sampler2D texPass0;
uniform vec3 eye;
uniform mat4 view;
struct FrameInfo{
    vec3 position;
    float meshId;
    vec4 normal;
};
struct PreFrameInfo {

    vec3 color;
    float meshId;
};
struct PreCameraParm {
    vec4 cameraPosition;
    mat4 preViewMat;
};


layout(std430,binding=4) buffer framessbo{
    FrameInfo frameInfo[];
};
layout(std430,binding=6) buffer preFrameInfo{
    PreFrameInfo preframeInfo[];
};
layout(std430,binding=5) buffer camerassbo{
    PreCameraParm precamera;
};
FrameInfo getFrameInfo(ivec2 index){

    return frameInfo[index.x*512+index.y];

}
vec3 toneMapping(in vec3 c, float limit) {
    float luminance = 0.3*c.x + 0.6*c.y + 0.1*c.z;
    return c * 1.0 / (1.0 + luminance / limit);
}

void main() {
   

    vec3 color = texture2D(texPass0, pix.xy*0.5+0.5).rgb;



    ivec2 index = ivec2(gl_FragCoord.xy);
 
    preframeInfo[index.x*512+index.y].color = color;
    preframeInfo[index.x*512+index.y].meshId = getFrameInfo(index).meshId;
    precamera.cameraPosition = vec4(eye,1.0);
    precamera.preViewMat = view;


    color = toneMapping(color, 1.5);
    color = pow(color, vec3(1.0 / 2.2));
    fragColor = vec4(color, 1.0);
}
