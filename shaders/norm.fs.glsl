#version 330

#define SKYBOX 0
#define TRIANGLES 1

#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6

#define VEGET 7

in vec3 gNormal;
in vec2 gTexCoords;
in float gBending;
in float gDrain;
in float gGradient;
in float gSurface;
in float gAltitude;

uniform vec3 uLightSunVect = vec3(0.,0.,0.);
uniform vec3 uLightMoonVect = vec3(0.,0.,0.);
uniform sampler2D uSkyTex;
uniform sampler2D uNightTex;
uniform sampler2D uGrassTex;
uniform sampler2D uWaterTex;
uniform sampler2D uStoneTex;
uniform sampler2D uSnowTex;
uniform sampler2D uSandTex;
uniform sampler2D uPineTreeTex;
uniform int uMode;
uniform int uChoice;
uniform float uTime;
uniform float uDay;
uniform float uNight;
uniform float uMaxBending = 0;
uniform float uMaxDrain = 0;
uniform float uMaxGradient = 0;
uniform float uMaxSurface = 0;
uniform float uMaxAltitude = 0;

out vec4 fFragColor;

void main() {
	if(uMode == TRIANGLES){
	
		/* ratios */
		float ratioDrain;
		if(uMaxDrain == 0){	ratioDrain = 0;}
		else{ratioDrain = gDrain/uMaxDrain;}
		
		float ratioGradient;
		if(uMaxGradient == 0){	ratioGradient = 0;}
		else{ratioGradient = gGradient/uMaxGradient;}
		
		float ratioAltitude;
		if(uMaxAltitude == 0){	ratioAltitude = 0;}
		else{ratioAltitude = gAltitude/uMaxAltitude;}
		
		float ratioSurface;
		if(uMaxSurface == 0){	ratioSurface = 0;}
		else{ratioSurface = gSurface/uMaxSurface;}
		
		float restingCoef = 1.;
		
		float coefWater = min(10*ratioDrain, restingCoef);
		restingCoef -= coefWater;	
			
		float coefSnow = min(max(8*(restingCoef*ratioAltitude-0.75), 0.), restingCoef);
		restingCoef -= coefSnow;
		
		float coefStone = restingCoef*ratioGradient;
		restingCoef -= coefStone;		
		
		float coefSand = min(max(4*(restingCoef*(1-ratioAltitude)-0.75), 0.), restingCoef);
		restingCoef -= coefSand;
		
		float coefGrass = restingCoef;
		
		vec3 dColor = coefWater*texture(uWaterTex, gTexCoords).rgb;
		dColor += coefStone*texture(uStoneTex, gTexCoords).rgb;
		dColor += coefSnow*texture(uSnowTex, gTexCoords).rgb;
		dColor += coefSand*texture(uSandTex, gTexCoords).rgb;
		dColor += coefGrass*texture(uGrassTex, gTexCoords).rgb;
		
		vec3 dColorSun = dColor + vec3(0.5f*abs(uTime),0.f,0.f);
		vec3 dColorMoon = dColor + vec3(0.f,0.f,0.25f);
		
		float dCoeffSun = max(0, dot(normalize(gNormal), -normalize(uLightSunVect)));
		float dCoeffMoon = max(0, dot(normalize(gNormal), -normalize(uLightMoonVect)));
		dCoeffSun *= 0.7;
		dCoeffMoon *= 0.1;

		vec3 aColor = vec3(0.05f, 0.05f, 0.05f);
		vec3 color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColorSun*dCoeffSun + dColorMoon*dCoeffMoon);
		
		float ratio;
		if(uChoice == BENDING){
			ratio = gBending/uMaxBending;
			dColor = vec3(1.f - ratio, ratio, 1.f - ratio);
			color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColor*dCoeffSun + dColor*dCoeffMoon);
		}
		else if(uChoice == DRAIN){
			dColor = vec3(1.f - ratioDrain, 1.f - ratioDrain, ratioDrain);
			color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColor*dCoeffSun + dColor*dCoeffMoon);
		}
		else if(uChoice == GRADIENT){
			dColor = vec3(ratioGradient, 1.f - ratioGradient, 1.f - ratioGradient);
			color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColor*dCoeffSun + dColor*dCoeffMoon);
		}
		else if(uChoice == SURFACE){
			ratio = gSurface/uMaxSurface;
			dColor = vec3(0.5f - ratio, ratio, 0.5f - ratio);
			color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColor*dCoeffSun + dColor*dCoeffMoon);
		}
		
		fFragColor = vec4(color, 1.f);

		if(uChoice == VEGET){
			vec4 texel = texture(uPineTreeTex, gTexCoords);
				if(texel.a <0.5){
					discard;
			}
			fFragColor = texel;
		}	

	}
	else if(uMode == SKYBOX){
		float coefDay = uDay;
		float coefNight = uNight;
		if(coefDay < 0.){coefDay = 0.;}
		if(coefNight < 0.){coefNight = 0.;}
		fFragColor = texture(uSkyTex, gTexCoords)*(min(coefDay+0.05,1.)) + texture(uNightTex, gTexCoords)*(min(coefNight+0.,1.));
		fFragColor += vec4(0.1f*abs(uTime),0.f,0.05f*(1.-abs(uTime)),0.f);
	}
}
