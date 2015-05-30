
// vertex data
attribute vec2 vPosition;

varying vec2 fUV;

void main(){
	gl_Position =  vec4((vPosition.x*2.0)-1.0,(vPosition.y*2.0)-1.0,0.0,1.0);
	fUV = vPosition;
}
