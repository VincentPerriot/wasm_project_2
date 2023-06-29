precision mediump float;

uniform float time;

vec3 palette( float t )
{
	vec3 a = vec3(0.5, 0.5, 0.5);
	vec3 b = vec3(0.5, 0.5, 0.5);
	vec3 c = vec3(1.0, 1.0, 1.0);
	vec3 d = vec3(0.263, 0.416, 0.557);

	return a + b * cos(6.28318 * (c * t + d));
}

//Commenting excellent tutorial from kishimisu youtube channel
void main()
{
	vec2 uv = ( gl_FragCoord.xy * 2.0 - vec2(800.0, 600.0) ) / 600.0;
	vec2 uv0 = uv;

	vec3 finalColor = vec3(0.0);

	// Fract function only return fractional part of input
	uv = fract(uv * 2.0) - 0.5;

	float d = length(uv);
	float d0 = length(uv0);

	vec3 col = palette(d0 + time);

	// Use sin with time to obtain repetitive moving concentric circles
	d =sin(d*8.0 + time)/8.0;
	d = abs(d);
	
	// Space ranges from 0 to 1, so we need to scale inverse function
	d = 0.02 / d;

	finalColor += col * d;

	gl_FragColor = vec4(finalColor, 1.0);
}
