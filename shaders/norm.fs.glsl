#version 330

#define SKYBOX 0
#define TRIANGLES 1

in vec3 vNormal;
in vec2 vTexCoords;

uniform vec3 uLightVect = vec3(0.,0.,0.);
uniform sampler2D uTexture;
uniform int uMode;
uniform vec3 uColor = vec3(1., 1., 1.);

out vec4 fFragColor;

void main() {
	if(uMode == TRIANGLES){
		float dCoeff = max(0, dot(normalize(vNormal), -normalize(uLightVect)));

		vec3 aColor = vec3(0.02f, 0.02f, 0.f);
		vec3 dColor = uColor;
		vec3 color = vec3(1.f, 1.f, 1.f) * (aColor + dColor*dCoeff);
		
		fFragColor = vec4(color, 1.f);

	}else if(uMode == SKYBOX){
		fFragColor = texture(uTexture, vTexCoords);
	}
}
