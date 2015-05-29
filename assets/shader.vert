
precision mediump float;

// vertex data
attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vUV;
attribute vec4 vTangent;

// uniform data
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projTransform;
uniform mat4 depthBiasMvpTransform;

uniform mat4 mvTransform;
uniform mat4 mvpTransform;
uniform mat3 nTransform;

// fs output
varying vec2 fUV;
varying vec3 fPos;

varying vec3 fNormal;
varying vec4 fTangent;

varying vec3 fT;
varying vec3 fB;
varying vec3 fN;

varying vec3 fShadowCoord;

vec3 v3normalize(vec3 a)
{
	return a / length(a);
}


// main
void main()
{
	// TODO: move MV and MVP to uniform
	gl_Position = mvpTransform * vec4(vPosition,1);
	fUV = vUV;
	fPos = (mvTransform * vec4(vPosition,1)).xyz;
	fNormal = vNormal;
	fTangent = vTangent;//normalTransform * vTangent;
	
	fN = nTransform * v3normalize(fNormal);
	fT = nTransform * v3normalize(fTangent.xyz);
	fB = cross(fT, fN);
	
	fShadowCoord = (depthBiasMvpTransform * vec4(vPosition,1)).xyz;
}