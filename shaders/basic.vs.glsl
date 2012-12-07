#version 330

layout(location = 0) in vec3 position;

uniform mat4 uMVPMatrix = mat4(1.f);

void main(){
	gl_Position = uMVPMatrix * vec4(position, 1.f);
}
