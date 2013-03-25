#version 330

in vec2 vPos;

uniform vec3 uPlanOr;
uniform vec3 uPlanU;
uniform vec3 uPlanV;

out vec4 fFragColor;

void main(){
	vec3 absolutePos = normalize(uPlanOr + vPos.x*uPlanU + vPos.y*uPlanV);
	
	float skyGama = 0.5 + 0.5*max(0., 1 - absolutePos.y);
	fFragColor = vec4(skyGama, skyGama, skyGama, 1.f);
}
