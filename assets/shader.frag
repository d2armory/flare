
// settings
precision mediump float;

// vs input
varying vec2 fUV;

// uniform
uniform sampler2D texture;

// main
void main()
{
	//gl_FragColor = vec4(0.5) + fPosition;
	gl_FragColor = vec4(texture2D( texture, fUV ).rgb,1);
}
