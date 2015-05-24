
// vertex data
attribute vec4 vPosition;

// uniform data
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projTransform;

// fs output
varying vec4 fPosition;

// main
void main()
{
	gl_Position = modelTransform * vPosition;
	fPosition = vPosition;
}