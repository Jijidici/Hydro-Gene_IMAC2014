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

#define REFLECT_ANGLE -1.4835298642
#define CLOUD_HIGH 1.

in vec3 gPos;
in vec3 gNormal;
in vec2 gTexCoords;
in float gBending;
in float gDrain;
in float gGradient;
in float gSurface;
in float gAltitude;

uniform mat4 uMVPMatrix = mat4(1.f);
uniform mat4 uModelView = mat4(1.f);
uniform mat4 uInvViewMatrix = mat4(1.f);
uniform vec3 uFrontVector = vec3(0.);
uniform vec3 uSunDir = vec3(0.);

uniform samplerCube uSkyTex;
uniform samplerCube uEnvmapTex;
uniform sampler2D uCloudTex;
uniform sampler2D uGrassTex;
uniform sampler2D uWaterTex;
uniform sampler2D uWaterGroundTex;
uniform sampler2D uStoneTex;
uniform sampler2D uSnowTex;
uniform sampler2D uSandTex;
uniform sampler2D uRockTex;
uniform sampler2D uPlantTex;
uniform sampler2D uTreeTex;
uniform sampler2D uPineTreeTex;
uniform sampler2D uSnowTreeTex;

uniform int uMode;
uniform int uChoice;
uniform int uFog;
uniform float uWaterTime;
uniform float uMoveWaterTime;
uniform float uMaxBending = 0;
uniform float uMaxDrain = 0;
uniform float uMaxGradient = 0;
uniform float uMaxSurface = 0;
uniform float uMaxAltitude = 0;
uniform int uOcean;
uniform float uTerrainScale;


out vec4 fFragColor;

void main() {	
	if(uMode == TRIANGLES){
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
		
		float ratioBending;
		if(uMaxBending == 0){ ratioBending = 0;}
		else{ratioBending = gBending/uMaxBending;}
	
		/* case where displaying details */
		if(uChoice == VEGET){
			vec4 texel;
			if(ratioGradient < 0.8){
				if(ratioAltitude < 0.1 && ratioDrain < 0.1){
					texel = texture(uPlantTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uPlantTex, gTexCoords);
				}
				else if(ratioGradient < 0.1 && ratioDrain < 0.8){
					texel = texture(uRockTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uRockTex, gTexCoords);
				}
				else if(ratioAltitude > 0.5 && ratioAltitude <= 0.8){
					texel = texture(uPineTreeTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uPineTreeTex, gTexCoords);
				}
				else if(ratioAltitude > 0.8){
					texel = texture(uSnowTreeTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uSnowTreeTex, gTexCoords);
				}
				else{
					texel = texture(uTreeTex, gTexCoords)*0.1;
						if(texel.a <0.01){
							discard;
					}
					texel += texture(uTreeTex, gTexCoords);
				}
				fFragColor = texel;
			}else discard;
		}
		else if(uChoice == DEBUG_BOX){
			fFragColor = vec4(1.f, 0.f, 0.f, 1.f);
		}
		else if(uChoice == DEBUG_TRI){
			fFragColor = vec4(0.f, 1.f, 0.f, 1.f);
		}
		else{
			// REALISTIC ILLUMINATION
			vec3 aColor = vec3(0.05);
			
			/* Compute diffus coefficient */
			vec3 envMapSample = texture(uEnvmapTex, gNormal).xyz;
			
			/* Compute the diffus color */
			vec3 dColor = vec3(0.);
			if(uChoice == BENDING){
				dColor = vec3(1.f - ratioBending, ratioBending, 1.f - ratioBending);
			}
			else if(uChoice == DRAIN){
				dColor = vec3(1.f - ratioDrain, 1.f - ratioDrain, ratioDrain);
			}
			else if(uChoice == GRADIENT){
				dColor = vec3(ratioGradient, 1.f - ratioGradient, 1.f - ratioGradient);
			}
			else if(uChoice == SURFACE){
				dColor = vec3(0.5f - ratioSurface, ratioSurface, 0.5f - ratioSurface);
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
				
				/* Compute water's diffus color */
				vec4 dWater = vec4(0.f);
				if(coefWater>0.3f){
					/* Normal Mapping */
					vec2 HMCoord = gTexCoords*10 + uMoveWaterTime;
					mat2 rotatHMcoord;
					rotatHMcoord[0][0] = 0.965925826;
					rotatHMcoord[0][1] = 0.258819045;
					rotatHMcoord[1][0] = -0.258819045;
					rotatHMcoord[1][1] = 0.965925826;
					vec2 HMCoordAlt = -(rotatHMcoord*HMCoord);

					vec3 bump = normalize(texture(uWaterTex, HMCoord).xyz*2.f-1.f);
					vec3 bumpAlt = normalize(texture(uWaterTex, HMCoordAlt).xyz*2.f-1.f);
					vec3 bumpedNormal = uWaterTime*bump + (1.-uWaterTime)*bumpAlt + normalize(gNormal);
					envMapSample = texture(uEnvmapTex, bumpedNormal).xyz;
					vec4 N = normalize(uModelView*vec4(bumpedNormal, 0.f));
				
					/* fragment position in camera space */
					vec4 P = normalize(uModelView*vec4(gPos, 1.f));
					
					vec4 ref = reflect(P, N);
					ref.x /= ref.w;
					ref.y /= ref.w;
					ref.z /= ref.w;
					ref.w = 0.;
					ref = uInvViewMatrix*ref;
					mat4 reflectRotMat = mat4(1.f);
					reflectRotMat[1][1] = cos(REFLECT_ANGLE);
					reflectRotMat[2][2] = reflectRotMat[1][1];
					reflectRotMat[1][2] = sin(REFLECT_ANGLE);
					reflectRotMat[2][1] = - reflectRotMat[1][2];
					ref = reflectRotMat*ref;
					
					vec4 viewModelFrontVector = normalize(uModelView*vec4(uFrontVector,0.));
					
					float dCoeffRef = 0.4*pow(min(max(0, dot(normalize(gNormal), -normalize(uFrontVector))), 1.f),2)*pow(dot(P, viewModelFrontVector),2);
					
					dWater = (1-dCoeffRef)*texture(uSkyTex, ref.xyz) + dCoeffRef*texture(uWaterGroundTex, gTexCoords/4.);
					dWater.r = min(1,max(0,dWater.r));
					dWater.g = min(1,max(0,dWater.g));
					dWater.b = min(1,max(0,dWater.b));
				}
	
				dColor = coefWater*dWater.rgb;
				dColor += coefStone*texture(uStoneTex, gTexCoords).rgb;
				dColor += coefSnow*texture(uSnowTex, gTexCoords).rgb;
				dColor += coefSand*texture(uSandTex, gTexCoords).rgb;
				dColor += coefGrass*texture(uGrassTex, gTexCoords).rgb;
			}
			
			//Final light
			float dCoeff = (envMapSample.r+envMapSample.g+envMapSample.b)/3.;
			vec3 lightColor = mix(vec3(1.f), envMapSample, 0.4f);
			vec3 color = lightColor*(aColor+ dColor*dCoeff);
			
			/* Simulate fog */
			if(uFog == 1){
				float fogDensity = 0.05f;
				const float log2 = 1.442695;
				float fogZ = (gl_FragCoord.z+1.)/gl_FragCoord.w;
				float fogCoef = exp2(-fogDensity * fogDensity * fogZ * fogZ * log2);
				fogCoef = clamp(fogCoef, 0., 1.);
				vec3 fogColor = vec3(0.3);
				color = mix(fogColor, color, fogCoef);
			}
			
			/* Clouds shadow */
			float cloudShadow = 0.;
			/* if the sun is above the clouds */
			vec2 fragPos = gPos.xz;
			if(uOcean == 1){
				fragPos *= 20;
			}
			vec2 cloudShadowTexCoords = fragPos;
			cloudShadowTexCoords.x = (cloudShadowTexCoords.x-0.15*abs(uSunDir.x))*(1. - 0.3*abs(uSunDir.x));
			cloudShadow = texture(uCloudTex, cloudShadowTexCoords).r * max(0., -uSunDir.y);
			
			color.r = max(0.f, color.r-cloudShadow);
			color.g = max(0.f, color.g-cloudShadow);
			color.b = max(0.f, color.b-cloudShadow);
			
			fFragColor = vec4(color, 1.f);
		}
	}
	else if(uMode == SKYBOX){
		fFragColor = texture(uSkyTex, gPos);
	}
}
