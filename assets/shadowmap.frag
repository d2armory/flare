       
// settings
precision mediump float;

void main(){
	// Not really needed, OpenGL does it anyway
	//fragmentdepth = gl_FragCoord.z;
	gl_FragColor = vec4(gl_FragCoord.zzz,1);
	
}
        