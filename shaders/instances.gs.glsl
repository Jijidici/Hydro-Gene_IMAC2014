#version 330

#define SKYBOX 0
#define TRIANGLES 1
#define VEGET 7

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

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
	  for(int i=0; i<gl_in.length(); i++){
			gPos = vPos[i];
			gTexCoords = vTexCoords[i];
		  gl_Position = gl_in[i].gl_Position;
	 	  EmitVertex();
		}
	 	EndPrimitive();
	}
	else if(uMode == TRIANGLES){
		if(uChoice == VEGET){
			float distance = uDistance/2;
			vec4 testDistance = uModelView * vec4(vPos[0],1.f);
			float vegetSizeCoef = 0.5f;
			if(length(testDistance)-1.05f < distance){
				for(int i=0; i<gl_in.length(); i++){

					gBending = vBending[i];
					gDrain = vDrain[i];
					gGradient = vGradient[i];
					gSurface = vSurface[i];
					gAltitude = vAltitude[i];

					gl_Position = gl_in[i].gl_Position;
					gTexCoords = vec2(0.0, 1.0);
			 	  EmitVertex();
					gl_Position.x = gl_in[i].gl_Position.x;
					gl_Position.y = gl_in[i].gl_Position.y + 0.01f*vegetSizeCoef;
					gTexCoords = vec2(0.0, 0.0);
			 	  EmitVertex();
					gl_Position.x = gl_in[i].gl_Position.x + 0.005f*vegetSizeCoef;
					gl_Position.y = gl_in[i].gl_Position.y + 0.01f*vegetSizeCoef;
					gTexCoords = vec2(1.0, 0.0);
			 	  EmitVertex();
					gl_Position.x = gl_in[i].gl_Position.x + 0.005f*vegetSizeCoef;
					gl_Position.y = gl_in[i].gl_Position.y;
					gTexCoords = vec2(1.0, 1.0);
			 	  EmitVertex();
					gl_Position.x = gl_in[i].gl_Position.x;
					gl_Position.y = gl_in[i].gl_Position.y;
					gTexCoords = vec2(0.0, 1.0);
			 	  EmitVertex();
				}
		 	}
		 	EndPrimitive();
		}
		else{
			for(int i=0; i<gl_in.length(); i++){
				gTexCoords = vTexCoords[i];
				gNormal = vNormal[i];
				gPos = vPos[i];
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
}
