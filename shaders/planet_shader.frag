#version 300 es
precision mediump float;

in vec3 v_colors;
in vec3 v_normal;
in vec3 fragPos;

uniform vec3 viewPos;

out vec4 FragColor;


vec4 CreateLight(vec3 lightPos, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir)
{

    //ambient
    float a_strength = 0.3;
    vec3 ambient = a_strength * lightColor;

    //diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    float s_strength = 0.8;
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 15.0);
    vec3 specular = s_strength * spec * lightColor;

    if (diffuse.x < 0.2  && diffuse.y < 0.2 && diffuse.z < 0.2)
    {
        specular = vec3(0.0);
    }

   return vec4(v_colors * (ambient + diffuse + specular), 1);
}

void main()
{
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 lightPos = vec3(0.0, 60.0, -20.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec4 outColor = CreateLight(lightPos, lightColor, v_normal, fragPos, viewDir);
    FragColor = outColor * vec4(v_colors, 1.0);
}