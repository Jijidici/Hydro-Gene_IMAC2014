#version 330

in vec3 vNormal;

uniform vec3 uLightVect = vec3(-1.,1.,1.);

out vec4 fFragColor;

void main() {
	float NdotL = dot(normalize(vNormal), normalize(uLightVect));
	
	fFragColor = vec4(1.f * NdotL, 1.f * NdotL, 1.f * NdotL, 1.f);
}
