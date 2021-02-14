#include "object_gl.h"
#include "object.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "util/render/startupConsole.hpp"
#include <stb_image.h>


#include <util/render/render3D.hpp>
#include "util/handleinput.hpp"
#include "util/bulletDebug/collisiondebugdrawer.hpp"

//load shader at index i
void loadShader_gl(int i){

    //TODO: fix
    //top_shader[i].CullFaceState = (modelJson["shaders"][i]["CullFace"] == "front") ?  GL_FRONT : ((modelJson["shaders"][i]["CullFace"] == "back") ? GL_BACK: GL_NONE);

    string vCode;
    string fCode;
    ifstream vCodeFile;
    ifstream fCodeFile;
    vCodeFile.exceptions (ifstream::failbit | ifstream::badbit);
    fCodeFile.exceptions (ifstream::failbit | ifstream::badbit);
    try {
        vCodeFile.open(top_shader[i].vPath);
        fCodeFile.open(top_shader[i].fPath);
        stringstream vCodeStream, fCodeStream;
        vCodeStream << vCodeFile.rdbuf();
        fCodeStream << fCodeFile.rdbuf();
        vCodeFile.close();
        fCodeFile.close();
        vCode = vCodeStream.str();
        fCode = fCodeStream.str();
    }
    catch (ifstream::failure e)
    {
        printMessage("Shader code reading failed: " , top_shader[i].name.c_str(), ERROR);
    }

    const char* vCodeC = vCode.c_str();
    const char* fCodeC = fCode.c_str();
    unsigned int vertex, fragment;
    GLint success;
    GLchar infoLog[1024];
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vCodeC, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
        printMessage("Vertex shader compilation failed: " , top_shader[i].name.c_str(), infoLog, ERROR);
    }
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fCodeC, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
        printMessage("Fragment shader compilation failed:  " , top_shader[i].name.c_str(), infoLog, ERROR);
    }
    top_shader[i].ID = glCreateProgram();
    glAttachShader(top_shader[i].ID, vertex);
    glAttachShader(top_shader[i].ID, fragment);
    glLinkProgram(top_shader[i].ID);
    glGetProgramiv(top_shader[i].ID, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(vertex, 1024, NULL, infoLog);
        printMessage("Shader linking failed: " , top_shader[i].name.c_str(), infoLog, ERROR);
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    //load uniform locations:
    top_shader[i].projectionLocation = glGetUniformLocation(top_shader[i].ID, "projection");
    top_shader[i].viewLocation = glGetUniformLocation(top_shader[i].ID, "view");
    top_shader[i].modelLocation = glGetUniformLocation(top_shader[i].ID, "model");
    top_shader[i].lightPosLocation = glGetUniformLocation(top_shader[i].ID, "lightPos");
    top_shader[i].lightColorLocation = glGetUniformLocation(top_shader[i].ID, "lightColor");
    //texture sampler uniform locations:
    top_shader[i].texture_diffuse_location = glGetUniformLocation(top_shader[i].ID, "texture_diffuse1");
    top_shader[i].texture_specular_location = glGetUniformLocation(top_shader[i].ID, "texture_specular1");
    top_shader[i].texture_normal_location = glGetUniformLocation(top_shader[i].ID, "texture_normal1");
    top_shader[i].texture_height_location = glGetUniformLocation(top_shader[i].ID, "texture_height1");

    printMessage( top_shader[i].name.c_str(), " shader compiled", SUCCESS);
}

void unloadModels_buffers_gl(int i, int j){
    glDeleteVertexArrays(1, &top_shader[i].meshes[j]->VAO);
    glDeleteBuffers(1, &top_shader[i].meshes[j]->VBO);
    glDeleteBuffers(1, &top_shader[i].meshes[j]->EBO);
    if(top_shader[i].meshes[j]->isInstanced){
        glDeleteBuffers(1, &top_shader[i].meshes[j]->instanceModelBuffer);
    }
}

void unloadModels_shaders_gl(int i){
    glDeleteShader(top_shader[i].ID);
}

void processMesh_buffers_gl(Mesh *mesh){
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);
    //set vertex attribute pointers
    // positions:
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // normals:
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // texture cords:
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // tangent vectors:
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // bitangent vectors:
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    if(mesh->isInstanced){
        std::size_t vec4Size = sizeof(glm::vec4);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceModelBuffer);

        glGenBuffers(1, &mesh->instanceModelBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceModelBuffer);
        glBufferData(GL_ARRAY_BUFFER, mesh->parentModel->instanceCount * sizeof(glm::mat4), &mesh->parentModel->modelMatrices[0], GL_STATIC_DRAW);


        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
    }

    glBindVertexArray(0);
}

unsigned int TextureFromFile_gl(string filename){
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED; //red = didnt get set. indicates error, shouldn't happen.
        if (nrComponents == 1)
            format = GL_LUMINANCE; //grayscale
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        printf("ERROR: Texture failed to load: [%s]\n", filename.c_str());
        stbi_image_free(data);
        return 0;
    }
    return textureID;
}

float getPhysicsFrameTime(){
    return getFrameTime();
}

void drawObjects(){
    for(uint i = 0; i < top_shader.size(); i++){
        glUseProgram(top_shader[i].ID);
        glUniformMatrix4fv(top_shader[i].projectionLocation, 1, GL_FALSE, glm::value_ptr(getprojectionMatrix()));
        glUniformMatrix4fv(top_shader[i].viewLocation, 1, GL_FALSE, glm::value_ptr(getViewMatrix()));

        //todo: move this out of the render loop; also make the lighting code more parametric
        glm::vec3 lightPos = glm::vec3(10,10,10);
        glUniform3f(top_shader[i].lightPosLocation, 1, GL_FALSE, lightPos[0]);
        glm::vec3 lightColor = glm::vec3(0.2,0.2,0.2);
        glUniform3fv(top_shader[i].lightColorLocation, 1, &lightColor[0]);

        for(uint meshIndex = 0; meshIndex < top_shader[i].meshes.size(); meshIndex++){

            //per mesh uniforms:
            if(top_shader[i].meshes[meshIndex]->parentModel->hasPhysics){
                glm::mat4 modelPhys = glm::mat4(1.0f);
                top_shader[i].meshes[meshIndex]->parentModel->body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(modelPhys));
                //modelPhys = glm::scale(modelPhys, top_shader[i].meshes[meshIndex]->parentModel->scale);//edit to use classbtCollisionShape.getLocalScaling()
                btVector3 scale = top_shader[i].meshes[meshIndex]->parentModel->collisionShape->getLocalScaling();
                modelPhys = glm::scale(modelPhys, glm::vec3(scale.x(), scale.y(), scale.z()));
                glUniformMatrix4fv(top_shader[i].modelLocation, 1, GL_FALSE, glm::value_ptr(modelPhys));
            }else{
                glUniformMatrix4fv(top_shader[i].modelLocation, 1, GL_FALSE, glm::value_ptr(top_shader[i].meshes[meshIndex]->parentModel->modelMatrices[0]));
            }

            int location;

            //defuse texture
            location = top_shader[i].texture_diffuse_location;
            if( location != -1){
                glActiveTexture(GL_TEXTURE0); //enable texture unit 0
                glUniform1i(location, 0); //set diffuse to use unit 0
                glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex]->texture.defuseID); //bind difuse texture to unit 0
            }

            //specular texture
            location = top_shader[i].texture_specular_location;
            if(location != -1){
                glActiveTexture(GL_TEXTURE1);
                glUniform1i(location, 1);
                glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex]->texture.specularID);
            }

            //normal texture
            location = top_shader[i].texture_normal_location;
            if(location != -1){
                glActiveTexture(GL_TEXTURE2);
                glUniform1i(top_shader[i].texture_normal_location, 2);
                glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex]->texture.normalID);
            }

            //height texture
            location = top_shader[i].texture_height_location;
            if(location != -1){
                glActiveTexture(GL_TEXTURE3);
                glUniform1i(location, 3);
                glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex]->texture.heightID);
            }

            glBindVertexArray(top_shader[i].meshes[meshIndex]->VAO);
            if(top_shader[i].meshes[meshIndex]->isInstanced){
                glDrawElementsInstanced(
                    GL_TRIANGLES, top_shader[i].meshes[meshIndex]->indices.size(), GL_UNSIGNED_INT, 0, top_shader[i].meshes[meshIndex]->instanceCount );
            }else{
                glDrawElements(GL_TRIANGLES, top_shader[i].meshes[meshIndex]->indices.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glActiveTexture(GL_TEXTURE0); //is this reset needed? idk, probably not...
            }

        }

    }

}
