#version 300 es
precision mediump float;

#define NR_POINT_LIGHTS 8

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D tex;
    float shininess;
}; 


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

in vec3 v_colors;
in vec2 v_texcoord;
in vec3 v_normal;
in vec3 fragPos;

uniform Material material;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(v_normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 finalColor = vec3(0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        finalColor.xyz += CalcPointLight(pointLights[i], norm, fragPos, viewDir);
    
    FragColor = vec4(finalColor, 1.0) * vec4(v_colors, 1.0);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.tex, v_texcoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.tex, v_texcoord));
    vec3 specular = light.specular * spec * vec3(texture(material.tex, v_texcoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
