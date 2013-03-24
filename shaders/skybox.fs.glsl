#version 330

in vec2 vPos;

uniform vec3 uPlanOr;
uniform vec3 uPlanU;
uniform vec3 uPlanV;

out vec4 fFragColor;

void main(){
	float r = 0.f;
	float g = 0.f;
	float b = 0.f;
	
	if(uPlanOr.x == 0.5){
		r = 1.f;
	}else{
		g = 1.f;
	}
	
	fFragColor = vec4(r, g, b, 1.f);
}
