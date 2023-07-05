#version 300 es

layout(location = 0) in vec3 position;

uniform mat4 view;
uniform mat4 proj;
uniform float pointSize;

void main()
{
    vec4 viewPosition = vec4(position, 1.0) * view;
    float distance = length(viewPosition.xyz);
    
    float scalingFactor = pointSize / distance;
    gl_PointSize = scalingFactor;
    
    gl_Position = viewPosition * proj;
}

