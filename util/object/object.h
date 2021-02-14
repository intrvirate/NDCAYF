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
#include "util/json.hpp"

extern btDiscreteDynamicsWorld* dynamicsWorld;
extern BulletDebugDrawer_OpenGL debugDraw;

using namespace std;
using json = nlohmann::json;

struct Model;   //needed because of circular references
struct Shader;  //and yes, I don't care that this is bad practice. Performace < Style

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

//need to be extern because object_gl directly uses these
extern vector<Shader>  top_shader;          //top level render tree
extern vector<Model*>   top_model;          //top level model tree
extern vector<TextureLookup> top_texture;   //top level texture tree
extern json modelJson;

/*
 * Top level render tree; vector of shaders, which each have a vector of meshes
 *
 * These structs define the how the scene/object data is held in memory.
 * For performance reasons, data is organized by shader, not by object/model.
 * This allows for iterating over the struct tree at render time with minimal
 * state change overhead.
 * It also allows the use of different shaders for different parts of a single model,
 * for example shading the leaves of a tree differently than it's trunk.
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
    Shader* parentShader;
    bool showObjectSelection; //whether to highlight this mesh when it's object is selected
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
    string collisionType;
    //glm::vec3 scale;
    //glm::vec3 pos;
    //glm::quat rotation;
    //glm::mat4 model; //now stored as the first value in the modelMatrices vector
    //physics
    bool hasPhysics;
    bool isDynamic;
    bool useSinglePrimitive;
    string primitiveType;
    int16_t origionalCollisionGroup;
    int16_t origionalCollisionMask;
    btRigidBody* body;
    btCollisionShape* collisionShape;
    float mass;
    float inerta;
    //instancing
    bool isInstanced;
    unsigned int instanceCount;
    vector<glm::vec3> modelPositions;
    vector<glm::mat4> modelMatrices;
    //TODO: figure out how to instance bullet objects
    //TODO: networking

};

//=================================function prototypes==================================

void loadModels(string jsonPath); //loads models from a json file
void unloadModels();
collisionMasks getCollisionGroupByString(string str);
collisionGroups getCollisionMaskByString(string str);
void processMesh(Mesh *mesh, aiMesh *impMesh, const aiScene *assimpModel, string directory);
void saveJson(string jsonPath);
unsigned int findTextureID(const char* path);
unsigned int TextureFromFile(const char *path, const string &directory);
void InitializePhysicsWorld();
void RunStepSimulation();

//utility functions: use to access and manipulate model state
btVector3 tobt(glm::vec3 vec);
glm::vec3 toglm(btVector3 vec);
btQuaternion tobt(glm::quat quat);
glm::quat toglm(btQuaternion quat);
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
glm::vec3 getPos(Model* model);
glm::vec3 getPosInstanced(Model* model, int instance);
glm::vec3 getScale(Model* model);
glm::vec3 getScaleInstanced(Model* model, int instance);
glm::quat getRot(Model* model);
glm::quat getRotInstanced(Model* model, int instance);
void updateScale(Model* model, glm::vec3 scale);
void updateRelativeScale(Model* model, glm::vec3 scale);

#endif // OBJECT_H
