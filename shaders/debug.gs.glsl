#version 330

#define SKYBOX 0
#define TRIANGLES 1
#define VEGET 7

layout(triangles) in;
layout(line_strip, max_vertices=3) out;

in vec3 vPos[];
in vec3 vNormal[];
in vec2 vTexCoords[];
in float vBending[];
in float vDrain[];
in float vGradient[];
in float vSurface[];
in float vAltitude[];

uniform mat4 uMVPMatrix = mat4(1.f);
uniform mat4 uModelView = mat4(1.f);
uniform float uDistance;
uniform int uMode;
uniform int uChoice;

out vec3 gPos;
out vec3 gNormal;
out vec2 gTexCoords;
out float gBending;
out float gDrain;
out float gGradient;
out float gSurface;
out float gAltitude;

void main(){	
	if(uMode == SKYBOX){
		for(int i=0; i<gl_in.length(); ++i){
			gPos = vPos[i];
			gTexCoords = vTexCoords[i];
			gl_Position = gl_in[i].gl_Position;
	 	  EmitVertex();
		}
	 	EndPrimitive();
	}
	else if(uMode == TRIANGLES){
		for(int i=0; i<gl_in.length(); ++i){
			gPos = vPos[i];
			gTexCoords = vTexCoords[i];
			gNormal = vNormal[i];
			gBending = vBending[i];
			gDrain = vDrain[i];
			gGradient = vGradient[i];
			gSurface = vSurface[i];
			gAltitude = vAltitude[i];
			gl_Position = gl_in[i].gl_Position;
		  EmitVertex();
		}
		EndPrimitive();
	}
}
