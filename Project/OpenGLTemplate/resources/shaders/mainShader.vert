#version 400 core

// Structure for matrices
uniform struct Matrices
{
	mat4 projMatrix;
	mat4 modelViewMatrix; 
	mat3 normalMatrix;
} matrices;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

// Outputs to fragment shader
out vec3 vColour;       
out vec2 vTexCoord;     
out vec3 worldPosition; 
out vec3 eyePosition;   
out vec3 eyeNormal;     

void main()
{
    worldPosition = inPosition;
    gl_Position = matrices.projMatrix * matrices.modelViewMatrix * vec4(inPosition, 1.0);
    
    eyePosition = vec3(matrices.modelViewMatrix * vec4(inPosition, 1.0));
    eyeNormal = normalize(matrices.normalMatrix * inNormal);
    
    vTexCoord = inCoord;
    
    vColour = vec3(1.0, 1.0, 1.0);
}