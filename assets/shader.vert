
// vertex data
attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vUV;

// uniform data
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projTransform;

// fs output
varying vec2 fUV;

// main
void main()
{
	gl_Position = projTransform * viewTransform * modelTransform * vec4(vPosition,1);
	fUV = vUV;
}