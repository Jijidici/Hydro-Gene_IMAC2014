#version 330

in vec2 vPos;

out vec4 fFragColor;

void main(){
	fFragColor = vec4(vPos, 0.f, 1.f);
}
