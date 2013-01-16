#version 330

in vec3 vNormal;

uniform vec3 uLightVect = vec3(0.,0.,0.);

out vec4 fFragColor;

void main() {
	float dCoeff = max(0, dot(normalize(vNormal), -normalize(uLightVect)));

	vec3 aColor = vec3(0.02f, 0.02f, 0.f);
	vec3 dColor = vec3(0.88f, 0.7f, 0.23f);
	vec3 color = vec3(1.f, 1.f, 1.f) * (aColor + dColor*dCoeff);
	
	fFragColor = vec4(color, 1.f);
}
