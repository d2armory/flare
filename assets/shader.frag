
// settings
precision mediump float;

// vs input
varying vec4 fPosition;

// uniform

// main
void main()
{
    gl_FragColor = vec4(0.5) + fPosition;
}