#version 330
#define SUN_RADIUS 0.05
#define HALO_RADIUS 0.07

in vec2 vPos;

uniform vec3 uPlanOr;
uniform vec3 uPlanU;
uniform vec3 uPlanV;
uniform vec3 uSunPos;
uniform float uTime;
uniform samplerCube uSkyTex;
uniform samplerCube uEnvmapTex;
uniform float uSampleStep;
uniform int uIsSkybox;
uniform int uIsInitialBlur;

out vec4 fFragColor;


vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
	const vec2 C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i = floor(v + dot(v, C.yyy) );
	vec3 x0 = v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	// x0 = x0 - 0.0 + 0.0 * C.xxx;
	// x1 = x0 - i1 + 1.0 * C.xxx;
	// x2 = x0 - i2 + 2.0 * C.xxx;
	// x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i);
	vec4 p = permute( permute( permute(
			 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		   + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
		   + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3 ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ ); // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
								dot(p2,x2), dot(p3,x3) ) );
 }

// Hue - Saturation - Luminance to Red - Green - Blue model conversion
vec3 HSLtoRGB(int h, float s, float l){
	vec3 color;	
	float c = (1. - abs(2 * l - 1.)) * s;
	float m = 1. * (l - 0.5 * c);
	float x = c * (1. - abs(mod(h / 60., 2) - 1.));
	
	h = h%360;
	if(h >= 0 && h < 60){ color = vec3(c + m, x + m, m); }
	else if(h >= 60 && h < 120){ color = vec3(x + m, c + m, m);	}
	else if(h >= 120 && h < 180){ color = vec3(m, c + m, x + m); }
	else if(h >= 180 && h < 240){ color = vec3(m, x + m, c + m); }
	else if(h >= 240 && h < 300){ color = vec3(x + m, m, c + m); }
	else if(h >= 300 && h < 360){ color = vec3(c + m, m, x + m); }
	else{ color = vec3(m, m, m); }
	return color;
}

vec3 getBluredTexel(vec3 position){
	vec3 color = vec3(0.f);
	int nbIt = 0;
	int blurWidth = 2;
	//~ /* case of X-Y plane */
	if(uPlanU.z == 0 && uPlanV.z == 0){
		float blurBeginX = position.x-blurWidth*uSampleStep;
		float blurBeginY = position.y-blurWidth*uSampleStep;
		float blurEndX = position.x+(blurWidth+1)*uSampleStep;
		float blurEndY = position.y+(blurWidth+1)*uSampleStep;
		for(float i=blurBeginX;i<blurEndX;i+=uSampleStep){
			for(float j=blurBeginY;j<blurEndY;j+=uSampleStep){
				color+= texture(uEnvmapTex, vec3(i, j, position.z)).rgb;
				++nbIt;
			}
		}
	}
	/* case of Z-Y plane */
	else if(uPlanU.x == 0 && uPlanV.x == 0){
		float blurBeginZ = position.z-blurWidth*uSampleStep;
		float blurBeginY = position.y-blurWidth*uSampleStep;
		float blurEndZ = position.z+(blurWidth+1)*uSampleStep;
		float blurEndY = position.y+(blurWidth+1)*uSampleStep;
		for(float k=blurBeginZ;k<blurEndZ;k+=uSampleStep){
			for(float j=blurBeginY;j<blurEndY;j+=uSampleStep){
				color+= texture(uEnvmapTex, vec3(position.x, j, k)).rgb;
				++nbIt;
			}
		}
	}
	/* case of X-Z plane */
	else{
		float blurBeginX = position.x-blurWidth*uSampleStep;
		float blurBeginZ = position.z-blurWidth*uSampleStep;
		float blurEndX = position.x+(blurWidth+1)*uSampleStep;
		float blurEndZ = position.z+(blurWidth+1)*uSampleStep;
		for(float i=blurBeginX;i<blurEndX;i+=uSampleStep){
			for(float k=blurBeginZ;k<blurEndZ;k+=uSampleStep){
				color+= texture(uEnvmapTex, vec3(i, position.y, k)).rgb;
				++nbIt;
			}
		}
	}
	color /= nbIt;
	return color;
}

void main(){
	vec3 absolutePos = normalize(uPlanOr + vPos.x*uPlanU + vPos.y*uPlanV);
	
	//Draw the skybox
	if(uIsSkybox == 1){
		float time = uTime*0.125f;
		float sunY = (uSunPos.y+1.)*0.5;
		float sunX = (uSunPos.x+1.)*0.5;
		float normPosX = (absolutePos.x+1)*0.5;
		float distanceToSun = distance(absolutePos, uSunPos);
		float satGradient = 0.18*pow((1. - absolutePos.y), 2);
		float lighnessGradient = 0.28 + 0.31*pow((1. - absolutePos.y), 2);

		/* sky color */
		vec3 skyColor;
		skyColor.x = 209;
		skyColor.y = 0.76 + satGradient;
		skyColor.z = 0.1 + lighnessGradient*((2-distanceToSun*0.5)*sunY);
		
		/* dawn effect */
		if(sunX > 0.9){
			vec3 dawnColor;
			dawnColor.x = 359;
			dawnColor.y = 0.82+satGradient;
			dawnColor.z = 0.325+lighnessGradient;
			float dawnCoef = (1-(1-sunX)/0.1)*max(0., normPosX);
			skyColor = mix(skyColor, dawnColor, dawnCoef);
		}
		
		/* twillight effect */
		if(sunX < 0.1){
			vec3 twillightColor;
			twillightColor.x = 384;
			twillightColor.y = 0.8+satGradient;
			twillightColor.z = 0.165+lighnessGradient;
			float twillightCoef = (1-(sunX)/0.1)*max(0., 1-normPosX);
			skyColor = mix(skyColor, twillightColor, twillightCoef);
		}
		
		/* stars noise */
		float starsCoef = 0.f;
		if(absolutePos.y > 0.f && sunY < 0.6){
			starsCoef = snoise(absolutePos*100);
			if(starsCoef < 0.99f) starsCoef *= pow(0.5f, (1.f-starsCoef)*15.f);
		}

		/* day */
		float sun_radius = 0.03 + (1-sunY)*0.05;
		float halo_radius = sun_radius + 0.08;
		if(distanceToSun <= sun_radius){
			skyColor.z = 1;
		}else if(distanceToSun <= halo_radius){
			skyColor.z += (1-skyColor.z)*pow((1-((distanceToSun-sun_radius)/0.08)), 3);
		}


		/* clouds noise */
		// where we draw clouds
		float cloudCoef = 0.f;
		if(absolutePos.y > 0.f && sunY > 0.2){
			float cloudZone = ((snoise((absolutePos+time)*2)+1.)/2.)*0.9;
			if(cloudZone < 0.f) cloudZone = 0.f;

			// clouds noise inside this zone
			absolutePos.x += time*2;
			cloudCoef = snoise(vec3(absolutePos.x*2., (1.-absolutePos.y)*4., absolutePos.z*0.75)*2)*3;
			//~ cloudCoef = cnoise(vec3(absolutePos.x*2., (1.-absolutePos.y)*4., absolutePos.z*0.75)*(0.3/cloudZone))*3;
			//~ cloudCoef = (cloudCoef + 2.f)*0.3f;
			cloudCoef = cloudCoef*cloudCoef/2.;

			cloudCoef *= cloudZone;
			if(cloudCoef < 0.f) cloudCoef = 0.f;
			if(cloudCoef > 1.f) cloudCoef = 1.f;

			/* no clouds on the horizon */
			if(absolutePos.y < 0.1){
				cloudCoef = cloudCoef*(absolutePos.y*10);
			}
		}
		
		float cloudTempo = max(sunY-0.2,0)/0.8; //sun position influence the density of clouds and stars
		float starsTempo = max((1.-sunY)-0.4, 0)/0.6;
		
		fFragColor = vec4( mix(HSLtoRGB(int(skyColor.x), skyColor.y, skyColor.z), vec3(1.f), cloudCoef*cloudTempo), 1.f );
		fFragColor = mix( fFragColor, vec4(1.), starsCoef*starsTempo);
		
		//~ test skybox
		//~ vec3 testColor = vec3(0.f);
		//~ if(absolutePos.x >= 0.) testColor.r = 1.f;
		//~ else testColor.g = 1.f;
		//~ if(absolutePos.z >= 0.) testColor.b = 1.;
		//~ fFragColor = vec4(testColor, 1.f);
	}
	//Draw the envmap
	else{
		if(uIsInitialBlur == 1){
			fFragColor = texture(uSkyTex, absolutePos);
		}else{
			fFragColor = vec4(getBluredTexel(absolutePos), 1.f);
		}
	}
}
