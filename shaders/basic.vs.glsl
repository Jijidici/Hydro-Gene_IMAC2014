#version 330

#define SKYBOX 0
#define TRIANGLES 1

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoords;

uniform mat4 uMVPMatrix = mat4(1.f);
uniform int uMode;

out vec3 vNormal;
out vec2 vTexCoords;

void main(){

	if(uMode == SKYBOX){
		vTexCoords = texcoords;
		vec4 realPosition = uMVPMatrix * vec4(position, 1.f);
		realPosition = realPosition.xyww;
		gl_Position = realPosition;
	}
	else if(uMode == TRIANGLES){
		vNormal = normal;
		gl_Position = uMVPMatrix * vec4(position, 1.f);
	}
}
