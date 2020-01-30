#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util/globalStateHandlers.hpp"

using namespace std;

/*
 * this file stores all global configuration variables that
 * have a reason to be changed at runtime. All variables
 * here have:
 *      a variable definition
 *      a get function
 *      a set function
 *      a listing in the array of global configuration options
 *
 * For organazation, keep the order of variables constant across
 * all listings. This **INCLUDES** settings.json
 *
 * When adding new variables to this file,
 * use the same naming convention.
 *
 * Keep in mind that these variables are referenced from an external
 * json file, so avoid re-naming variables.
 */


bool FPScounter = false;
//false = display sec/frame, true = frame/sec
bool GetFPScounter()            { return FPScounter; }
void SetFPScounter(bool state)  { FPScounter = state; }


//link variables to setting name

const uint boolLinkArraySize = 1;

boolLink boolLinkArray[boolLinkArraySize]{
    {.ID = "FPScounter", .ptr = &FPScounter}

};

//int
const uint intLinkArraySize = 1;

boolLink intLinkArray[intLinkArraySize]{
    {.ID = "FPScounter", .ptr = &FPScounter}

};
























