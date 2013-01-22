#version 330

#define SKYBOX 0
#define TRIANGLES 1

#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6

in vec3 vNormal;
in vec2 vTexCoords;
in float vBending;
in float vDrain;
in float vGradient;
in float vSurface;

uniform vec3 uLightVect = vec3(0.,0.,0.);
uniform sampler2D uTexture;
uniform int uMode;
uniform int uChoice;
uniform vec3 uColor = vec3(1., 1., 1.);
uniform float uMaxBending;
uniform float uMaxDrain;
uniform float uMaxGradient;
uniform float uMaxSurface;

out vec4 fFragColor;

void main() {
	if(uMode == TRIANGLES){

		vec3 dColor = uColor;
		float ratio;

		if(uChoice == BENDING){
			ratio = vBending/uMaxBending;
			dColor = vec3(1.f - ratio, ratio, 1.f - ratio);
		}
		else if(uChoice == DRAIN){
			ratio = vDrain/uMaxDrain;
			dColor = vec3(1.f - ratio, 1.f - ratio, ratio);
		}
		else if(uChoice == GRADIENT){
			ratio = vGradient/uMaxGradient;
			dColor = vec3(ratio, 1.f - ratio, 1.f - ratio);
		}
		else if(uChoice == SURFACE){
			ratio = vSurface/uMaxSurface;
			dColor = vec3(1.f - ratio, ratio, 1.f - ratio);
		}

		float dCoeff = max(0, dot(normalize(vNormal), -normalize(uLightVect)));

		vec3 aColor = vec3(0.02f, 0.02f, 0.f);
		vec3 color = vec3(1.f, 1.f, 1.f) * (aColor + dColor*dCoeff);
		
		fFragColor = vec4(color, 1.f);

	}else if(uMode == SKYBOX){
		fFragColor = texture(uTexture, vTexCoords);
	}
}
