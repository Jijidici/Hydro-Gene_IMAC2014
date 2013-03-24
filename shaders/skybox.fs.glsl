#version 330

in vec2 vPos;

out vec4 fFragColor;

void main(){
	float r = 0.f;
	float g = 0.f;
	float b = 0.f;
	
	if(vPos.y > 0){
		r = 1.f;
	}else{
		g = 1.f;
	}
	
	if(vPos.x > 0){
		b = 1.f;
	}
	
	fFragColor = vec4(r, g, b, 1.f);
}
