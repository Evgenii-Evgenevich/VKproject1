#version 450

layout (location = 0) in vec3 inputPosition;
layout (location = 1) in vec3 inputColor;

layout (binding = 0) uniform MVP
{
	mat4 uModelMatrix;
	mat4 uViewMatrix;
	mat4 uProjectionMatrix;
} uniforms;

layout (location = 0) out vec3 fragColor;
 
out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
	fragColor = inputColor;
	
	gl_Position = uniforms.uProjectionMatrix * uniforms.uViewMatrix * uniforms.uModelMatrix * vec4(inputPosition, 1.0f);
}
