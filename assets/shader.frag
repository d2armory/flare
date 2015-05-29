
// settings
precision mediump float;

// vs input
varying vec2 fUV;
varying vec3 fPos;
varying vec3 fNormal;
varying vec4 fTangent;

varying vec3 fT;
varying vec3 fB;
varying vec3 fN;
varying vec3 fShadowCoord;

// uniform
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projTransform;
uniform sampler2D texture[5];

uniform vec3 lightDir;

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

// main
void main()
{
	//gl_FragColor = vec4(0.5) + fPosition;
	
	vec4 color = texture2D( texture[0], fUV);
	vec4 normal = texture2D( texture[1], fUV);
	vec4 mask1 = texture2D( texture[2], fUV);
	vec4 mask2 = texture2D( texture[3], fUV);
	
	// sampling on demand
	//vec4 shadow = texture2D( texture[4], fShadowCoord.xy);
	
	//vec3 L = vec3(-1.0,-10.0,-1.0);
	//L = v3normalize(L);
	
	// TODO: move to uniform
	// mat3 normalTransform = mat3(viewTransform) * mat3(modelTransform);
	
	// TODO: precomputed L in camera coord
	//L = mat3(viewTransform) * L;
	
	vec3 L = lightDir;
	
	vec3 N = fN;//normalTransform * v3normalize(fNormal);
	vec3 T = fT;//normalTransform * v3normalize(fTangent.xyz);
	vec3 B = fB;//cross(T, N);

	mat3 TBN = mat3( T.x, B.x, N.x, T.y, B.y, N.y, T.z, B.z, N.z );
	//mat3 invTBN = m3transpose(TBN);
	//mat3 invNT = m3transpose(normalTransform);
	
	//vec3 txtNworld = invNT * invTBN * v3normalize((normal.rgb * 2.0) - 1.0);

	vec3 txtN = v3normalize((normal.rgb * 2.0) - 1.0);
	vec3 tangentL = TBN * L;
	
	float NdotL = -1.0 * dot(txtN, tangentL);
	//float NdotL = dot(N, L);
	
	vec3 lColor = vec3(0.8,0.8,0.8);
	vec3 sColor = vec3(0.9,0.8,1.0);
	vec3 ambient = vec3(0.35,0.35,0.35);
	
	vec3 diffuse = vec3(0,0,0);
	vec3 specular = vec3(0,0,0);
	
	// shadow mapping
	
	vec2 poissonDisk[4];
	poissonDisk[0] = vec2( -0.94201624, -0.39906216 );
	poissonDisk[1] = vec2( 0.94558609, -0.76890725 );
	poissonDisk[2] = vec2( -0.094184101, -0.92938870 );
	poissonDisk[3] = vec2( 0.34495938, 0.29387760 );
	
	float visibility = 1.0;
	float bias = 0.005*tan(acos(NdotL)); // cosTheta is dot( n,l ), clamped between 0 and 1
	bias = fclamp(bias, 0.0,0.01);
	vec4 shadow = vec4(0,0,0,0);
	for(int i=0;i<4;i++)
	{
		shadow = texture2D( texture[4], fShadowCoord.xy + poissonDisk[i]/700.0);
		if ( shadow.x  <  fShadowCoord.z - bias){
			visibility -= 0.25;
		}
	}

	float diffuseLight = mask1.a;
	if(NdotL > 1e-6)
	{
		diffuseLight  = diffuseLight + NdotL;
	}
	diffuse = lColor * max(diffuseLight, 1e-6) * visibility;
	
	vec3 V = v3normalize(-fPos);
	vec3 tangentV = TBN * V;
	
	if(NdotL > 1e-6)
	{	
		
		//vec3 R = tangentL - (txtN * NdotL * 2.0);
		vec3 tangentR = tangentL + (txtN * NdotL * 2.0);
		float RdotV = dot(tangentR, tangentV);
		
		sColor = (sColor * (1.0 - mask2.b)) + (color.rgb * (mask2.b));
		
		float specExp = 20.0;
		float specScale = 0.5;//2.5;
		
		if(RdotV > 1e-6)
		{
			specular = sColor * (pow(max(1e-6, RdotV ), specExp * mask2.a) * mask2.r * specScale * visibility);
		}
	}
	
	float rimScale = 0.75;
	vec3 rimColor = vec3(1,1,1);
	
	// somehow, normal from normal map break rimlight
	// disabled until I find a reliable way to detect edge
	//float VdotN = dot(V,N);
	vec3 rimLight = vec3(0,0,0);
	//if(VdotN > 1e-6)
	//{
	//	rimLight = rimColor * (smoothstep(0.8, 1.0,(1.0 - VdotN)) * (rimScale * mask2.g));
	//}
	
	vec3 light = ambient + diffuse; 
	vec3 finalcolor = (color.rgb) * (1.0 - mask1.b);
	//finalcolor = vec3(0.7,0.7,0.7);
	
	gl_FragColor = vec4((light * finalcolor) + specular + rimLight,1);
	//gl_FragColor = vec4((txtNworld) * 0.25,1) + vec4(0.25,0.25,0.25,0) + (vec4(0.5,0.5,0.5,0) * ((fTangent.a / 2.0) + 0.5));
	//gl_FragColor = (vec4(fTangent.aaa,1) / 4.0) + vec4(0.5,0.5,0.5,0);
	//gl_FragColor = vec4(texture2D( texture[3], fUV ).rrr,1);
	//gl_FragColor = vec4(shadow.xxx,1);
}
