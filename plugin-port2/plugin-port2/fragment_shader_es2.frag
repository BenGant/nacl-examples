precision mediump float; 
varying vec3 v_color;
varying vec2 v_texCoord;
uniform sampler2D s_texture; 
void main() 
{ 
	gl_FragColor = texture2D( s_texture, v_texCoord ) + vec4(v_color.x,v_color.y,v_color.z,1);
}