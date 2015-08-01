
#extension GL_OES_standard_derivatives : enable

// settings
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

// vs input
varying vec2 fUV;
varying highp vec3 fPos;
varying vec3 fNormal;
varying vec4 fTangent;

varying vec3 fN;
varying vec3 fShadowCoord;

varying vec4 boneShader;

// uniform
//uniform mat4 modelTransform;
//uniform mat4 viewTransform;
//uniform mat4 projTransform;
uniform sampler2D texture[5];

uniform vec3 lightDir;
uniform int drawShadow;
uniform int hqNomal;

// 2 1/4 vec4

/*
vec3 v3cross(vec3 a, vec3 b)
{
  return vec3(a.y*b.z - a.z*b.y,a.z*b.x - a.x*b.z,a.x*b.y - a.y*b.x);
}
*/

vec3 v3normalize(vec3 a)
{
	return a / length(a);
}

mat3 m3transpose(mat3 m)
{
	return mat3(m[0].x,m[1].x,m[2].x,m[0].y,m[1].y,m[2].y,m[0].z,m[1].z,m[2].z);
}

float fclamp(float v, float min, float max)
{
	if(v<min) return min;
	if(v>max) return max;
	return v;
}

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );
	
	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
	
	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

// main
void main()
{
	//gl_FragColor = vec4(0.5) + fPosition;
	
	vec4 color = texture2D( texture[0], fUV);
	vec4 normal = texture2D( texture[1], fUV) * 2.0 - 1.0;
	vec4 mask1 = texture2D( texture[2], fUV);
	vec4 mask2 = texture2D( texture[3], fUV);
	
	// resconstructing normal for source 2 (dx10)
	//if(hqNomal==1)
	//{
		normal.r = normal.a;
		normal.g = normal.g;
		normal.b = sqrt(1.0 - fclamp(dot(normal.rg,normal.rg),0.0,1.0));
		normal.a = 1.0;
		// if we want bluish looking normal map, (normal/2.0)+0.5 yield that result
	//}
	
	vec3 V = v3normalize(-fPos);
	
	mat3 invTBN = cotangent_frame( fN, -V, fUV );
	
	// Light calc
	vec3 L = lightDir;
	
	vec3 N = fN;//normalTransform * v3normalize(fNormal);
	//vec3 T = fT;//normalTransform * v3normalize(fTangent.xyz);
	//vec3 B = fB;//cross(T, N);

	//mat3 TBN = mat3( T.x, B.x, N.x, T.y, B.y, N.y, T.z, B.z, N.z );
	//mat3 invTBN = m3transpose(TBN);
	//mat3 invNT = m3transpose(normalTransform);
	
	//vec3 txtNworld = invNT * invTBN * v3normalize((normal.rgb * 2.0) - 1.0);

	vec3 txtN = v3normalize(normal.rgb);
	vec3 worldN = invTBN * txtN;
	//vec3 tangentL = TBN * L;
	//vec3 worldN2 = worldN;
	
	//worldN = fN;
	
	float NdotL = -1.0 * dot(worldN, L);
	//float NdotL = dot(N, L);
	
	vec3 lColor = vec3(0.8,0.8,0.8);
	vec3 sColor = vec3(0.9,0.8,1.0);
	vec3 ambient = vec3(0.55,0.55,0.55);
	
	vec3 diffuse = vec3(0,0,0);
	vec3 specular = vec3(0,0,0);
	
	// shadow mapping
	
	//vec2 poissonDisk[4];
	//poissonDisk[0] = vec2( -0.94201624, -0.39906216 );
	//poissonDisk[1] = vec2( 0.94558609, -0.76890725 );
	//poissonDisk[2] = vec2( -0.094184101, -0.92938870 );
	//poissonDisk[3] = vec2( 0.34495938, 0.29387760 );
	
	vec2 poissonDisk[16];
	poissonDisk[0] = vec2( -0.94201624, -0.39906216 );
	poissonDisk[1] = vec2( 0.94558609, -0.76890725 );
	poissonDisk[2] = vec2( -0.094184101, -0.92938870 );
	poissonDisk[3] = vec2( 0.34495938, 0.29387760 );
	poissonDisk[4] = vec2( -0.91588581, 0.45771432 );
	poissonDisk[5] = vec2( -0.81544232, -0.87912464 );
	poissonDisk[6] = vec2( -0.38277543, 0.27676845 );
	poissonDisk[7] = vec2( 0.97484398, 0.75648379 );
	poissonDisk[8] = vec2( 0.44323325, -0.97511554 );
	poissonDisk[9] = vec2( 0.53742981, -0.47373420 );
	poissonDisk[10] = vec2( -0.26496911, -0.41893023); 
	poissonDisk[11] = vec2( 0.79197514, 0.19090188 );
	poissonDisk[12] = vec2( -0.24188840, 0.99706507 );
	poissonDisk[13] = vec2( -0.81409955, 0.91437590 );
	poissonDisk[14] = vec2( 0.19984126, 0.78641367 );
	poissonDisk[15] = vec2( 0.14383161, -0.14100790 );
	
	float visibility = 1.0;
	float bias = 0.005*tan(acos(NdotL)); // cosTheta is dot( n,l ), clamped between 0 and 1
	bias = fclamp(bias, 0.0,0.01);
	vec4 shadow = vec4(0,0,0,0);
	if(drawShadow==1)
	{
		for(int i=0;i<16;i++)
		{
			shadow = texture2D( texture[4], fShadowCoord.xy + poissonDisk[i]/700.0);
			if ( shadow.x  <  fShadowCoord.z - bias){
				visibility -= 0.0625;
			}
		}
	}

	float diffuseLight = 0.0;
	if(NdotL > 1e-6)
	{
		diffuseLight  = NdotL;
	}
	diffuse = lColor * max(max(diffuseLight, 1e-6) * visibility,mask1.a * 2.0);
	
	// moved to front
	//vec3 V = v3normalize(-fPos);
	//vec3 tangentV = TBN * V;
	
	if(NdotL > 1e-6)
	{	
		
		vec3 R = L + (worldN * NdotL * 2.0);
		//vec3 tangentR = tangentL + (txtN * NdotL * 2.0);
		
		float RdotV = dot(R, V);
		
		sColor = (sColor * (1.0 - mask2.b)) + (color.rgb * (mask2.b));
		
		float specExp = 20.0;
		float specScale = 1.25;//2.5;
		
		if(RdotV > 1e-6)
		{
			specular = sColor * (pow(max(1e-6, RdotV ), specExp * mask2.a) * mask2.r * specScale * visibility);
		}
	}
	
	float rimScale = 0.5;
	vec3 rimColor = vec3(1,1,1);
	
	// somehow, normal from normal map break rimlight
	// disabled until I find a reliable way to detect edge
	float VdotN = dot(V,worldN);
	vec3 rimLight = vec3(0,0,0);
	if(VdotN > 1e-6)
	{
		//rimLight = rimColor * (smoothstep(0.85, 1.0,(1.0 - VdotN)) * (rimScale * mask2.g * (1.0 - mask1.b)));
	}
	
	vec3 light = ambient + diffuse; 
	vec3 finalcolor = (color.rgb) * (1.0 - mask1.b);
	//finalcolor = vec3(0.5,0.5,0.5) + (finalcolor * 0.5);
	
	//
	
	gl_FragColor = vec4((light * finalcolor) + specular + rimLight,1);
	//gl_FragColor = vec4((txtNworld) * 0.25,1) + vec4(0.25,0.25,0.25,0) + (vec4(0.5,0.5,0.5,0) * ((fTangent.a / 2.0) + 0.5));
	//gl_FragColor = (vec4(fTangent.aaa,1) / 4.0) + vec4(0.5,0.5,0.5,0);
	//gl_FragColor = vec4(texture2D( texture[3], fUV ).rrr,1);
	//gl_FragColor = normal;
	
	//gl_FragColor = vec4(mask1.rgb,1);
	
	/* int bshader= int(boneShader);
	if(bshader&0x10==0 && bshader&0x01==0)
	{
		gl_FragColor = vec4(boneShader/3.0,0,0,1);
	}
	else if(bshader&0x10==0 && bshader&0x01==1)
	{
		gl_FragColor = vec4(0,boneShader/3.0,0,1);
	}
	else
	{
		gl_FragColor = vec4(0,0,boneShader/3.0 - 2.0,1);
	} */
	
	//gl_FragColor = boneShader;
}
