
// settings
precision mediump float;

// vs input
varying vec4 fPosition;

// uniform
uniform sampler2D texture;

// main
void main()
{
	//gl_FragColor = vec4(0.5) + fPosition;
	gl_FragColor = vec4(texture2D( texture, fPosition.xy + vec2(0.5,0.5) ).rgb,1);
}