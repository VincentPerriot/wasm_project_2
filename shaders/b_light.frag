#version 300 es

precision mediump float;

uniform vec4 pointColor;

out vec4 FragColor;

void main()
{
	float r = 0.0, delta = 0.0, alpha = 1.0;
    
    vec2 cxy = 2.0 * gl_PointCoord - 1.0;
    r = dot(cxy, cxy);

    // Create smooth circle
    delta = fwidth(r);
    alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r);

    // Add a little glow effect
    float dist = 1.0/length(cxy);
    dist *= 0.6;
    dist = pow(dist, 0.8);

    if (alpha * dist < 0.2)
        alpha = 0.0;

    FragColor = pointColor * alpha * dist;
 }

