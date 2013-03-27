#version 330
#define SUN_RADIUS 0.05
#define HALO_RADIUS 0.08

in vec2 vPos;

uniform vec3 uPlanOr;
uniform vec3 uPlanU;
uniform vec3 uPlanV;
uniform vec3 uSunPos;
uniform float uTime;

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

float snoise(vec3 v){
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
	float time = uTime*0.125f;
	//~ float cloudZone = snoise(vec3(absolutePos.x + uTime, absolutePos.y, absolutePos.z)*2)*0.7;
	float cloudZone = snoise((absolutePos+time)*2)*0.9;
	
	float cloudCoef = 0.f;
	absolutePos.x += time*2;
	//~ absolutePos.z += time;
	cloudCoef = snoise(vec3(absolutePos.x*2, absolutePos.y*4, absolutePos.z*0.75)*3);
	cloudCoef = (cloudCoef + 1.f)/2.;
	
	cloudCoef *= cloudZone;
	
	
	float sunFragDistance = distance(absolutePos, uSunPos);
	if(sunFragDistance <= SUN_RADIUS){
		skyLightness = 1.f;
	}else if(sunFragDistance <= HALO_RADIUS){
		skyLightness += (1-skyLightness)*(1-(sunFragDistance-SUN_RADIUS)/(HALO_RADIUS-SUN_RADIUS));
	}
	
	fFragColor = vec4( mix(HSLtoRGB(skyHue, skySat, skyLightness), vec3(1.), cloudCoef), 1.f);
}
