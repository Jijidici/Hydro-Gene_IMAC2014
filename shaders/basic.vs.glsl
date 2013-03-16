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
uniform mat4 uViewMatrix = mat4(1.f);
uniform vec3 uLightSunVect = vec3(0.,0.,0.);

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoords;
out vec2 vCloudsTexCoords;
out float vBending;
out float vDrain;
out float vGradient;
out float vSurface;
out float vAltitude;

out vec4 gN;
out vec4 gH;
out vec4 gL;
out vec4 gV;

void main(){

	if(uMode == SKYBOX){
		vPos = position;
		vec4 realPosition = uMVPMatrix * vec4(position, 1.f);
		gl_Position = realPosition.xyww;
	}
	else if(uMode == TRIANGLES){
		vPos = position;
		//~ vTexCoords = (position.xz+1)*0.5;
		vTexCoords = (position.xz)*50;
		vCloudsTexCoords = (position.xz+1)*0.5;
		vNormal = normal;
		vBending = bending;
		vDrain = drain;
		vGradient = gradient;
		vSurface = surface;
		vAltitude = position.y;
		gl_Position = uMVPMatrix * vec4(position, 1.f);

		gN = normalize(transpose(inverse(uViewMatrix)) * vec4(normal, 0.f));
		gL = normalize(vec4(uLightSunVect,0.f));
		gV = normalize(uViewMatrix * vec4(vPos,1.f));
		gH = normalize(gV-gL);

	}
}
