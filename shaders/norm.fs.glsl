#version 330

#define SKYBOX 0
#define TRIANGLES 1

#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6

in vec3 gNormal;
in vec2 gTexCoords;
in float gBending;
in float gDrain;
in float gGradient;
in float gSurface;

uniform vec3 uLightVect = vec3(0.,0.,0.);
uniform sampler2D uTexture;
uniform int uMode;
uniform int uChoice;
uniform float uMaxBending;
uniform float uMaxDrain;
uniform float uMaxGradient;
uniform float uMaxSurface;

out vec4 fFragColor;

void main() {
	if(uMode == TRIANGLES){

		vec3 dColor = vec3(0.8f, 0.8f, 0.85f);
		float ratio;

		if(uChoice == BENDING){
			ratio = gBending/uMaxBending;
			dColor = vec3(1.f - ratio, ratio, 1.f - ratio);
		}
		else if(uChoice == DRAIN){
			ratio = gDrain/uMaxDrain;
			dColor = vec3(1.f - ratio, 1.f - ratio, ratio);
		}
		else if(uChoice == GRADIENT){
			ratio = gGradient/uMaxGradient;
			dColor = vec3(ratio, 1.f - ratio, 1.f - ratio);
		}
		else if(uChoice == SURFACE){
			ratio = gSurface/uMaxSurface;
			dColor = vec3(0.5f - ratio, ratio, 0.5f - ratio);
		}

		float dCoeff = max(0, dot(normalize(gNormal), -normalize(uLightVect)));

		vec3 aColor = vec3(0.1f, 0.1f, 0.1f);
		vec3 color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColor*dCoeff);
		
		fFragColor = vec4(color, 1.f);

	}else if(uMode == SKYBOX){
		fFragColor = texture(uTexture, gTexCoords);
	}
}
