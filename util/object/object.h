#ifndef OBJECT_H
#define OBJECT_H


#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <btBulletDynamicsCommon.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "util/bulletDebug/collisiondebugdrawer.hpp"
#include "collisionMaskClasses.hpp"

extern btDiscreteDynamicsWorld* dynamicsWorld;
extern BulletDebugDrawer_OpenGL debugDraw;

using namespace std;

struct Model;   //needed because of circular reference
                //and yes, I don't care that this is bad practice. Performace < Style

/*
 * Top level texture tree. This simply stores all loaded textures, preventing the same
 * texture from being loaded multiple times.
 *
 */

struct Texture {
    unsigned int defuseID; //these IDs are zero if no texture was set. See https://stackoverflow.com/questions/7322147/what-is-the-range-of-opengl-texture-id
    unsigned int specularID;
    unsigned int normalID;
    unsigned int heightID;
};

struct TextureLookup{//linkes each Id to it's path; used to prevent duplicate texture loading
    unsigned int id;
    string path;

};

/*
 * Top level render tree; vector of shaders
 *
 * These structs define the how the scene/object data is held in memory.
 * For performance reasons, data is organized by shader, not by object/model.
 * This allows for iterating over the struct tree at render time with minimal
 * state change overhead.
 * It also allows the use of different shaders for different parts of a single model,
 * for example shade the leaves of a tree differently than it's trunk.
 * High level state, such as whether to cull faces, is also stored at the shader level
 *
 * Note on instancing: multiple instanced meshes are stored as a single mesh in the tree,
 * with a flag storing whether it's instanced. This flag is stored both in the mesh, and
 * in the model. The array of position matrices is stored in only the model tree, but the
 * ID of the gpu buffer storing the same data is stored in the mesh and model trees.
 */

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct Mesh{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    Texture texture;
    unsigned int VAO, VBO, EBO;
    Model* parentModel;
    bool showObjectSelection; //whether to highlight this mesh when it's object is selected
    glm::mat4 model;
    //instancing
    bool isInstanced;
    unsigned int instancedMatrixBufferID;
    unsigned int instanceCount;
    unsigned int instanceModelBuffer;

};

struct Shader {
    string vPath;
    string fPath;
    unsigned int ID;
    vector<Mesh*> meshes;
    GLenum CullFaceState;
    string name; //shader name
    int meshCountHint; //used during world generation to reserve memory for meshes vector;
                       //do not assume its accurate. use meshes.size() instead.
    GLint projectionLocation;
    GLint viewLocation;
    GLint modelLocation;
    GLint lightPosLocation;
    GLint lightColorLocation;

    GLint texture_diffuse_location;
    GLint texture_specular_location;
    GLint texture_normal_location;
    GLint texture_height_location;
};

/*
 * Top level model tree. This is not used to render the scene, rather it handles
 * remembering what 'object' each mesh belongs to, and stores physics data belonging
 * to that object. Since many meshes do not need physics representations, it would be
 * redundant to store this in the render tree.
 *
 * Links between the model tree and the render tree are handled by reference; each mesh
 * stores what model it belongs to (each mesh has one parent model) and each model can
 * own multiple meshes.
 *
 */

struct Model {
    //graphics
    vector<Mesh*> meshes; //vector of pointers; actual mesh is dynamicaly allocated
    string directory;
    string objectPath;
    glm::vec3 scale;
    glm::vec3 pos;
    glm::quat rotation;
    //unsigned int instanceModelBuffer;
    //physics
    bool hasPhysics;
    bool isDynamic;
    bool useSinglePrimitive;
    int16_t origionalCollisionGroup;
    int16_t origionalCollisionMask;
    btRigidBody* body;
    btCollisionShape* collisionShape;
    float mass;
    float inerta;
    //instancing
    bool isInstanced;
    unsigned int instanceCount;
    vector<glm::mat4> modelMatrices;
    //TODO: figure out how to instance bullet objects
    //networking

};

//=================================function prototypes==================================

void loadModels(string jsonPath); //loads models from a json file
collisionMasks getCollisionGroupByString(string str);
collisionGroups getCollisionMaskByString(string str);
void processMesh(Mesh *mesh, aiMesh *impMesh, const aiScene *assimpModel, string directory);
unsigned int findTextureID(const char* path);
unsigned int TextureFromFile(const char *path, const string &directory);
void drawObjects();
void InitializePhysicsWorld();
void RunStepSimulation();

//utility functions: use to access and manipulate model state
void disableCollision(Model* model);
void enableCollision(Model* model);
Model* getModelPointerByName(string name);
void updateModelPosition(Model* model, glm::vec3 pos);
void updateModelPosition(Model* model, btVector3 pos);
void makeStatic(Model* model);
void makeDynamic(Model* model);
void updateModelRotation(Model* model, glm::quat rotation);
void updateRelativeModelRotation(Model* model, glm::quat rotation);
void updateRelativeModelRotation(Model* model, glm::vec3 rotation);
void syncMeshMatrices(Model* model);


#endif // OBJECT_H
