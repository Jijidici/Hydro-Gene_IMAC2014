#version 330

uniform ivec2 uNbIntersection = ivec2(1.f);

out vec4 fFragColor;

void main() {
	
	float ratioIntersection = uNbIntersection[0] / float(uNbIntersection[1]);
	fFragColor = vec4(1.f, 1.f - ratioIntersection, 0.1f, 1.f);
}
