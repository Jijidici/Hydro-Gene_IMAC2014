#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 uMVPMatrix = mat4(1.f);

out vec3 vNormal;

void main(){
	vNormal = normal;
	gl_Position = uMVPMatrix * vec4(position, 1.f);
}
