
// settings
precision mediump float;

// vs input
varying vec2 fUV;
varying vec3 fPos;
varying vec3 fNormal;
varying vec4 fTangent;

// uniform
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projTransform;
uniform sampler2D texture[4];

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

// main
void main()
{
	//gl_FragColor = vec4(0.5) + fPosition;
	
	vec4 color = texture2D( texture[0], fUV);
	vec4 normal = texture2D( texture[1], fUV);
	vec4 mask1 = texture2D( texture[2], fUV);
	vec4 mask2 = texture2D( texture[3], fUV);
	
	vec3 L = vec3(1,0,0);
	L = v3normalize(L);
	
	mat3 normalTransform = mat3(viewTransform) * mat3(modelTransform);
	
	L = mat3(viewTransform) * L;
	
	vec3 N = normalTransform * fNormal;
	vec3 T = normalTransform * fTangent.xyz;
	vec3 B = cross(N, T);

	mat3 TBN = mat3( T.x, B.x, N.x, T.y, B.y, N.y, T.z, B.z, N.z );

	vec3 txtN = normal.rgb;
	vec3 tangentL = TBN * L;
	
	float NdotL = -1.0 * dot(txtN, tangentL);
	//float NdotL = dot(N, L);
	
	vec4 lColor = vec4(0.5,0.5,0.5,1);
	vec4 sColor = vec4(0.5,0.5,0.5,1);
	vec4 ambient = vec4(0.5,0.5,0.5,1);
	
	float diffuseLight = NdotL + mask1.a;
	
	vec4 diffuse = lColor * max(diffuseLight, 1e-6);
	
	vec3 V = v3normalize(-fPos);
	vec3 tangentV = TBN * V;
	
	//vec3 R = tangentL - (txtN * NdotL * 2.0);
	vec3 tangentR = tangentL + (txtN * NdotL * 2.0);
	float RdotV = dot(tangentR, tangentV);
	
	sColor = (sColor * (1.0 - mask2.b)) + (color * (mask2.b));
  
	vec4 specular = vec4(0,0,0,1);
	if(NdotL > 1e-6 && RdotV > 1e-6)
	{
		specular = sColor * (pow(max(1e-6, RdotV ), 1.0 + 4.0 * mask2.a) * mask2.r);
	}
	
	vec4 light = ambient + diffuse; 
	vec3 finalcolor = (color.rgb) * (1.0 - mask1.b);
	
	gl_FragColor = (light * vec4(color.rgb,1)) + specular;
	//gl_FragColor = specular;
	//gl_FragColor = (vec4(fTangent.aaa,1) / 4.0) + vec4(0.5,0.5,0.5,0);
	//gl_FragColor = vec4(texture2D( texture[3], fUV ).rrr,1);
}
