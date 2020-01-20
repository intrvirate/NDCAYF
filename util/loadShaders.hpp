#ifndef LOADSHADERS_HPP
#define LOADSHADERS_HPP


GLuint LoadVertexShader(const char * VertexShader_filepath);
GLuint LoadFragmentShader(const char * FragmentShader_filepath);
GLuint LinkShaders(GLuint vertexShader, GLuint fragmentShader);


#endif // LOADSHADERS_HPP
