        
precision mediump float;

// vertex data
attribute vec3 vPosition;

// Values that stay constant for the whole mesh.
uniform mat4 mvpTransform;
 
void main(){
	gl_Position =  mvpTransform * vec4(vPosition,1);
}
        