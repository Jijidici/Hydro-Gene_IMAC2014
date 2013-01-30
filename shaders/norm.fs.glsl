#version 330

#define SKYBOX 0
#define TRIANGLES 1

#define NORMAL 2
#define BENDING 3
#define DRAIN 4
#define GRADIENT 5
#define SURFACE 6

#define VEGET 7
#define DEBUG_BOX 8
#define DEBUG_TRI 9

in vec3 gPos;
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
uniform sampler2D uRockTex;
uniform sampler2D uPlantTex;
uniform sampler2D uTreeTex;
uniform sampler2D uPineTreeTex;
uniform sampler2D uSnowTreeTex;
uniform sampler2D uCloudsShadows;

uniform int uMode;
uniform int uChoice;
uniform int uFog;
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
	float coefDay = uDay;
	float coefNight = uNight;
	if(coefDay < 0.){coefDay = 0.;}
	if(coefNight < 0.){coefNight = 0.;}
	
	if(uMode == TRIANGLES){
		
		vec3 aColor = vec3(0.05);
		vec3 dColor;
		vec3 color;
		/* clouds shadowmap */
		vec2 shadowCloudTexcoord = gTexCoords;
		shadowCloudTexcoord.x += uTime;
		float cloudsColor = texture(uCloudsShadows, shadowCloudTexcoord).r;		
		
		/* compute ratios */		
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
		
		float ratio;
		
		/* case where displaying details */
		if(uChoice == VEGET){
			vec4 texel;
			if(ratioGradient < 0.8){
				if(ratioAltitude < 0.1 && ratioDrain < 0.1){
					texel = texture(uPlantTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uPlantTex, gTexCoords)*min(coefDay, 0.7);
				}
				else if(ratioGradient < 0.1 && ratioDrain < 0.8){
					texel = texture(uRockTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uRockTex, gTexCoords)*min(coefDay, 0.7);
				}
				else if(ratioAltitude > 0.5 && ratioAltitude <= 0.8){
					texel = texture(uPineTreeTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uPineTreeTex, gTexCoords)*min(coefDay, 0.7);
				}
				else if(ratioAltitude > 0.8){
					texel = texture(uSnowTreeTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uSnowTreeTex, gTexCoords)*min(coefDay, 0.7);
				}
				else{
					texel = texture(uTreeTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uTreeTex, gTexCoords)*min(coefDay, 0.7);
				}
				texel += vec4(0.1f*abs(uTime)*min(coefDay, 0.3),0.f,0.05f*(1.-abs(uTime))*min(coefNight, 0.3),0.f);
				fFragColor = texel;
				fFragColor *= (1. - cloudsColor)*coefDay;
			}else discard;
		}
		else if(uChoice == DEBUG_BOX){
			fFragColor = vec4(1.f, 0.f, 0.f, 1.f);
		}
		else if(uChoice == DEBUG_TRI){
			fFragColor = vec4(0.f, 1.f, 0.f, 1.f);
		}
		else{
			if(uChoice == BENDING){
				float ratio = gBending/uMaxBending;
				dColor = vec3(1.f - ratio, ratio, 1.f - ratio);
			}
			else if(uChoice == DRAIN){
				dColor = vec3(1.f - ratioDrain, 1.f - ratioDrain, ratioDrain);
			}
			else if(uChoice == GRADIENT){
				dColor = vec3(ratioGradient, 1.f - ratioGradient, 1.f - ratioGradient);
			}
			else if(uChoice == SURFACE){
				float ratio = gSurface/uMaxSurface;
				dColor = vec3(0.5f - ratio, ratio, 0.5f - ratio);
			}
			else if(uChoice == NORMAL){
				/* compute texture coefs */
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
				
				dColor = coefWater*texture(uWaterTex, gTexCoords).rgb;
				dColor += coefStone*texture(uStoneTex, gTexCoords).rgb;
				dColor += coefSnow*texture(uSnowTex, gTexCoords).rgb;
				dColor += coefSand*texture(uSandTex, gTexCoords).rgb;
				dColor += coefGrass*texture(uGrassTex, gTexCoords).rgb;
			}
			
			vec3 dColorSun = dColor + vec3(0.5f*abs(uTime),0.f,0.f);
			dColorSun *= (1. - cloudsColor)*coefDay;
			vec3 dColorMoon = dColor + vec3(0.f,0.f,0.25f);
			float dCoeffSun = max(0, dot(normalize(gNormal), -normalize(uLightSunVect)));
			float dCoeffMoon = max(0, dot(normalize(gNormal), -normalize(uLightMoonVect)));
			dCoeffSun *= 0.7;
			dCoeffMoon *= 0.1;
			color = vec3(0.8f, 0.8f, 0.8f) * (aColor + dColorSun*dCoeffSun + dColorMoon*dCoeffMoon);
			
			/* Simulate fog */
			if(uFog == 1){
				float fogDensity = 0.5;
				const float log2 = 1.442695;
				float fogZ = (gl_FragCoord.z+1.)/gl_FragCoord.w;
				float fogCoef = exp2(-fogDensity * fogDensity * fogZ * fogZ * log2);
				fogCoef = clamp(fogCoef, 0., 1.);
				vec3 fogColor = vec3(0.3);
				color = mix(fogColor, color, fogCoef);
			}
			fFragColor = vec4(color, 1.f);	
		}
	}
	else if(uMode == SKYBOX){
		/* Moving sky */
		vec2 cloudTexCoord = gTexCoords;
		cloudTexCoord.x -= uTime;
		
		vec3 nPos = normalize(gPos);
		vec4 skyColor = mix(vec4(0.466666667, 0.682352941, 0.82745098, 1.f), vec4(0.235294118, 0.586956522, 0.721568627, 1.f), nPos.y);
		vec4 cloudColor = texture(uSkyTex, cloudTexCoord);
		skyColor = skyColor*(1-cloudColor.a) + cloudColor*cloudColor.a;
		skyColor.r = min(skyColor.r, 1.);
		skyColor.g = min(skyColor.g, 1.);
		skyColor.b = min(skyColor.b, 1.);
		
		vec4 composDay = skyColor*(min(coefDay,1.));
		vec4 composNight = texture(uNightTex, gTexCoords)*(min(coefNight,1.));
		
		fFragColor = composDay + composNight;
		fFragColor += vec4(0.1f*abs(uTime),0.f,0.05f*(1.-abs(uTime)),0.f);
	}
}
