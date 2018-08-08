attribute highp vec4 a_Vertex;
attribute highp vec4 a_TexCoord0;
uniform highp mat4 u_ModelViewProjectionMatrix;
varying highp vec4 v_TexCoord0;

void main(void)
{
    gl_Position = u_ModelViewProjectionMatrix * a_Vertex;
    v_TexCoord0 = a_TexCoord0;
}
