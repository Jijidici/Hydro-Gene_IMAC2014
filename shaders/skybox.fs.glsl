#version 330

in vec2 vPos;

uniform vec3 uPlanOr;
uniform vec3 uPlanU;
uniform vec3 uPlanV;
uniform vec3 uSunPos;

out vec4 fFragColor;

vec3 HSLtoRGB(int H, float S, float L){
	float chroma = (1 - abs(2.*L -1.))*S;
	int Hprime = H/60;
	float X = chroma*(1. - abs(Hprime%2 -1.));
	
	vec3 rgbColor;
	switch(Hprime){
		case 0:
			rgbColor = vec3(chroma, X, 0.);
			break;
		
		case 1:
			rgbColor = vec3(X, chroma, 0.);
			break;
			
		case 2:
			rgbColor = vec3(0., chroma, X);
			break;
			
		case 3:
			rgbColor = vec3(0., X, chroma);
			break;
			
		case 4:
			rgbColor = vec3(X, 0., chroma);
			break;
			
		case 5:
			rgbColor = vec3(chroma, 0., X);
			break;
			
		default:
			rgbColor = vec3(0., 0., 0.);
			break;
	}
	
	float m = L - 0.5*chroma;
	return rgbColor + m;
}

void main(){
	vec3 absolutePos = normalize(uPlanOr + vPos.x*uPlanU + vPos.y*uPlanV);
	
	float skyLightness = 0.3 + 0.5*max(0., 1 - absolutePos.y);
	float skySat = 0.7;
	int skyHue = 220;
	fFragColor = vec4(HSLtoRGB(skyHue, skySat, skyLightness), 1.f);
}
