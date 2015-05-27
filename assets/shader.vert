
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

// fs output
varying vec2 fUV;
varying vec3 fPos;
varying vec3 fNormal;
varying vec4 fTangent;

// main
void main()
{
	gl_Position = projTransform * viewTransform * modelTransform * vec4(vPosition,1);
	fUV = vUV;
	fPos = (viewTransform * modelTransform * vec4(vPosition,1)).xyz;
	fNormal = vNormal;
	fTangent = vTangent;//normalTransform * vTangent;
}