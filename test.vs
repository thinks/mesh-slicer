#version 150
//#extension GL_EXT_gpu_shader4 : enable

in vec3 inVtx;    // Object space vertex.

void 
main(void)
{
    gl_Position = vec4(inVtx, 1.0);
}
