#version 150
// The MIT License
// Copyright © 2020 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// This shader adapts code and concepts from several works by Inigo Quilez
// Exact euclidean distance to a box frame
// Happy Jumping
// Ladybug
//
// List of other 3D SDFs: https://www.shadertoy.com/playlist/43cXRl
//
// and https://iquilezles.org/articles/distfunctions

/////////////////////////////////////////////////////////////////////
// SWITCHES
/////////////////////////////////////////////////////////////////////

#define NATIVE 1
#define DITHERING 1
#define PROBLEMS 1
#define DEBUG_ITERATIONS 0
#define DEBUG_NORMALS 0
#define DEBUG_NORMALS_DIFF 0
#define ANALYTICAL_NORMALS 0
#define DEMO_DATA_LOCALITY 0

/////////////////////////////////////////////////////////////////////
// MATERIALS
/////////////////////////////////////////////////////////////////////
#define MAT_NONE 0.0
#define MAT_INTERIOR MAT_TUL_IN_BULB
    #define MAT_SCENE MAT_TULIP_BULB

#define MAT_TULIP_BULB  1.0
#define MAT_TULIP_LEAF  2.0
#define MAT_TULIP_STEM  3.0
#define MAT_EARTH       21.0

#define MAT_TUL_IN_BULB 11.0
#define MAT_TUL_IN_LEAF 12.0
#define MAT_TUL_IN_STEM 13.0
#define MAT_SKY         22.0
#define MAT_ERROR       23.0
#define MAT_EARTH_INTERIOR 31.0

#define isMaterial(Value, Material) ((Value) > (Material) - 0.5 && (Value) < (Material) + 0.5)

#define PI 3.14159

#if DEMO_DATA_LOCALITY
const float ditherPattern[64] = float[](0,  32, 8,  40, 2,  34, 10, 42,
                                        48, 16, 56, 24, 50, 18, 58, 26,
                                        12, 44, 4,  36, 14, 46, 6,  38,
                                        60, 28, 52, 20, 62, 30, 54, 22,
                                        3,  35, 11, 43, 1,  33, 9,  41,
                                        51, 19, 59, 27, 49, 17, 57, 25,
                                        15, 47, 7,  39, 13, 45, 5,  37,
                                        63, 31, 55, 23, 61, 29, 53, 21);
#endif

#if NATIVE
    uniform float iTime;
    uniform vec3  iResolution;
    uniform vec3  co;
    uniform vec3  cx;
    uniform vec3  cy;
    uniform vec3  cz;
    
    out vec4 fragColor;
#else
#endif

struct sdfres {
	float dist;
	//vec3  nor;
	float mat;
};

/////////////////////////////////////////////////////////////////////

float dot2(vec3 v) {
    return dot(v,v);
}

float lerp (float a, float t, float b) {
    return (1.0-t)*a + t*b;
}

vec2 lerp(vec2 a, float t, vec2 b) {
    float x = lerp(a.x, t, b.x);
    float y = lerp(a.y, t, b.y);
    return vec2(x, y);
}

vec3 lerp(vec3 a, float t, vec3 b) {
    float x = lerp(a.x, t, b.x);
    float y = lerp(a.y, t, b.y);
    float z = lerp(a.z, t, b.z);
    return vec3(x, y, z);
}

// From iq: return smoothstep and its derivative
vec2 smoothstepd( float a, float b, float x)
{
	if( x<a ) return vec2( 0.0, 0.0 );
	if( x>b ) return vec2( 1.0, 0.0 );
    float ir = 1.0/(b-a);
    x = (x-a)*ir;
    return vec2( x*x*(3.0-2.0*x), 6.0*x*(1.0-x)*ir );
}

#if 1 || QUARANTINE
float hash1( vec2 p )
{
    p  = 50.0*fract( p*0.3183099 );
    return fract( p.x*p.y*(p.x+p.y) );
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 w = fract(x);
    #if 1
    vec2 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    #else
    vec2 u = w*w*(3.0-2.0*w);
    #endif

    float a = hash1(p+vec2(0,0));
    float b = hash1(p+vec2(1,0));
    float c = hash1(p+vec2(0,1));
    float d = hash1(p+vec2(1,1));
    
    return -1.0+2.0*(a + (b-a)*u.x + (c-a)*u.y + (a - b - c + d)*u.x*u.y);
}

float fbm_9( in vec2 x, mat2 m2)
{
    float f = 1.9;
    float s = 0.55;
    float a = 0.0;
    float b = 0.5;
    for( int i=0; i<4; i++ )
    {
        float n = noise(x);
        a += b*n;
        b *= s;
        x = f*m2*x;
    }
    
	return a;
}

float hash1( float n )
{
    return fract( n*17.0*fract( n*0.3183099 ) );
}

float hash1( vec3 p )
{
    p  = 50.0*fract( p*0.3183099 );
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    #if 1
    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    #else
    vec3 u = w*w*(3.0-2.0*w);
    #endif
    


    float n = p.x + 317.0*p.y + 157.0*p.z;
    
    float a = hash1(n+0.0);
    float b = hash1(n+1.0);
    float c = hash1(n+317.0);
    float d = hash1(n+318.0);
    float e = hash1(n+157.0);
	float f = hash1(n+158.0);
    float g = hash1(n+474.0);
    float h = hash1(n+475.0);

    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return -1.0+2.0*(k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z);
}

float fbm_9( in vec3 x, mat3 m3)
{
    float f = 1.9;
    float s = 0.55;
    float a = 0.0;
    float b = 0.5;
    for( int i=0; i<4; i++ )
    {
        float n = noise(x);
        a += b*n;
        b *= s;
        x = f*m3*x;
    }
    
	return a;
}

// TODO(isuru): Test
vec3 norVerticalCapsule(vec3 p, float h) {
#if 0
	if (p.y < 0.0) {
		return normalize(p);
	}
	else if (p.y > h) {
		p.y -= h;
		return normalize(p);
	}
	else {
		p.y -= p.y;
		return normalize(p);
	}
#else
	p.y -= clamp( p.y, 0.0, h);
	return normalize(p);
#endif
}

sdfres sdVerticalCapsule( vec3 p, float r, float h )
{
  sdfres res;
  p.y -= clamp( p.y, 0.0, h );
  res.dist = length(p)-r;
  return res;
}

float sdCylinder(vec3 p, float r) {
    return length(p.xz)-r;
}

#if 0
float sdVerticalCylinderInterior(vec3 pos, vec3 c, float r, vec3 forward) {
    // We assume the ray will never be beyond the cylinder which, for the way this is being used, is true    
    vec3 right = cross(forward, vec3(0.0, 1.0, 0.0));
    float px = dot(pos, right);
    float pz = dot(pos, forward);
    float cx = dot(c, right);
    float cz = dot(c, forward);
    float pclen = length(vec2(px-cx, pz-cz));
    if (pz <= cz) {
        return length(vec2(r-abs(px-cx), pz-cz));
    }
    else {
        float dx = px - (cx+r*(px-cx)/pclen);
        float dz = pz - (cz+r*(pz-cz)/pclen);
        return length(vec2(dx, dz));
    }
}
#else
float sdVerticalCylinderInterior(vec3 pos, vec3 c, float r, vec3 forward) {
    // We assume the ray will never be beyond the cylinder which, for the way this is being used, is true    
    vec2 right = vec2(-forward.z, forward.x);
    float px = dot(pos.xz, right);
    float pz = dot(pos.xz, forward.xz);
    float cx = dot(c.xz, right);
    float cz = dot(c.xz, forward.xz);
    
    float pcx  = px-cx;
    float pcz  = pz-cz;
    float pczm = max(pz,cz)-cz;
    float t = r/length(vec2(pcx, pczm));
    
    return length(vec2(pcx-t*pcx, pcz-t*pczm));
}
#endif

// From iq's sdRoundCone
float sdTulipBulb(vec3 pos, float ra, float rb, float rc, float hab, float hbc) {
	float r1, r2, h;
	vec2 q;
	if (pos.y < hab) {
		r1 = ra;
		r2 = rb;
		h = hab;
		q = vec2(length(pos.xz), pos.y);
	}
	else {
		r1 = rb;
		r2 = rc;
		h = hbc;
		pos.y -= hab;
		q = vec2(length(pos.xz), pos.y);
	}

	float b = (r1-r2)/h;
	float a = sqrt(1.0-b*b);
	float k = dot(q, vec2(-b, a));
	if( k<0.0 )      return length(q) - r1;
	else if( k>a*h ) return length(q-vec2(0.0,h)) - r2;
	else             return dot(q, vec2(a,b) ) - r1;
}

vec3 norTulipBulb(vec3 pos, float ra, float rb, float rc, float hab, float hbc) {
	float r1, r2, h;
	if (pos.y < hab) {
		r1 = ra;
		r2 = rb;
		h = hab;
	}
	else {
		r1 = rb;
		r2 = rc;
		h = hbc;
		pos.y -= hab;
	}

	float b = (r1-r2)/h;
	float a = sqrt(1.0-b*b);
	vec2 q = vec2(length(pos.xz), pos.y);
	float k = dot(q, vec2(-b, a));
	if( k<0.0 )      return normalize(pos);
	else if( k>a*h ) return normalize(pos-vec3(0.0, h, 0.0));
	else             return normalize(cross(vec3(-b*pos.x/q.x, a, -b*pos.z/q.x), vec3(-pos.z, 0.0, pos.x)));
}

vec3 opWind(in vec3 p, float k, float h)
{
    float c = cos(k*(p.y-h));
    float s = sin(k*(p.y-h));
    mat3 m = mat3(1, 0, 0, 0, c,-s, 0, s,c);
    vec3 q = m * p;
    q.z += k * h;
    return q;
}

vec3 opRep( in vec3 p, in vec3 c)
{
    vec3 q = mod(p+0.5*c,c)-0.5*c;
    return q;
}

vec3 opWind2(in vec3 p, float k, float h)
{
    float c = cos(k*(p.y-h));
    float s = sin(k*(p.y-h));
    mat3 m = mat3(1, 0, 0, 0, c,-s, 0, s,c);
    vec3 q = m * p;
    q.z += k * h;
    return q;
}

vec3 opRep2( in vec3 p, in vec3 c)
{
    vec3 q = mod(p+0.5*c,c)-0.5*c;
    return q;
}

vec3 opRepLim( in vec3 p, in float c, in vec3 l )
{
    vec3 q = p-c*clamp(round(p/c),-l,l);
    return q;
}

vec2 sdLineOriY( in vec3 p, in float b )
{
    float h = clamp( p.y, 0.0, b );
    p.y -= h;
    return vec2( length(p), h );
}

#define BULB_SCALE 0.8
#define BULB_RA 0.05*BULB_SCALE
#define BULB_RB 0.20*BULB_SCALE
#define BULB_RC 0.08*BULB_SCALE
#define BULB_HAB 0.20*BULB_SCALE
#define BULB_HBC 0.20*BULB_SCALE
#define STEM_H 1.4
#define STEM_R 0.033
#define TUL_OFFSET -0.5
sdfres sdTulip( in vec3 pos) {
    sdfres res;
	res.dist = 1e20;
	res.mat  = MAT_NONE;

	// @Incomplete(isuru): This is doing some fucked up slicing for some reason
	float dist2tul = sdVerticalCapsule(pos, BULB_RB, STEM_H+BULB_HAB+BULB_HBC+BULB_RC).dist; 
	//if (dist2tul < 0.25)
	{
    //bulb
//	if (false)
    {
	float tmp = sdTulipBulb(pos-vec3(0.0, STEM_H, 0.0), BULB_RA, BULB_RB, BULB_RC, BULB_HAB, BULB_HBC);
	if (tmp < res.dist) {
		res.dist = tmp;
		res.mat = MAT_TULIP_BULB;
	}
    }

    //stem
//	if (false)
	{
    sdfres tmp = sdVerticalCapsule(pos, STEM_R, STEM_H);
    if (tmp.dist < res.dist) {
		res = tmp;
		res.mat = MAT_TULIP_STEM;
	}
    }
	}
    
    return res;
}

// @Incomplete(isuru)
vec3 norTerrain(vec3 pos) {
	vec3 q = opRep(0.8*pos, vec3(4.*PI, 0.0, 4.*PI));
	float dyx = 0.045*cos(q.x+q.z);
	float dyz = 0.045*cos(q.x+q.z);
	return cross(vec3(1.0, dyx, 0.0), vec3(0.0, dyz, 1.0));
}

float terrainHeight(vec3 pos) {
//	return 0.0;
	vec3 q = opRep(0.8*pos, vec3(4.*PI, 0.0, 4.*PI));
	float d = 0.1*sin(q.x + q.z)-0.1;
	
	vec2 p = pos.xz;
	mat2 m2 = mat2(3.,4.,-3.,4.)/5.;
	return d+0.05*fbm_9(p,m2);
}

// @Incomplete(isuru): Prob the reason why tulips fade out weirdly 
float sdTerrain(in vec3 pos) {
    float res = 1e20;
    
    float terrain = terrainHeight(pos)-2.1;
//    float d = max((pos.y-0.45*terrain), -(pos.y-terrain+0.5));
    float d = max((pos.y+0.45*terrain*terrain), -(pos.y-terrain+0.5));
    //d = pos.y-terrain;
    res = d;
    return res;
}

float sdSphere(vec3 pos, vec3 c, float r) {
	return length(pos-c)-r;
}

#define REP_COUNT 5.
#define REP_PAD 2.
#define REP_LIM floor(REP_COUNT/2.)
#define INNER_REP 0.7
#define OUTER_REP (REP_PAD+REP_COUNT)*INNER_REP

vec3 xformTulips(in vec3 pos) {
	float th = terrainHeight(pos);
	vec3 q = pos + vec3(0.0, 0.45*th*th+0.2, 0.0);
    q = opRep(q, vec3(OUTER_REP, 0.0, INNER_REP));
    q = opRepLim(q, INNER_REP, vec3(REP_LIM, 0.0, REP_LIM));

#if 0&& PROBLEMS
	vec2 m = floor(q.xz/(INNER_REP-BULB_RB));
	float n = 107.*m.x + 113*m.y + 131;
	// TODO(isuru): Different rand functionsw
	vec2 c = 2*m - 0.5 + fract(n*fract(n/vec2(PI, 2.13241)));
	q = q-vec3(c.x, 0.0, c.y);
#endif

    float ws = 0.05*(0.5*sin(0.7*iTime+0.5)+0.5)
             + 0.07*(0.5*sin(0.4*iTime+0.5)+0.4)
             + 0.003*(sin(1.3*iTime));
    q = opWind(q, 1.*ws, -4.3);

    q.y += STEM_H-2.*TUL_OFFSET;
    return q;
}

sdfres mapScene( in vec3 pos) {
    sdfres res;
	res.dist = 1e20;
	res.mat = MAT_NONE;    

    // terrain
    {
    float tmp = sdTerrain(pos);
    if (tmp < res.dist) {
		res.dist = tmp;
		res.mat  = MAT_EARTH;
	}
    }
    
#if 1
	//grass
	{
	float rep = INNER_REP*8. + 0.001*hash1(pos.xz);
	vec3 gp = pos - vec3(0.5, 0.0, 0.0);
	gp.y *= 2.0;
	gp = opRep(gp, vec3(rep, 0.0, rep));
	gp += vec3(0.0, terrainHeight(pos)+0.2, 0.0);
	gp.y += 6.0;

	gp.y -= 0.2;
	vec2 ld = sdLineOriY(gp - vec3(1.0, 0.0, 0.0), 0.03);
	float tmp = 0.5*ld.x - (1.0-0.99*ld.y);
	if (tmp < res.dist) {
		res.dist = tmp;
		res.mat = MAT_TULIP_LEAF;
	}
	}
#endif

    // tulips
    {
	// TODO(isuru): Inconsistency between terrain height here and in sdterrain?
	vec3 q = xformTulips(pos);
	sdfres tmp = sdTulip(q);
	if (tmp.dist < res.dist) {
		res = tmp;
	}
	}

    return res;
}

sdfres map( in vec3 pos, vec3 co, vec3 cz) {    
    sdfres res;
	res.dist = 1e20;
	res.mat  = MAT_NONE;
    
    // bounding cylinder
    vec3 cf = normalize(vec3(cz.x, 0.0, cz.z));
    vec3 vc = co+8.0*cf;
    float vr = 3.0;
    res.dist = sdCylinder(pos-vc, vr);
    if (res.dist < 0.05)
	{
    res.dist = 1e20;
    
        
    // scene
    {
    sdfres tmp = mapScene(pos);
    if (tmp.dist < res.dist) res = tmp;
    }

    // sky
//	if (false)
    {
    float tmp = (sdVerticalCylinderInterior(pos, vc, vr, cf));
    if (tmp < res.dist) {
        res.dist = tmp;
        res.mat = MAT_SKY;
    }
    }
    
    // vignette
    {
    float tmp = (sdCylinder(pos-vc,vr));
    if (tmp > res.dist) {
        res.dist = tmp;
        if (abs(res.mat-5.0) < 5.0) res.mat += 10.0; // Change material to inner material
        if (isMaterial(res.mat, MAT_EARTH)) res.mat += 10.0; // Change material to inner material
    }
    }
    }
    
    return res;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos, vec3 ro, vec3 cz)
{
    vec2 e = vec2(1.0,-1.0)*0.5773;
    const float eps = 0.0005;
    return normalize( e.xyy*map( pos + e.xyy*eps ,ro,cz).dist + 
					  e.yyx*map( pos + e.yyx*eps ,ro,cz).dist + 
					  e.yxy*map( pos + e.yxy*eps ,ro,cz).dist + 
					  e.xxx*map( pos + e.xxx*eps ,ro,cz).dist );
}

bool occluded(vec3 ro, vec3 rd, float tmax) {
    bool occluded = false;
    float t = 0.0;
    for( int i=0; i<256; i++ )
    {
        vec3 pos = ro + t*rd;
        sdfres h = mapScene(pos);
        if( abs(h.dist)<0.0001) {
            occluded = true;
            break;
        }
        else if (t > tmax) {
            break;
        }
        t += h.dist;
    }
    return occluded;
}

float
matSky(in vec2 p, vec3 rd) {

	float res = 0.0;

	float th = 1.3;
	float sinth = sin(th);
	float costh = cos(th);

	p = (rd.xy+1.0) * 2.;
	mat2 m2;
	if (p.x < 5.) {
	m2  = mat2(5.0, 12.0, -5.0, 12.0)/13.0;
	}
	else {
	m2 = mat2(3.0, 4.0, -3.0, 4.0)/5.0;
	}
	res = fbm_9(p + vec2(fbm_9(p + vec2(fbm_9(p, m2)), m2)), m2);
	m2 = mat2(sinth, costh, -sinth, costh);
	p = (rd.zy-1.0) * 2.;
	res = res * fbm_9(p + vec2(fbm_9(p, m2)), m2);
	sinth = sin(2.0);
	costh = cos(2.0);
	m2 = mat2(sinth, costh, -sinth, costh);
	sinth = sin(0.003*iTime);
	costh = cos(0.003*iTime);
	mat2 m2pert = mat2(sinth, costh, -sinth, costh);
	res = fbm_9(p + vec2(fbm_9(p, m2pert)), m2) - res;

	res = 5.0*min(res, 0.55);
	res = 1.0-res*res;
	res = min(res, 0.5);
	res *= smoothstepd( 0.1, 0.4, res).x;
	//res *= 0.1;
	return res;
}
#endif

#if NATIVE
void main() {
#else
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
#endif

#if NATIVE
    vec2 screenXY = gl_FragCoord.xy;
	screenXY.y += 0.01*iResolution.y;
#else
    vec2 screenXY = fragCoord.xy;
#endif

	// White fill + early exit if we aren't going to hit the scene
	float sx = iResolution.x/1080;
	float sy = iResolution.y/1920;
	if (   screenXY.x < 245*sx
	    || screenXY.x > iResolution.x-245*sx
	    || screenXY.y < 550*sy) {
		fragColor = vec4(1.);
		return;
	}

#if !NATIVE
     // camera movement	
	float an = 0.5*(iTime-10.0);
    an = lerp(12.2, 0.5*sin(iTime)+0.5,12.350);
    an = 12.2;
	vec3 co = 2.2*vec3( 1.0*cos(an), 0.0, 1.0*sin(an) );
    co.x += 0.1*(iTime + 200.0);  
	// camera matrix
    vec3 cz = normalize(-co);
    vec3 cx = normalize( cross(cz,vec3(0.0,1.0,0.0) ) );
    vec3 cy = normalize( cross(cx,cz));
#endif
	vec3 cpos = co + vec3(0.0, 0.0, 0.0);
    vec3 fr = normalize(vec3(cz.x, 0.0, cz.z));
   
    // render    
    vec3 tot = vec3(0.0);    
    vec2 p = 2.0*(2.0*screenXY-iResolution.xy)/iResolution.y;

	// create view ray
	vec3 rd = normalize( p.x*cx + p.y*cy + 1.5*cz );

    // raymarch
    sdfres res;
    const float tmax = 15.0;
    float t = 0.0;
	float iterations = 0.;
    for( int i=0; i<256; i++ )
       {
            vec3 pos = cpos + t*rd;
            sdfres h = map(pos,cpos,fr);
            if( abs(h.dist)<0.0001*t || t>tmax ) {
                res = h;
                break;
            }
            t += h.dist;
			iterations += 1;
        }    
    
        // shading/lighting	
        vec3 col = vec3(0.0);
        if( t<tmax )
        {

#if DEBUG_ITERATIONS
		col = vec3(clamp(iterations/256., 0.0, 0.5));
#else
            vec3 pos = cpos + t*rd;
	        vec3 sundir = normalize(vec3(1.0,-1.0,1.0));
			vec3 nor;
			float dif;
			float amb;

			float th = terrainHeight(pos) - 2.1;
			float thl = th-0.5;
			float thu = -0.45*th*th;
			bool overground = pos.y > thu;
			bool underearth = pos.y < thl;

			if (res.mat == MAT_TUL_IN_LEAF) {
				res.mat = MAT_EARTH_INTERIOR;
			}

			if (underearth) {
				res.mat = MAT_NONE;
			}
			else if (!overground) {
				res.mat = MAT_EARTH_INTERIOR;
			}

            if (isMaterial(res.mat, MAT_NONE) || isMaterial(res.mat, MAT_SKY)) {
				if (screenXY.x > 245.0 * sx
					&& screenXY.x < iResolution.x - 245.0 * sx) {
					
					if (overground) {
						res.mat = MAT_SKY;
					}
				}
            }

			if (res.mat < MAT_TULIP_BULB + 0.5) {
				vec3 q = xformTulips(pos);
				q.y -= STEM_H;
#if ANALYTICAL_NORMALS
			    nor = norTulipBulb(q, BULB_RA, BULB_RB, BULB_RC, BULB_HAB, BULB_HBC);
#else
				nor = calcNormal(pos, cpos, fr);
#endif


  	          dif = clamp( dot(nor,vec3(0.57703)), 0.0, 1.0 );
     	       amb = 0.5 - 0.5*dot(nor,sundir);
            
                col = vec3(0.3,0.4,0.5)*amb + vec3(0.85,0.75,0.65)*dif;

#if DEBUG_NORMALS_DIFF
				col = norTulipBulb(q, BULB_RA, BULB_RB, BULB_RC, BULB_HAB, BULB_HBC);
				col -= calcNormal(pos, cpos, fr);
#endif
            }
            else if (res.mat < MAT_TULIP_LEAF + 0.5) { // @Incomplete(isuru)
				vec3 q = xformTulips(pos);
#if ANALYTICAL_NORMALS
				nor = vec3(0.0);
#else
				nor = calcNormal(pos, cpos, fr);
#endif
 	 	      dif = clamp( dot(nor,vec3(0.57703)), 0.0, 1.0 );
	     	   amb = 0.5 - 0.5*dot(nor,sundir);

                col = vec3(0.2,0.3,0.4)*amb + vec3(0.85,0.75,0.65)*dif;
            }
            else if (res.mat < MAT_TULIP_STEM + 0.5) {
				vec3 q = xformTulips(pos);
#if ANALYTICAL_NORMALS
				nor = norVerticalCapsule(q, STEM_H);
#else
				nor = calcNormal(pos, cpos, fr);
#endif
 	 	      dif = clamp( dot(nor,vec3(0.57703)), 0.0, 1.0 );
	     	   amb = 0.5 - 0.5*dot(nor,sundir);

                col = vec3(0.1,0.15,0.15)*amb + vec3(0.85,0.75,0.65)*dif;
            }
            else if (res.mat < MAT_EARTH + 0.5) { // @Incomplete(isuru)
				nor = calcNormal(pos, cpos, fr);
 	 	      dif = clamp( dot(nor,vec3(0.57703)), 0.0, 1.0 );
	     	   amb = 0.5 - 0.5*dot(nor,sundir);

                col = vec3(0.4,0.5,0.6)*amb + vec3(0.85,0.75,0.65)*dif;
                if (nor.y > 0.05)
                {
#if NATIVE
					mat3 m3 = mat3(3.0, 0.0, 4.0, 0.0, 1.0, 0.0, -3.0, 0.0, 4.0)/5.0;
					col += 0.1*fbm_9(10.0*pos, m3);
#else
					col = lerp(col, 0.5, texture(iChannel0, mod(0.001*screenXY-0.05*cpos.zx, 512.0)).xyz);
#endif
                    col *= col;
                }
                else {
                    col = vec3(0.5);
					col *= 0.3*col;
                }
            }
            else if (res.mat < MAT_SKY + 0.5) {
				if (screenXY.y/iResolution.y > 0.35) {
// @Incomplete(isuru)
					vec2 skyp = vec2(screenXY.x+iResolution.x/2., screenXY.y-0.3);
					float skyvalue = matSky(0.0005*skyp, rd);
					skyvalue -= 0.2*sqrt(skyvalue);
					//skyvalue = sqrt(skyvalue);
					//skyvalue = 1.0 - skyvalue;

					float bcmin = 0.30;
					float bcmax = 0.50;
					float bcl = length(screenXY-iResolution.xy*vec2(0.5, 0.2))/iResolution.y;
					bcl = clamp(bcmin, bcl, bcmax);
					float bt = (bcl-bcmin)/(bcmax-bcmin);

					float tcmin = 0.3;
					float tcmax = 0.6;
					float tcl = length(screenXY-iResolution.xy*vec2(0.5, 1.1))/iResolution.y;
					tcl = clamp(tcmin, tcl, tcmax);
					float tt = (tcl-tcmin)/(tcmax-tcmin);

					col = vec3(mix(1.0, mix(1.0, skyvalue, tt), bt));
				}
            }
			else if (isMaterial(res.mat, MAT_EARTH_INTERIOR)) {
				mat3 m3 = mat3(3.0, 0.0, 4.0, 0.0, 1.0, 0.0, -3.0, 0.0, 4.0)/5.0;
				col = vec3(0.1, 0.15, 0.2);
				col += 0.1*fbm_9(8.0*pos, m3);
			}
            else if (res.mat < MAT_ERROR + 0.5) {
				//col = vec3(0.1);
			}

#if PROBLEMS
            bool dark = dot(nor, sundir) > 0.0 || occluded(cpos+t*rd+0.001*nor,-sundir, 5.0);
            if (dark) {
                if (isMaterial(res.mat, MAT_EARTH)) {
//                if (res.mat < MAT_SKY - 0.5) {
					col -= 0.15;
					col = vec3(0.25);
				}
            }
#endif

#if DEBUG_NORMALS
				col = nor;
#endif

#endif
        }

        // gamma
        col = sqrt( col );
	    tot += col;
  
    float lum = dot(vec3(0.2126, 0.7152, 0.0722), tot);
	if (lum == 0.0) {
		fragColor = vec4(1.0); // Whiteout background
	}
	else { 
    	vec3 gra1 = vec3(250.0, 185.0, 85.0);
    	vec3 gra2 = vec3(213.0, 75.0, 95.0);
    	vec3 graColour = vec3(lerp(gra1, 1.0-(screenXY.y/iResolution.y-0.1), gra2))/255.;
		

#if !DEMO_DATA_LOCALITY
const float ditherPattern[64] = float[](0,  32, 8,  40, 2,  34, 10, 42,
                                        48, 16, 56, 24, 50, 18, 58, 26,
                                        12, 44, 4,  36, 14, 46, 6,  38,
                                        60, 28, 52, 20, 62, 30, 54, 22,
                                        3,  35, 11, 43, 1,  33, 9,  41,
                                        51, 19, 59, 27, 49, 17, 57, 25,
                                        15, 47, 7,  39, 13, 45, 5,  37,
                                        63, 31, 55, 23, 61, 29, 53, 21);
#endif

#if DITHERING == 0
    // NO DITHERING
    fragColor = vec4( tot, 1.0 );

#elif DITHERING == 1
    // BAYER DITHERING
	vec2 xy = screenXY-floor(screenXY/8.0)*8.0;
    float value = ditherPattern[int(xy.x + xy.y * 8.0)] / 64.0;
	fragColor = vec4(graColour + ceil(lum-value), 1.0);

#elif DITHERING == 2
    // HALFTONE
    float x = screenXY.x;
    float y = screenXY.y;
	float s = 4.;
    float scpi45 = sqrt(2.0)/2.0;
    float rx = x*scpi45 - y*scpi45;
    float ry = x*scpi45 + y*scpi45;
    float c = sqrt(sqrt(abs(sin(rx*s)*sin(ry*s))));
	fragColor = vec4(graColour + ceil(lum-c), 1.0);
#endif
    }
}
