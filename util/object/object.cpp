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
#include "util/render/startupConsole.hpp"
//gtc
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
//gtx
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <btBulletDynamicsCommon.h>
#include "collisionMaskClasses.hpp"
#include "object_gl.h"

using namespace std;
using json = nlohmann::json;

vector<Shader>  top_shader;         //top level render tree
vector<Model*>   top_model;         //top level model tree
vector<TextureLookup> top_texture;  //top level texture tree

json modelJson;

//bullet world stuff:
btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

BulletDebugDrawer_OpenGL debugDraw;

void loadModels(string jsonPath){

    //load json
    std::ifstream jsonModelFile(jsonPath, std::ios::in);
    if(!jsonModelFile.is_open()){
        //printf("ERROR: unable to open json Model file: [%s]\n", jsonPath.c_str());
        printMessage("unable to upen JSON model file: " + jsonPath, FATALERROR);
        exit(1);
    }
    printMessage("loading modelJson file: " + jsonPath, STATUS);
    std::stringstream jsonModelString;
    jsonModelString << jsonModelFile.rdbuf();
    jsonModelFile.close();
    try{
        modelJson = json::parse(jsonModelString.str());
    }
    catch(json::parse_error& e){
        printMessage("parsing json failed: " , e.what(), ERROR);
    }

    //count model
    int modelCount;
    try{
        modelCount = modelJson["models"].size();
    }catch(json::parse_error& e){
        printMessage("models array may not exist (size method failed)", FATALERROR);
    }
    top_model.reserve(modelCount);


    //count shaders
    int shaderCount;
    try{
        shaderCount = modelJson["shaders"].size();
    }catch(json::parse_error& e){
        printMessage("shaders array may not exist (size method failed)", FATALERROR);
    }

    top_shader.reserve(shaderCount); //reserve enough space to hold all shaders


    //load shaders
    printMessage("loading shaders", STATUS);

    for(uint i = 0; !modelJson["shaders"][i].is_null(); i++){

        Shader* newShader = new Shader;
        top_shader.push_back(*newShader);
        try{
            top_shader[i].name = modelJson["shaders"][i]["name"];
            top_shader[i].vPath = modelJson["shaders"][i]["vPath"];
            top_shader[i].fPath = modelJson["shaders"][i]["fPath"];
        }
        catch(json::type_error& e){
            printMessage("loading shader paths failed: ", e.what() , FATALERROR);
        }

        loadShader_gl(i);
    }

    //count meshes per shader:
    try{
        for(int i = 0; !modelJson["shaders"][i].is_null(); i++){
            top_shader[i].meshCountHint = 0;
            for(int j = 0; !modelJson["models"][j].is_null(); j++){
                if(modelJson["models"][j]["shader"] == modelJson["shaders"][i]["name"]){
                    int meshCountHint;
                    try{
                        meshCountHint = modelJson["models"][j]["meshCountHint"];
                    }catch(json::type_error& e){
                        printMessage( "meshCountHint field missing in model, not optimizing shader vector memory usage", WARNING);
                        meshCountHint = 0;
                    }
                    top_shader[i].meshCountHint += meshCountHint;
                }
            }
            top_shader[i].meshes.reserve(top_shader[i].meshCountHint); //reserve memory
        }
    }catch(json::type_error& e){
        printMessage("counting meshes failed", e.what() , FATALERROR);
    }

    printMessage("building Model tree", STATUS);

    //load models
    for(int i = 0; !modelJson["models"][i].is_null(); i++){ //for each moddel

        Model* newModel = new Model;
        top_model.push_back(newModel);

        //path field
        try{
            top_model[i]->objectPath = modelJson["models"][i]["path"];
        }catch(json::type_error& e){
            printMessage("path value missing for model " + to_string(i) , FATALERROR);
        }

        printMessage("loading: ", top_model[i]->objectPath.c_str(), " ", STATUS);

        //directory field
        top_model[i]->directory = top_model[i]->objectPath.substr(0, top_model[i]->objectPath.find_last_of('/'));

        //hasPhysics field
        try{
            top_model[i]->hasPhysics = modelJson["models"][i]["hasPhysics"];
        }catch(json::type_error& e){
            printMessage("hasPhysics field missing, setting to default (no) ", e.what() , WARNING);
            top_model[i]->hasPhysics = false;
        }

        //isInstanced field
        try{
            top_model[i]->isInstanced = modelJson["models"][i]["isInstanced"];
        }catch(json::type_error& e){
            printMessage("isInstanced field missing, setting to default (no) ", e.what() , WARNING);
            top_model[i]->isInstanced = false;
        }

        glm::vec3 pos; //out here because the physics code uses it below
        float scale;
        glm::quat rot;

        if(top_model[i]->isInstanced){
            //load the modelMatrices vector here
            try{
                top_model[i]->modelPositions.reserve(modelJson["models"][i]["pos"].size());
                top_model[i]->modelMatrices.reserve(modelJson["models"][i]["pos"].size());
            }catch(json::type_error& e){
                printMessage("pos field missing, unable to set size. ", e.what() , ERROR);
            }
            try{
                top_model[i]->instanceCount = modelJson["models"][i]["pos"].size()/3;
                for(uint instanceCounter = 0; !modelJson["models"][i]["pos"][instanceCounter].is_null(); instanceCounter = instanceCounter + 3){
                    glm::vec3 instancePos;
                    instancePos.x = modelJson["models"][i]["pos"][instanceCounter+0];
                    instancePos.y = modelJson["models"][i]["pos"][instanceCounter+1];
                    instancePos.z = modelJson["models"][i]["pos"][instanceCounter+2];
                    glm::mat4 posMatrix  = glm::mat4(1.0);
                    posMatrix = glm::translate(posMatrix, instancePos);
                    //todo: add scaling and rotation
                    top_model[i]->modelPositions.push_back(instancePos);
                    top_model[i]->modelMatrices.push_back(posMatrix);
                }
            }catch(json::type_error& e){
                printMessage("unable to process instanced position data: " + top_model[i]->objectPath, " ", e.what() , ERROR);
            }
        }else{
            glm::mat4 posMatrix  = glm::mat4(1.0);
            try{
                scale = modelJson["models"][i]["scale"];
                updateMessage("scale: " + to_string(scale));

                rot.x = modelJson["models"][i]["rotquat"][0];
                rot.y = modelJson["models"][i]["rotquat"][1];
                rot.z = modelJson["models"][i]["rotquat"][2];
                rot.w = modelJson["models"][i]["rotquat"][3];

                //pos is declared higher up
                pos.x = modelJson["models"][i]["pos"][0];
                pos.y = modelJson["models"][i]["pos"][1];
                pos.z = modelJson["models"][i]["pos"][2];

            } catch(json::type_error& e){
                printMessage("position / rotation / scale data missing, using default values (0,0,0) ", e.what() , WARNING);
                scale = 1;
                rot = glm::quat(1,0,0,0);
                pos = glm::vec3(0,0,0);
            }
            posMatrix = glm::scale(posMatrix, glm::vec3(scale, scale, scale));
            posMatrix = glm::mat4_cast(rot) * posMatrix;
            posMatrix = glm::translate(posMatrix, pos);

            top_model[i]->modelMatrices.push_back(posMatrix);
        }

        //read file
        Assimp::Importer importer;
        const aiScene* assimpModel = importer.ReadFile(top_model[i]->objectPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_Debone);
        if(!assimpModel || assimpModel->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpModel->mRootNode) // if is Not Zero
        {
            printMessage("loading model via assimp failed: ", top_model[i]->objectPath.c_str(), importer.GetErrorString(), FATALERROR);
        }

        //read meshes:
        updateMessage(" with " + to_string(assimpModel->mNumMeshes) + " meshes");

        top_model[i]->meshes.reserve(assimpModel->mNumMeshes);

        for(uint meshCounter = 0; meshCounter < assimpModel->mNumMeshes; meshCounter++){ //for all meshes
            for(uint j = 0; j < top_shader.size(); j++){ //find the shader
                if(modelJson["models"][i]["shaders"][meshCounter] == top_shader[j].name){
                    printMessage("loading mesh " + to_string(meshCounter), " using shader ", top_shader[j].name.c_str(), STATUS);

                    Mesh *currentMesh = new Mesh;
                    top_shader[j].meshes.push_back(currentMesh);
                    currentMesh->parentModel = top_model[i];
                    currentMesh->parentShader = &top_shader[j];
                    try{
                        currentMesh->isInstanced = modelJson["models"][i]["isInstanced"];
                    }catch(json::type_error& e){
                        printMessage("isInstanced field missing, assuming (false) ", e.what() , WARNING);
                        currentMesh->isInstanced = false;
                    }

                    if(currentMesh->isInstanced){
                        currentMesh->instanceCount = top_model[i]->instanceCount;
                    }

                    processMesh(currentMesh, assimpModel->mMeshes[meshCounter] ,assimpModel,top_model[i]->directory);

                    if(meshCounter == 0){                                                                 //set mesh flags/settings for mesh 0
                        currentMesh->showObjectSelection = true;                                          //selection shows by default only on first mesh
                    }else{
                        currentMesh->showObjectSelection = false;
                    }

                    top_model[i]->meshes.push_back(currentMesh); //push pointer to vector of pointers

                    //fprintf(stderr, "added mesh. pos = %f,%f,%f\n", top_model[i]->pos.x, top_model[i]->pos.y, top_model[i]->pos.z);
                    updateMessageType(SUCCESS);
                    break; //don't add to multiple shaders, so don't need to keep scanning remaining
                }

            }

        }

        //physics:
        //If mesh physics is enabled, the first root level mesh is the collision object.
        if(top_model[i]->hasPhysics == true){
            printMessage("loading physics ", STATUS);
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
            transform.setRotation(tobt(rot));

            //mass field
            float mass;
            try{
                mass = modelJson["models"][i]["mass"]; //NOTE: to make an object static, mass and inerta must be set to 0
            }catch(json::type_error& e){
                printMessage("mass field missing, setting to default (0) ", e.what() , WARNING);
                mass = 0;
            }
            top_model[i]->mass = mass;
            if(mass != 0)
                 top_model[i]->isDynamic = true;

            float friction, rotatingFriction;
            try{
                friction = modelJson["models"][i]["friction"];
                rotatingFriction = modelJson["models"][i]["rotatingfriction"];
            }catch(json::type_error& e){
                printMessage("friction fields missing, setting to default (0) ", e.what() , WARNING);
                friction = 0;
                rotatingFriction = 0;
            }

            int numTriangles = top_model[i]->meshes[0]->indices.size() / 3 ;
            uint* triangleIndexBase = &(top_model[i]->meshes[0]->indices[0]);
            int triangleIndexStride = sizeof(top_model[i]->meshes[0]->indices[0]) * 3;

            int numVertices = top_model[i]->meshes[0]->vertices.size();
            btScalar * vertexBase = (btScalar*)&(top_model[i]->meshes[0]->vertices[0]);
            int vertexStride = sizeof(Vertex);

            try{
                top_model[i]->useSinglePrimitive = modelJson["models"][i]["useSinglePrimitive"];
            } catch(json::type_error& e){
                printMessage("useSinglePrimitive field missing, setting to default (false) ", e.what() , WARNING);
                top_model[i]->useSinglePrimitive = false;
            }

            if(top_model[i]->useSinglePrimitive == true){
                string primitiveType = modelJson["models"][i]["primitiveType"];
                top_model[i]->primitiveType = primitiveType;

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
                    updateMessage(" [invalid primitive passed]");
                    updateMessageType(ERROR);
                }

            }else{
                //mesh primitive TODO: handle non-static meshes. At the moment, any mesh is created static.

                btTriangleIndexVertexArray* indexVertexArrays = new btTriangleIndexVertexArray(numTriangles, (int*)(size_t)triangleIndexBase, triangleIndexStride, numVertices, vertexBase, vertexStride );
                bool useQuantizedAabbCompression = true;
                //set collisionShape to be a meshShape
                top_model[i]->collisionShape = new btBvhTriangleMeshShape(indexVertexArrays, useQuantizedAabbCompression);

            }


            top_model[i]->collisionShape->setUserPointer(top_model[i]);
            top_model[i]->collisionShape->setLocalScaling(btVector3(scale, scale, scale));

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
            top_model[i]->collisionType = collisionType;
            top_model[i]->origionalCollisionGroup = getCollisionGroupByString(collisionType);
            top_model[i]->origionalCollisionMask = getCollisionMaskByString(collisionType);
            // syntax: addRigidBody( body, group, mask);
            dynamicsWorld->addRigidBody(top_model[i]->body, top_model[i]->origionalCollisionGroup, top_model[i]->origionalCollisionMask);
        }
        updateMessageType(SUCCESS);
    }
}

void unloadModels(){

//structure:
    //top level shader tree
        //meshes of each shader
            //vertex vector
            //index vector
            //opengl stuff
    //top level model tree
        //modelPositions
        //modelMatrices
    //top level texture tree
        //opengl textures
    //physics trees


    //shader
    for(uint i = 0; i < top_shader.size(); i++){
        //meshes
        for(uint j = 0; j < top_shader[i].meshes.size(); j++){
            top_shader[i].meshes[j]->vertices.clear();
            top_shader[i].meshes[j]->indices.clear();
            top_shader[i].meshes[j]->vertices.shrink_to_fit();
            top_shader[i].meshes[j]->indices.shrink_to_fit();
            //opengl buffers
            unloadModels_buffers_gl(i, j);
        }
        top_shader[i].meshes.clear();
        top_shader[i].meshes.shrink_to_fit();
        //shader GL objects:
        unloadModels_shaders_gl(i);

    }
    top_shader.clear();
    top_shader.shrink_to_fit();

    //model
    for (uint i = 0; i < top_model.size(); i++){
        top_model[i]->modelMatrices.clear();
        top_model[i]->modelPositions.clear();
        top_model[i]->modelMatrices.shrink_to_fit();
        top_model[i]->modelPositions.shrink_to_fit();
        top_model[i]->meshes.clear();
        top_model[i]->meshes.shrink_to_fit();
    }
    top_model.clear();
    top_model.shrink_to_fit();

    //texture
    for(uint i = 0; i < top_texture.size(); i++){
        glDeleteTextures(1, &top_texture[i].id);
    }
    top_texture.clear();
    top_texture.shrink_to_fit();

    //physics
    //https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=696

    //delete global physics objects:
    delete dynamicsWorld;
    delete collisionConfiguration;
    delete dispatcher;
    delete overlappingPairCache;
    delete solver;

    //recreate global physics objects: (now they are fresh blanks
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);


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
        updateMessage("[difuse] ");
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
        updateMessage("[specular] ");
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
        updateMessage("[normal/height] ");
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
        updateMessage("[height/ambient] ");
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
    processMesh_buffers_gl(mesh);
}


void saveJson(string jsonPath){
    json exportJson;
    //shaders:
    for(uint i = 0; i < top_shader.size(); i++){
        exportJson["shaders"][i]["name"] = top_shader[i].name;
        exportJson["shaders"][i]["vPath"] = top_shader[i].vPath;
        exportJson["shaders"][i]["fPath"] = top_shader[i].fPath;

    }
    //models:
    for(uint i = 0; i < top_model.size(); i++){
        exportJson["models"][i]["path"] = top_model[i]->objectPath;
        exportJson["models"][i]["scale"] = getScale(top_model[i]).x;
        exportJson["models"][i]["isInstanced"] = top_model[i]->isInstanced;
        exportJson["models"][i]["meshCountHint"] = top_model[i]->meshes.size();

        //physics fields
        exportJson["models"][i]["hasPhysics"] = top_model[i]->hasPhysics;
        if(top_model[i]->hasPhysics){
            exportJson["models"][i]["collisionType"] = top_model[i]->collisionType;
            exportJson["models"][i]["mass"] = top_model[i]->mass;
            exportJson["models"][i]["inerta"] = top_model[i]->inerta;
            exportJson["models"][i]["friction"] = top_model[i]->body->getFriction();
            exportJson["models"][i]["rotatingfriction"] = top_model[i]->body->getRollingFriction();
            exportJson["models"][i]["primitiveType"] = top_model[i]->primitiveType;

            exportJson["models"][i]["useSinglePrimitive"] = top_model[i]->useSinglePrimitive;
            if(top_model[i]->useSinglePrimitive){
                if(top_model[i]->primitiveType == "sphere"){
                    exportJson["models"][i]["primitiveSize"] = getScale(top_model[i]).x;
                }else if(top_model[i]->primitiveType == "cylinder"){
                    fprintf(stderr, "\n\n>>>>NEED TO ADD SUPPORT FOR THIS\n\n");
                }
            }
        }

        //fields that exist per mesh:
        for(uint j = 0; j < top_model[i]->meshes.size(); j++){
            exportJson["models"][i]["shaders"][j] = top_model[i]->meshes[j]->parentShader->name;
        }
        //instancing fields
        if(!top_model[i]->isInstanced){
            glm::vec3 pos = getPos(top_model[i]);
            exportJson["models"][i]["pos"][0] = pos.x;
            exportJson["models"][i]["pos"][1] = pos.y;
            exportJson["models"][i]["pos"][2] = pos.z;

            float scale = getScale(top_model[i]).x;
            exportJson["models"][i]["scale"] = scale;

            glm::quat rot = getRot(top_model[i]);
            exportJson["models"][i]["rotquat"][0] = rot.x;
            exportJson["models"][i]["rotquat"][1] = rot.y;
            exportJson["models"][i]["rotquat"][2] = rot.z;
            exportJson["models"][i]["rotquat"][3] = rot.w;
        }else{
            for(uint j = 0; j < top_model[i]->modelMatrices.size(); j++){
                glm::vec3 pos = getPosInstanced(top_model[i], j);
                exportJson["models"][i]["pos"][3*j + 0] = pos.x;
                exportJson["models"][i]["pos"][3*j + 1] = pos.y;
                exportJson["models"][i]["pos"][3*j + 2] = pos.z;

                glm::vec3 scale = getScaleInstanced(top_model[i], j);
                exportJson["models"][i]["scale"][3*j + 0] = scale.x;

                glm::quat rot = getRotInstanced(top_model[i], j);
                exportJson["models"][i]["rotquat"][3*j + 0] = rot.x;
                exportJson["models"][i]["rotquat"][3*j + 1] = rot.y;
                exportJson["models"][i]["rotquat"][3*j + 2] = rot.z;
                exportJson["models"][i]["rotquat"][3*j + 3] = rot.w;
            }
        }
    }

    //write to file
    ofstream outputFile;
    outputFile.open (jsonPath);
    outputFile << exportJson.dump(3);
    outputFile.close();
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
    unsigned int textureID = TextureFromFile_gl(filename);
    return textureID;
}

void InitializePhysicsWorld(){
    dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
    debugDraw.loadDebugShaders();
    dynamicsWorld->setDebugDrawer(&debugDraw);
}

void RunStepSimulation(){
    dynamicsWorld->stepSimulation(getPhysicsFrameTime(), 10);
}

//helper conversion functions:
btVector3 tobt(glm::vec3 vec){
    return btVector3(vec.x, vec.y, vec.z);
}

glm::vec3 toglm(btVector3 vec){
    return glm::vec3(vec.x(), vec.y(), vec.z());
}

btQuaternion tobt(glm::quat quat){
    return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

glm::quat toglm(btQuaternion quat){
    return glm::quat(quat.x(), quat.y(), quat.z(), quat.w());
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

void updateModelPosition(Model* model, glm::vec3 newPos){
    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    pos = newPos;
    btTransform tr = model->body->getWorldTransform();
    tr.setOrigin(btVector3(pos.x,pos.y,pos.z));
    model->body->setWorldTransform(tr);
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);

}

void updateModelPosition(Model* model, btVector3 newPos){
    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    pos = glm::vec3(newPos.x(), newPos.y(), newPos.z());
    btTransform tr = model->body->getWorldTransform();
    tr.setOrigin(newPos);
    model->body->setWorldTransform(tr);
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);

}

void updateModelRotation(Model* model, glm::quat newRotation){
    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    rotation = newRotation;
    btTransform tr = model->body->getWorldTransform();
    tr.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    model->body->setWorldTransform(tr);
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);

}

void updateRelativeModelRotation(Model* model, glm::quat newRotation){
    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    rotation = rotation * newRotation;
    btTransform tr = model->body->getWorldTransform();
    tr.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    model->body->setWorldTransform(tr);
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);
}

void updateRelativeModelRotation(Model* model, glm::vec3 newRotation){
    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    rotation = rotation * glm::quat(newRotation);
    btTransform tr = model->body->getWorldTransform();
    tr.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    model->body->setWorldTransform(tr);
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);
}

glm::vec3 getPos(Model* model){
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    return pos;
}

glm::vec3 getPosInstanced(Model* model, int instance){
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[instance], scale, rotation, pos, skew, perspective);
    return pos;
}

glm::vec3 getScaleInstanced(Model* model, int instance){
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[instance], scale, rotation, pos, skew, perspective);
    return scale;
}

glm::vec3 getScale(Model* model){
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    return scale;
}

glm::quat getRot(Model* model){
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    return rotation;
}

glm::quat getRotInstanced(Model* model, int instance){
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[instance], scale, rotation, pos, skew, perspective);
    return rotation;
}

void updateScale(Model* model, glm::vec3 newScale){
    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    scale = newScale;
    model->collisionShape->setLocalScaling( btVector3(scale.x, scale.y, scale.z));
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);

}

void updateRelativeScale(Model* model, glm::vec3 changeScaleBy){

    //pull data
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 pos;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model->modelMatrices[0], scale, rotation, pos, skew, perspective);
    //update data: openGL and physics
    fprintf(stderr, "oldScale = %f\n\n", scale.y);
    scale += changeScaleBy;
    fprintf(stderr, "changeScale = %f\n", changeScaleBy.y);
    fprintf(stderr, "newscale = %f\n\n", scale.y);
    model->collisionShape->setLocalScaling( btVector3(scale.y, scale.y, scale.y));
    //reconstruct matrix
    model->modelMatrices[0] = glm::mat4(1.0); //identity matrix
    model->modelMatrices[0] = glm::scale(model->modelMatrices[0], scale);
    model->modelMatrices[0] = glm::mat4_cast(rotation) * model->modelMatrices[0];
    model->modelMatrices[0] = glm::translate(model->modelMatrices[0], pos);

}
