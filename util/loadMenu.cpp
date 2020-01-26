#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util/json.hpp"
#include "util/handleinput.hpp"
#include "util/render/render2D.hpp"


using json = nlohmann::json;
using namespace std;

bool debugMode = false;

//constants that describe location and size of the menu on screen
const float menuTopOffsetFromCenter = 0.9;
const float menuLeftOffsetFromCenter = -0.3;
const float menuSpacing = 0.1;
const float menuTextSize = 0.07;

json menujson;
json settingsjson;

string currentMenu = "root";

void setJsonDebugMode(bool mode){
    debugMode = mode;
}

int buildMenu(){

    //open files
    std::ifstream jsonMenuFile("gamedata/menu.json", std::ios::in);
    std::ifstream jsonSettingsFile("gamedata/settings.json", std::ios::in);

    //check files
    if(!jsonMenuFile.is_open()){
        printf("ERROR: unable to open menu file");
        jsonMenuFile.close();
        exit(1);
    }
    if(!jsonSettingsFile.is_open()){
        printf("ERROR: unable to open menu file");
        jsonSettingsFile.close();
        exit(1);
    }

    //read to string
    std::stringstream jsonMenuString;
    jsonMenuString << jsonMenuFile.rdbuf();

    std::stringstream jsonSettingsString;
    jsonSettingsString << jsonSettingsFile.rdbuf();

    //close files
    jsonMenuFile.close();
    jsonSettingsFile.close();

    //parse
    try{
         menujson = json::parse(jsonMenuString.str());

    }
    catch (json::parse_error& e) {
        std::cout << e.what() << endl; //print error
    }

    try{
         settingsjson = json::parse(jsonSettingsString.str());

    }
    catch (json::parse_error& e) {
        std::cout << e.what() << endl; //print error
    }

    //print json to terminal
    if(debugMode == true){
        std::cout << menujson.dump(3) << endl;
        std::cout << settingsjson.dump(3) << endl;

    }

    for(int i = 0; !menujson[currentMenu][i].is_null(); i++){
        addTextString(menujson[currentMenu][i]["name"], menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize);
    }

    return 0;
}

/*
const float menuTopOffsetFromCenter
const float menuSpacing
const float menuTextSize
*/

void updateMenu(){
    clearTextStrings();
    for(int i = 0; !menujson[currentMenu][i].is_null(); i++){
        addTextString(menujson[currentMenu][i]["name"], menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize);
    }
}

void handleMenuClick(){

    float clickY = getMousePos().y;

    clickY = clickY + 1;
    clickY = (-clickY + 2);

    clickY -= 1 - menuTopOffsetFromCenter;
    clickY += menuSpacing;
    clickY = clickY/menuSpacing;

    int index = clickY;
    index = index-1;

    fprintf(stderr, "f = %f, i = %d\n diff=%f", clickY, index, clickY - (float)index-1);

    if(index >= 0){
        fprintf(stderr, "check1\n");
       if((!menujson[currentMenu][index].is_null())){
            fprintf(stderr, "clicked!\n");
            cout << menujson[currentMenu][index]["name"] << endl;

            currentMenu = menujson[currentMenu][index]["target"];

            updateMenu();




       }
    }

}




