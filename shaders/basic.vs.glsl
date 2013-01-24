#version 330

#define SKYBOX 0
#define TRIANGLES 1

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoords;
layout(location = 3) in float bending;
layout(location = 4) in float drain;
layout(location = 5) in float gradient;
layout(location = 6) in float surface;

uniform mat4 uMVPMatrix = mat4(1.f);
uniform int uMode;

out vec3 vNormal;
out vec2 vTexCoords;
out float vBending;
out float vDrain;
out float vGradient;
out float vSurface;

void main(){

	if(uMode == SKYBOX){
		vTexCoords = texcoords;
		vec4 realPosition = uMVPMatrix * vec4(position, 1.f);
		realPosition = realPosition.xyww;
		gl_Position = realPosition;
	}
	else if(uMode == TRIANGLES){
		vTexCoords = (position.xz+1)*0.5;
		vNormal = normal;
		vBending = bending;
		vDrain = drain;
		vGradient = gradient;
		vSurface = surface;
		gl_Position = uMVPMatrix * vec4(position, 1.f);
	}
}
