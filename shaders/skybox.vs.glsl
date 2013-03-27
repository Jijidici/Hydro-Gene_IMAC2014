#version 330

layout(location=0) in vec2 position;

out vec2 vPos;

void main(){
	vPos = (position+1.)*0.5;
	gl_Position = vec4(position, 0.f, 1.f);
}
