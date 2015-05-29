
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
	gl_Position = projTransform * viewTransform * modelTransform * vec4(vPosition,1);
	fUV = vUV;
	fPos = (viewTransform * modelTransform * vec4(vPosition,1)).xyz;
	fNormal = vNormal;
	fTangent = vTangent;//normalTransform * vTangent;
	
	// TODO: move to uniform
	mat3 normalTransform = mat3(viewTransform) * mat3(modelTransform);
	
	fN = normalTransform * v3normalize(fNormal);
	fT = normalTransform * v3normalize(fTangent.xyz);
	fB = cross(fT, fN);
	
	fShadowCoord = (depthBiasMvpTransform * vec4(vPosition,1)).xyz;
}