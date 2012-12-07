#version 330

uniform ivec2 uNbIntersection = ivec2(1.f);
uniform vec3 uNormSum = vec3(1.);
uniform vec3 uLightVect = vec3(-1.,1.,1.);

out vec4 fFragColor;

void main() {
	
	vec3 light = vec3(cos(uLightVect.x), uLightVect.y, uLightVect.z);
	float NdotL = dot(normalize(uNormSum/uNbIntersection[0]), normalize(light)) / 2.f;
	
	fFragColor = vec4(1.f * NdotL, 1.f * NdotL, 1.f * NdotL, 1.f);
}
