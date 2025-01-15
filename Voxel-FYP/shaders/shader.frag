#version 450

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 FragPos;

layout(location = 0) out vec4 outColor;

vec3 lightPos = vec3(10.0, 15.0, -20.0);
vec3 viewPos = vec3(0.0, 10.0, -35.0);
vec3 lightColor = vec3(0.0, 0.0, 1.0);
vec3 objectColor = vec3(0.7, 0.7, 0.7);

void main() {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    outColor = vec4(result, 1.0);
}