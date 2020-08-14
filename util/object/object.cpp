#include "object.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "util/json.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <util/render/render3D.hpp>

using namespace std;
using json = nlohmann::json;

vector<Shader>  top_shader;  //top level render tree
vector<Model>   top_model;   //top level model tree
vector<TextureLookup> top_texture; //top level texture tree

json modelJson;

void loadModels(string jsonPath){

    //load json
    std::ifstream jsonModelFile(jsonPath, std::ios::in);
    if(!jsonModelFile.is_open()){
        printf("ERROR: unable to open json Model file: [%s]\n", jsonPath.c_str());
        exit(1);
    }
    std::stringstream jsonModelString;
    jsonModelString << jsonModelFile.rdbuf();
    jsonModelFile.close();
    try{
        modelJson = json::parse(jsonModelString.str());
    }
    catch(json::parse_error& e){
        std::cout << e.what() << endl; //print error
    }

    //count shaders
    int shaderCount = modelJson["shaders"].size();
    top_shader.reserve(shaderCount); //reserve enough space to hold all shaders

    //load shaders
    for(int i = 0; !modelJson["shaders"][i].is_null(); i++){
        Shader newShader;
        top_shader.push_back(newShader);
        top_shader[i].name = modelJson["shaders"][i]["name"];
        top_shader[i].vPath = modelJson["shaders"][i]["vPath"];
        top_shader[i].fPath = modelJson["shaders"][i]["fPath"];
        top_shader[i].CullFaceState = (modelJson["shaders"][i]["CullFace"] == "front") ?  GL_FRONT : ((modelJson["shaders"][i]["CullFace"] == "back") ? GL_BACK: GL_NONE);

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
            printf("ERROR: Shader code reading failed: [%s]\n", top_shader[i].name.c_str());
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
            printf("ERROR: Vertex shader compilation failed: [%s]\n >%s", top_shader[i].name.c_str(), infoLog);
        }
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fCodeC, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
            printf("ERROR: Fragment shader compilation failed: [%s]\n >%s", top_shader[i].name.c_str(), infoLog);
        }
        top_shader[i].ID = glCreateProgram();
        glAttachShader(top_shader[i].ID, vertex);
        glAttachShader(top_shader[i].ID, fragment);
        glGetProgramiv(top_shader[i].ID, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(vertex, 1024, NULL, infoLog);
            printf("ERROR: Shader linking failed: [%s]\n >%s", top_shader[i].name.c_str(), infoLog);
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

    }

    //count models:
    int modelCount = modelJson["models"].size();
    top_model.reserve(modelCount);

    //count meshes per shader:
    for(int i = 0; !modelJson["shaders"][i].is_null(); i++){
        top_shader[i].meshCountHint = 0;
        for(int j = 0; !modelJson["models"][j].is_null(); j++){
            if(modelJson["models"][j]["shader"] == modelJson["shaders"][i]["name"]){
                int meshCountHint = modelJson["models"][j]["meshCountHint"];
                top_shader[i].meshCountHint += meshCountHint;
            }
        }
        top_shader[i].meshes.reserve(top_shader[i].meshCountHint); //reserve memory
    }

    //load models
    for(int i = 0; !modelJson["models"][i].is_null(); i++){ //for each moddel
        Model newModel;
        top_model.push_back(newModel);
        top_model[i].objectPath = modelJson["models"][i]["path"];
        top_model[i].directory = top_model[i].objectPath.substr(0, top_model[i].objectPath.find_last_of('/'));
        top_model[i].hasPhysics = modelJson["models"][i]["hasPhysics"];
        top_model[i].useSinglePrimitive = modelJson["models"][i]["useSinglePrimitive"];
        top_model[i].scale = glm::vec3(modelJson["models"][i]["scale"]);
        top_model[i].isInstanced = modelJson["models"][i]["isInstanced"];
        if(top_model[i].isInstanced){
            //TODO: load the modelMatrices vector here
        }
        //read file
        Assimp::Importer importer;
        const aiScene* assimpModel = importer.ReadFile(top_model[i].objectPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if(!assimpModel || assimpModel->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpModel->mRootNode) // if is Not Zero
        {
            printf("ERROR: loading model failed: [%s]\n", top_model[i].objectPath.c_str());
            exit(1);
        }
        //this code only handles a single layer (root + 1) of assimp's nodes. A non-critical
        //error is thrown if the model excedes this value. If mesh physics is enabled,
        //the first root level mesh is the collision object. If we eventualy need more
        //depth, recursion might become a valid aproach.

        aiNode *node = assimpModel->mRootNode;

        for(unsigned int meshCounter = 0;  meshCounter < node->mNumMeshes; meshCounter++){                     //for each mesh in the moddel
            aiMesh* impMesh = assimpModel->mMeshes[node->mMeshes[meshCounter]];

            for(uint j = 0; j < top_shader.size(); j++){                                                        //find the shader
                if(modelJson["models"][i]["shaders"][meshCounter] == top_shader[j].name){                       //for the mesh

                    top_shader[j].meshes.push_back(processMesh(impMesh,assimpModel,top_model[i].directory));    //generate mesh data
                    Mesh* currentMesh = &top_shader[j].meshes[top_shader[j].meshes.size()];

                    if(meshCounter == 0){                                                                       //set mesh flags/settings for mesh 0
                        currentMesh->showObjectSelection = true;                                                //selection shows by default only on first mesh
                    }else{
                        currentMesh->showObjectSelection = false;
                    }
                    currentMesh->isInstanced = modelJson["models"][i]["isInstanced"];                           //for all meshes
                    currentMesh->parentModel = &top_model[i];
                    glm:: vec3 pos;
                    pos.x = modelJson["models"][i]["pos"][0];
                    pos.y = modelJson["models"][i]["pos"][1];
                    pos.z = modelJson["models"][i]["pos"][2];
                    currentMesh->model = glm::translate(glm::mat4(), pos);
                    //TODO: add instancing code here to set up the instancedMatrixBufferID and instanceCount. also, figure out how to import position matricies

                }
            }



        }



    }

}

Mesh processMesh(aiMesh *impMesh, const aiScene *assimpModel, string directory){

    Mesh mesh;
    mesh.vertices.reserve(impMesh->mNumVertices);
    for(unsigned int i = 0; i < impMesh->mNumVertices; i++){
        Vertex vertex;
        vertex.Position.x = impMesh->mVertices[i].x;
        vertex.Position.y = impMesh->mVertices[i].y;
        vertex.Position.z = impMesh->mVertices[i].z;
        vertex.Normal.x = impMesh->mNormals[i].x;
        vertex.Normal.y = impMesh->mNormals[i].y;
        vertex.Normal.z = impMesh->mNormals[i].z;
        vertex.Tangent.x = impMesh->mTangents[i].x;
        vertex.Tangent.y = impMesh->mTangents[i].y;
        vertex.Tangent.z = impMesh->mTangents[i].z;
        vertex.Bitangent.x = impMesh->mBitangents[i].x;
        vertex.Bitangent.y = impMesh->mBitangents[i].y;
        vertex.Bitangent.z = impMesh->mBitangents[i].z;
        if(impMesh->mTextureCoords[0]){ //if there are texture cords
            vertex.TexCoords.x = impMesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y = impMesh->mTextureCoords[0][i].y;
        }else{
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        mesh.vertices.push_back(vertex);
    }

    mesh.indices.reserve(impMesh->mNumFaces*3); //3 indices per triangle.
    for(unsigned int i = 0; i < impMesh->mNumFaces; i++){
        mesh.indices.push_back(impMesh->mFaces[i].mIndices[0]);
        mesh.indices.push_back(impMesh->mFaces[i].mIndices[1]);
        mesh.indices.push_back(impMesh->mFaces[i].mIndices[2]);
    }
    /*  Textures:
     *  ---------
     *  Loads the first texture of each type per mesh. This is suficient for
     *  all models I have so far encountered, but this may change. Tf there
     *  is a second texture that is missed, the extra texture flag will be
     *  set in the mesh.
     */
    aiMaterial* material = assimpModel->mMaterials[impMesh->mMaterialIndex];
    Texture texture;
    aiString path;
    uint textureLookupID;
    //difuse
    if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0){
        material->GetTexture(aiTextureType_DIFFUSE, 0, &path); //get path
        textureLookupID = findTextureID(path.C_Str());
        if(textureLookupID != 0){
            texture.defuseID = textureLookupID;
        }else{
            texture.defuseID = TextureFromFile(path.C_Str(), directory);
            TextureLookup textureLink;
            textureLink.id = texture.defuseID;
            textureLink.path = path.C_Str();
            top_texture.push_back(textureLink);
        }
    }else
        texture.defuseID = 0; //0 = no texture

    //specular
    if(material->GetTextureCount(aiTextureType_SPECULAR) > 0){
        material->GetTexture(aiTextureType_SPECULAR, 0, &path); //get path
        textureLookupID = findTextureID(path.C_Str());
        if(textureLookupID != 0){
            texture.specularID = textureLookupID;
        }else{
            texture.specularID = TextureFromFile(path.C_Str(), directory);
            TextureLookup textureLink;
            textureLink.id = texture.specularID;
            textureLink.path = path.C_Str();
            top_texture.push_back(textureLink);
        }
    }else
        texture.specularID = 0; //0 = no texture
    //normal
    if(material->GetTextureCount(aiTextureType_SPECULAR) > 0){
        material->GetTexture(aiTextureType_SPECULAR, 0, &path); //get path
        textureLookupID = findTextureID(path.C_Str());
        if(textureLookupID != 0){
            texture.normalID = textureLookupID;
        }else{
            texture.normalID = TextureFromFile(path.C_Str(), directory);
            TextureLookup textureLink;
            textureLink.id = texture.normalID;
            textureLink.path = path.C_Str();
            top_texture.push_back(textureLink);
        }
    }else
        texture.normalID = 0; //0 = no texture
    //ambient/height?   Usualy not used...
    if(material->GetTextureCount(aiTextureType_AMBIENT) > 0){
        material->GetTexture(aiTextureType_AMBIENT, 0, &path); //get path
        textureLookupID = findTextureID(path.C_Str());
        if(textureLookupID != 0){
            texture.heightID = textureLookupID;
        }else{
            texture.heightID = TextureFromFile(path.C_Str(), directory);
            TextureLookup textureLink;
            textureLink.id = texture.heightID;
            textureLink.path = path.C_Str();
            top_texture.push_back(textureLink);
        }
    }else
        texture.heightID = 0; //0 = no texture
    mesh.texture = texture;

    //build opengl buffers: VAO/VBO/EBO
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);
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

    glBindVertexArray(0);

    return mesh;
}



unsigned int findTextureID(const char* path){
    for(uint i = 0; i < top_texture.size(); i++){
        if(std::strcmp(top_texture[i].path.c_str(), path) == 0){
            return i;
        }
    }
    return 0;
}

unsigned int TextureFromFile(const char *path, const string &directory){
    string filename = string(path);
    filename = directory + '/' + filename;

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
        printf("ERROR: Texture failed to load: [%s]\n", path);
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}


void drawObjects(){
    for(uint i = 0; i < top_shader.size(); i++){
        glUseProgram(top_shader[i].ID);
        glUniformMatrix4fv(top_shader[i].projectionLocation, 1, GL_FALSE, &getprojectionMatrix()[0][0]);
        glUniformMatrix4fv(top_shader[i].viewLocation, 1, GL_FALSE, &getViewMatrix()[0][0]);

        //todo: move this out of the render loop; also make the lighting code more parametric
        glm::vec3 lightPos = glm::vec3(70,200,10);
        glUniform3f(top_shader[i].lightPosLocation, 1, GL_FALSE, lightPos[0]);
        glm::vec3 lightColor = glm::vec3(0.2,0.2,0.2);
        glUniform3f(top_shader[i].lightColorLocation, 1, GL_FALSE, lightColor[0]);

        for(uint meshIndex = 0; meshIndex < top_shader[i].meshes.size(); meshIndex++){
            //defuse texture
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(top_shader[i].texture_diffuse_location, 0);
            glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex].texture.defuseID);
            //specular texture
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(top_shader[i].texture_specular_location, 1);
            glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex].texture.specularID);
            //normal texture
            glActiveTexture(GL_TEXTURE2);
            glUniform1i(top_shader[i].texture_normal_location, 2);
            glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex].texture.normalID);
            //height texture
            glActiveTexture(GL_TEXTURE3);
            glUniform1i(top_shader[i].texture_height_location, 3);
            glBindTexture(GL_TEXTURE_2D, top_shader[i].meshes[meshIndex].texture.heightID);

            glBindVertexArray(top_shader[i].meshes[meshIndex].VAO);
            glDrawElements(GL_TRIANGLES, top_shader[i].meshes[meshIndex].indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE0); //is this reset needed? idk, probably not...

        }

    }

}
