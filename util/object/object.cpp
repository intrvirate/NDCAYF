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
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <util/render/render3D.hpp>
#include "util/handleinput.hpp"

#include <btBulletDynamicsCommon.h>
#include "util/bulletDebug/collisiondebugdrawer.hpp"
#include "collisionMaskClasses.hpp"


using namespace std;
using json = nlohmann::json;

vector<Shader>  top_shader;  //top level render tree
vector<Model*>   top_model;   //top level model tree
vector<TextureLookup> top_texture; //top level texture tree

json modelJson;

//bullet world stuff:
btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

btAlignedObjectArray<btCollisionShape*> collisionShapes;
BulletDebugDrawer_OpenGL debugDraw;

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
    for(uint i = 0; !modelJson["shaders"][i].is_null(); i++){

        Shader newShader;
        top_shader.push_back(newShader);
        top_shader[i].name = modelJson["shaders"][i]["name"];
        top_shader[i].vPath = modelJson["shaders"][i]["vPath"];
        top_shader[i].fPath = modelJson["shaders"][i]["fPath"];
        //TODO: fix
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
            printf("ERROR: Vertex shader compilation failed: [%s]\n >%s\n", top_shader[i].name.c_str(), infoLog);
        }
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fCodeC, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
            printf("ERROR: Fragment shader compilation failed: [%s]\n >%s\n", top_shader[i].name.c_str(), infoLog);
        }
        top_shader[i].ID = glCreateProgram();
        glAttachShader(top_shader[i].ID, vertex);
        glAttachShader(top_shader[i].ID, fragment);
        glLinkProgram(top_shader[i].ID);
        glGetProgramiv(top_shader[i].ID, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(vertex, 1024, NULL, infoLog);
            printf("ERROR: Shader linking failed: [%s]\n >%s\n", top_shader[i].name.c_str(), infoLog);
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

    //count model
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

        Model* newModel = new Model;
        top_model.push_back(newModel);
        top_model[i]->objectPath = modelJson["models"][i]["path"];
        top_model[i]->directory = top_model[i]->objectPath.substr(0, top_model[i]->objectPath.find_last_of('/'));
        top_model[i]->hasPhysics = modelJson["models"][i]["hasPhysics"];
        top_model[i]->useSinglePrimitive = modelJson["models"][i]["useSinglePrimitive"];
        top_model[i]->scale = glm::vec3(modelJson["models"][i]["scale"]);
        top_model[i]->isInstanced = modelJson["models"][i]["isInstanced"];
        if(top_model[i]->isInstanced){
            //load the modelMatrices vector here
            top_model[i]->modelMatrices.reserve(modelJson["models"][i]["pos"].size());
            top_model[i]->instanceCount = modelJson["models"][i]["pos"].size()/3;
            for(uint instanceCounter = 0; !modelJson["models"][i]["pos"][instanceCounter].is_null(); instanceCounter = instanceCounter + 3){
                glm::vec3 instancePos;
                instancePos.x = modelJson["models"][i]["pos"][instanceCounter+0];
                instancePos.y = modelJson["models"][i]["pos"][instanceCounter+1];
                instancePos.z = modelJson["models"][i]["pos"][instanceCounter+2];
                glm::mat4 posMatrix  = glm::mat4(1.0);
                posMatrix = glm::translate(posMatrix, instancePos);
                //todo: add scaling and rotation
                top_model[i]->modelMatrices.push_back(posMatrix);
            }
        }else{
            top_model[i]->pos.x = modelJson["models"][i]["pos"][0];
            top_model[i]->pos.y = modelJson["models"][i]["pos"][1];
            top_model[i]->pos.z = modelJson["models"][i]["pos"][2];
        }

        //read file
        Assimp::Importer importer;
        const aiScene* assimpModel = importer.ReadFile(top_model[i]->objectPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_Debone);
        if(!assimpModel || assimpModel->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpModel->mRootNode) // if is Not Zero
        {
            printf("ERROR: loading model failed: [%s]\n", top_model[i]->objectPath.c_str());
            printf("Error log:\n> %s\n", importer.GetErrorString());
            exit(1);
        }

        //read meshes:
        top_model[i]->meshes.reserve(assimpModel->mNumMeshes);
        for(uint meshCounter = 0; meshCounter < assimpModel->mNumMeshes; meshCounter++){ //for all meshes
            for(uint j = 0; j < top_shader.size(); j++){ //find the shader
                if(modelJson["models"][i]["shaders"][meshCounter] == top_shader[j].name){

                    Mesh *currentMesh = new Mesh;
                    top_shader[j].meshes.push_back(currentMesh);
                    currentMesh->parentModel = top_model[i];

                    currentMesh->isInstanced = modelJson["models"][i]["isInstanced"];
                    if(currentMesh->isInstanced){
                        currentMesh->instanceCount = top_model[i]->instanceCount;
                    }

                    processMesh(currentMesh, assimpModel->mMeshes[meshCounter] ,assimpModel,top_model[i]->directory);

                    if(meshCounter == 0){                                                                 //set mesh flags/settings for mesh 0
                        currentMesh->showObjectSelection = true;                                          //selection shows by default only on first mesh
                    }else{
                        currentMesh->showObjectSelection = false;
                    }

                    glm::mat4 Matrix = glm::scale(glm::mat4(1.0f), top_model[i]->scale);
                    Matrix = glm::translate(Matrix, top_model[i]->pos);

                    currentMesh->model = Matrix;

                    top_model[i]->meshes.push_back(currentMesh); //push pointer to vector of pointers

                    fprintf(stderr, "added mesh. pos = %f,%f,%f\n", top_model[i]->pos.x, top_model[i]->pos.y, top_model[i]->pos.z);
                    break; //don't add to multiple shaders, so don't need to keep scanning remaining
                }

            }

        }

        //physics:
        //If mesh physics is enabled, the first root level mesh is the collision object.
        if(modelJson["models"][i]["hasPhysics"] == true){

            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(top_model[i]->pos.x, top_model[i]->pos.y, top_model[i]->pos.z));
            float mass = modelJson["models"][i]["mass"]; //NOTE: to make an object static, mass and inerta must be set to 0
            top_model[i]->mass = mass;
            if(mass != 0)
                 top_model[i]->isDynamic = true;

            float friction = modelJson["models"][i]["friction"];
            float rotatingFriction = modelJson["models"][i]["Rotatingfriction"];

            int numTriangles = top_model[i]->meshes[0]->indices.size() / 3 ;
            uint* triangleIndexBase = &(top_model[i]->meshes[0]->indices[0]);
            int triangleIndexStride = sizeof(top_model[i]->meshes[0]->indices[0]) * 3;

            int numVertices = top_model[i]->meshes[0]->vertices.size();
            btScalar * vertexBase = (btScalar*)&(top_model[i]->meshes[0]->vertices[0]);
            int vertexStride = sizeof(Vertex);


            if(modelJson["models"][i]["useSinglePrimitive"] == true){
                string primitiveType = modelJson["models"][i]["primitiveType"];

                if(primitiveType == "sphere"){
                    float primitiveSize = modelJson["models"][i]["primitiveSize"];
                    top_model[i]->collisionShape = new btSphereShape(btScalar(primitiveSize));

                }else if (primitiveType == "cylinder"){
                    float h = modelJson["models"][i]["primitiveH"];
                    float r = modelJson["models"][i]["primitiveR"];
                    btVector3 primitiveDimentions =btVector3(r,h,h);
                    top_model[i]->collisionShape = new btCylinderShape(primitiveDimentions);

                }else if (primitiveType == "box"){
                    float x = modelJson["models"][i]["primitiveX"];
                    float y = modelJson["models"][i]["primitiveY"];
                    float z = modelJson["models"][i]["primitiveZ"];
                    btVector3 primitiveDimentions =btVector3(x,y,z);
                    top_model[i]->collisionShape = new btBoxShape(primitiveDimentions);
                }else if (primitiveType == "hull"){
                    btConvexHullShape * newShape = new btConvexHullShape (vertexBase, numVertices, vertexStride);
                    newShape->optimizeConvexHull();
                    top_model[i]->collisionShape = newShape;

                }else{
                    fprintf(stderr, "invalid primitive passed");
                }
            }else{
                //mesh primitive TODO: handle non-static meshes. At the moment, any mesh is created static.

                btTriangleIndexVertexArray* indexVertexArrays = new btTriangleIndexVertexArray(numTriangles, (int*)(size_t)triangleIndexBase, triangleIndexStride, numVertices, vertexBase, vertexStride );
                bool useQuantizedAabbCompression = true;
                //set collisionShape to be a meshShape
                top_model[i]->collisionShape = new btBvhTriangleMeshShape(indexVertexArrays, useQuantizedAabbCompression);

            }

            top_model[i]->collisionShape->setUserPointer(top_model[i]);
            top_model[i]->collisionShape->setLocalScaling( btVector3(top_model[i]->scale.x, top_model[i]->scale.y, top_model[i]->scale.z));
            float inerta = modelJson["models"][i]["inerta"];
            btVector3 localInertia(inerta, inerta, inerta);
            top_model[i]->inerta = inerta;

            top_model[i]->collisionShape->calculateLocalInertia(mass, localInertia);

            btDefaultMotionState* motionState = new btDefaultMotionState(transform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, top_model[i]->collisionShape, localInertia);

            rbInfo.m_friction = friction;
            rbInfo.m_rollingFriction = rotatingFriction;
            rbInfo.m_spinningFriction = rotatingFriction;

            top_model[i]->body = new btRigidBody(rbInfo);

            top_model[i]->body->setContactProcessingThreshold(BT_LARGE_FLOAT);
            top_model[i]->body->setCcdMotionThreshold(.5);
            top_model[i]->body->setCcdSweptSphereRadius(0);

            //collision masks:
            string collisionType = modelJson["models"][i]["collisionType"];
            top_model[i]->origionalCollisionGroup = getCollisionGroupByString(collisionType);
            top_model[i]->origionalCollisionMask = getCollisionMaskByString(collisionType);
            // syntax: addRigidBody( body, group, mask);
            dynamicsWorld->addRigidBody(top_model[i]->body, top_model[i]->origionalCollisionGroup, top_model[i]->origionalCollisionMask);
        }
    }
}

//groups
collisionMasks getCollisionGroupByString(string str){

    if (str == "nothing"){
        return COL_NOTHING;
    }else if (str == "terrain"){
        return COL_TERRAIN;
    }else if (str == "selectable"){
        return COL_SELECTER;
    }else if (str == "damageable"){
        return COL_DAMAGEABLE;
    }else if (str == "powerup"){
        return COL_POWERUP;
    }else if (str == "player"){
        return COL_PLAYER;
    }else if (str == "throwable"){
        return COL_THROWABLE;
    }else if (str == "item"){
        return COL_ITEM;
    }else if (str == "proximity"){
        return COL_PROXIMITY;
    }
    return COL_NOTHING; //todo: error handle the invalid case
}

//masks
collisionGroups getCollisionMaskByString(string str){

    if (str == "terrain"){
        return COL_TERRAIN_COLLIDES_WITH;
    }else if (str == "selectable"){
        return COL_SELECT_RAY_COLLIDES_WITH;
    }else if (str == "powerup"){
        return COL_POWERUP_COLLIDES_WITH;
    }else if (str == "player"){
        return COL_PLAYER_COLLIDES_WITH;
    }else if (str == "throwable"){
        return COL_THROWABLE_COLLIDES_WITH;
    }else if (str == "item"){
        return COL_ITEM_COLLIDES_WITH;
    }else if (str == "proximity"){
        return COL_PROXIMITY_COLLIDES_WITH;
    }
    return COL_NOTHING_COLLIDES_WITH; //todo: error handle the invalid case

}


void processMesh(Mesh *mesh,aiMesh *impMesh, const aiScene *assimpModel, string directory){

    mesh->vertices.reserve(impMesh->mNumVertices);
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
        mesh->vertices.push_back(vertex);
    }

    mesh->indices.reserve(impMesh->mNumFaces*3); //3 indices per triangle.
    for(unsigned int i = 0; i < impMesh->mNumFaces; i++){
        mesh->indices.push_back(impMesh->mFaces[i].mIndices[0]);
        mesh->indices.push_back(impMesh->mFaces[i].mIndices[1]);
        mesh->indices.push_back(impMesh->mFaces[i].mIndices[2]);
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
    //diffuse
    if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0){
        fprintf(stderr, "loaded difuse\n");
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
        fprintf(stderr, "loaded specular\n");
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
    if(material->GetTextureCount(aiTextureType_HEIGHT) > 0){
        fprintf(stderr, "loaded normal (height)\n");
        material->GetTexture(aiTextureType_HEIGHT, 0, &path); //get path
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
        fprintf(stderr, "loaded height (ambient)\n");
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
    mesh->texture = texture;

    //build opengl buffers: VAO/VBO/EBO
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
                modelPhys = glm::scale(modelPhys, top_shader[i].meshes[meshIndex]->parentModel->scale);
                glUniformMatrix4fv(top_shader[i].modelLocation, 1, GL_FALSE, glm::value_ptr(modelPhys));
            }else{
                glUniformMatrix4fv(top_shader[i].modelLocation, 1, GL_FALSE, glm::value_ptr(top_shader[i].meshes[meshIndex]->model));
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

void InitializePhysicsWorld(){
    dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
    debugDraw.loadDebugShaders();
    dynamicsWorld->setDebugDrawer(&debugDraw);
}

void RunStepSimulation(){

    dynamicsWorld->stepSimulation(getFrameTime(), 10);
}


//see https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=11690 for context
void disableCollision(Model* model){
    //model->body->setCollisionFlags(model->body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

    btBroadphaseProxy* proxy = model->body->getBroadphaseProxy();
    proxy->m_collisionFilterGroup = COL_NOTHING;
    proxy->m_collisionFilterMask = COL_NOTHING_COLLIDES_WITH;
}
void enableCollision(Model* model){
    //model->body->setCollisionFlags(model->body->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);

    btBroadphaseProxy* proxy = model->body->getBroadphaseProxy();
    proxy->m_collisionFilterGroup = model->origionalCollisionGroup;
    proxy->m_collisionFilterMask = model->origionalCollisionMask;
}

void makeStatic(Model* model){

    model->body->setMassProps(0,btVector3(0,0,0));
    model->body->setLinearVelocity(btVector3(0.0,0.0,0.0));
    model->body->updateInertiaTensor();

    model->body->setCollisionFlags(model->body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);


}

void makeDynamic(Model* model){


    model->body->setMassProps(model->mass,btVector3(model->inerta,model->inerta,model->inerta));
    model->body->activate();
    model->body->updateInertiaTensor();

    model->body->setCollisionFlags(model->body->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT);

}

//note: this is not a very fast function; use it sparingly.
Model* getModelPointerByName(string name){
    for (uint i = 0; i < top_model.size(); i++){
        if (top_model[i]->objectPath.find(name) != string::npos){
            return top_model[i];
        }
    }
    return NULL;
}

void updateModelPosition(Model* model, glm::vec3 pos){

        btTransform tr = model->body->getWorldTransform();
        tr.setOrigin(btVector3(pos.x,pos.y,pos.z));
        model->body->setWorldTransform(tr);

    //update all sub meshes
    for (uint i = 0; i < model->meshes.size(); i++){
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, pos);
        modelMat = glm::scale(modelMat, model->scale);
        model->meshes[i]->model = modelMat;
    }
}

void updateModelPosition(Model* model, btVector3 pos){

        btTransform tr = model->body->getWorldTransform();
        tr.setOrigin(pos);
        model->body->setWorldTransform(tr);

    //update all sub meshes
    glm::vec3 glmPos = glm::vec3(pos.x(), pos.y(), pos.z());
    for (uint i = 0; i < model->meshes.size(); i++){
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, glmPos);
        modelMat = glm::scale(modelMat, model->scale);
        model->meshes[i]->model = modelMat;
    }
}







