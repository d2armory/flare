
precision mediump float;

// vertex data
attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vUV;
attribute vec4 vTangent;

attribute float vBoneCount;
attribute float vBone1;
attribute float vBone2;
attribute float vBone3;
attribute float vBoneweight1;
attribute float vBoneweight2;
attribute float vBoneweight3;

// uniform data
//uniform mat4 modelTransform;
//uniform mat4 viewTransform;
//uniform mat4 projTransform;
uniform mat4 depthBiasMvpTransform;

uniform mat4 mvTransform;
uniform mat4 mvpTransform;
uniform mat3 nTransform;

// 12 1/4 vec4

// current non-bone uniform: 14.5 vec4
// current bone uniform 92.75+32 = 124.75 vec4
// total = 139.25 vec4
// max = 250 (up from 128)
uniform int boneIndex[128];	// bone index into per-strip index
//uniform vec3 bonePos[53]; // translation data
//uniform vec4 boneRot[53]; // quat rotation data
uniform mat4 boneTransform[53];

// fs output
varying vec2 fUV;
varying vec3 fPos;

varying vec3 fNormal;
varying vec4 fTangent;

//varying vec3 fT;
//varying vec3 fB;
varying vec3 fN;

varying vec3 fShadowCoord;

varying float boneShader;

vec2 v2normalize(vec2 a)
{
	return a / length(a);
}

vec3 v3normalize(vec3 a)
{
	return a / length(a);
}

mat3 q4tom3(vec4 q)
{
	float qx = q.x;
	float qy = q.y;
	float qz = q.z;
	float qw = q.w;
	
	float qx2x2 = 2.0 * qx * qx;
	float qy2x2 = 2.0 * qy * qy;
	float qz2x2 = 2.0 * qz * qz;
	
	float qxqyx2 = 2.0 * qx * qy;
	float qzqwx2 = 2.0 * qz * qw;
	float qxqzx2 = 2.0 * qx * qz;
	float qyqwx2 = 2.0 * qy * qw;
	float qyqzx2 = 2.0 * qy * qz;
	float qxqwx2 = 2.0 * qx * qw;
	
	return mat3(	1.0 - qy2x2 - qz2x2	,	qxqyx2 - qzqwx2	,	qxqzx2 + qyqwx2	,
					qxqyx2 + qzqwx2	,	1.0 - qx2x2 - qz2x2	,	qyqzx2 - qxqwx2	,
					qxqzx2 - qyqwx2	,	qyqzx2 + qxqwx2	,	1.0 - qx2x2 - qy2x2);
}

// main
void main()
{
	
	vec3 vaPos = vPosition;
	
	/* vec3 vaPos = vec3(0,0,0);
	vec3 vaPos1 = (boneTransform[boneIndex[int(vBone1)]] * vec4(vaPosIn,1)).xyz;
	vaPos += vaPos1 * vBoneweight1;
	vec3 vaPos2 = (boneTransform[boneIndex[int(vBone2)]] * vec4(vaPosIn,1)).xyz;
	vaPos += vaPos2 * vBoneweight2;
	vec3 vaPos3 = (boneTransform[boneIndex[int(vBone3)]] * vec4(vaPosIn,1)).xyz;
	vaPos += vaPos3 * vBoneweight3; */
	
	gl_Position = mvpTransform * vec4(vaPos,1);
	fUV = vUV;//vec2(vUV.y,1.0 - vUV.x);
	fPos = (mvTransform * vec4(vaPos,1)).xyz;
	fNormal = vNormal;
	fTangent = vTangent;//normalTransform * vTangent;
	
	fN = nTransform * fNormal;
	//fT = nTransform * v3normalize(fTangent.xyz);
	//fB = cross(fT, fN);
	
	fShadowCoord = (depthBiasMvpTransform * vec4(vaPos,1)).xyz;
	
	boneShader = (vBone1);// / 128.0;
}