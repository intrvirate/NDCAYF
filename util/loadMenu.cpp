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
#include "util/globalStateHandlers.hpp"


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

//empty = not in setting menu
//set = name of the setting field currently displayed
string settingMenu = "";

void setJsonDebugMode(bool mode){
    debugMode = mode;
}


void updateMenu(){
    clearTextStrings();
    glm::vec3 activeColor;
    glm::vec3 passiveColor;
    string name;
    if(settingMenu == ""){
        inTextBox = false;
        //render menu item
        for(int i = 0; !menujson[currentMenu][i].is_null(); i++){
            name = menujson[currentMenu][i]["name"];
            if(menujson[currentMenu][i]["type"] == "menu"){
                activeColor  = glm::vec3(0.9, 0.9, 0.1);
                passiveColor = glm::vec3(0.2, 0.5, 0.0);
            }else if (menujson[currentMenu][i]["type"] == "setting"){
                activeColor  = glm::vec3(0.9, 0.9, 0.9);
                passiveColor = glm::vec3(0.2, 0.9, 0.0);
            }
            addTextString(name, menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, activeColor, passiveColor );
        }
    }else{
        inTextBox = true; //start reading text from keyboard

        //render setting item
        activeColor  = glm::vec3(0.1, 0.1, 0.1);
        passiveColor = glm::vec3(0.3, 0.3, 0.3);
        int i = 0; //will be used for spacing, not used yet

        if(!settingsjson[settingMenu].is_null()){

            if(settingsjson[settingMenu]["type"] == "bool"){
                name = settingsjson[settingMenu]["name"];
                    //first entry:  name
                addTextString(name, menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, passiveColor, passiveColor ); //both passive color so it doesn't highlight when moused over

                i++;//second entry: true
                addTextString(settingsjson[settingMenu]["true"], menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, activeColor, passiveColor );

                i++;//third entry:  true
                addTextString(settingsjson[settingMenu]["false"], menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, activeColor, passiveColor );

                i++;//last entry:   back
                addTextString("back", menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, activeColor, passiveColor );

            }else if(settingsjson[settingMenu]["type"] == "int"){

            }else if(settingsjson[settingMenu]["type"] == "string"){
                name = settingsjson[settingMenu]["name"];
                    //first entry:  name
                addTextString(name, menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, passiveColor, passiveColor ); //both passive color so it doesn't highlight when moused over

                i++;//second entry: text entry
                addTextString(textEntryString, menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, passiveColor, passiveColor ); //both passive color so it doesn't highlight when moused over

                i++;//last entry:   back
                addTextString("back", menuLeftOffsetFromCenter, menuTopOffsetFromCenter - (menuSpacing*i) , menuTextSize, activeColor, passiveColor );

            }

        }
    }
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

    updateMenu();

    return 0;
}

void handleMenuClick(){

    float clickY = getMousePos().y;

    clickY = clickY + 1;
    clickY = (-clickY + 2);

    clickY -= 1 - menuTopOffsetFromCenter;
    clickY += menuSpacing;
    clickY = clickY/menuSpacing;

    int index = clickY;
    index = index-1; //index now is == to the row that was clicked on (zero indexed)

    //fprintf(stderr, "f = %f, i = %d\n diff=%f", clickY, index, clickY - (float)index-1);

    if(index >= 0){
       fprintf(stderr, "clickedn");
       if(settingMenu == ""){ //are we in a setting menu? "" means no.
           if((!menujson[currentMenu][index].is_null())){
                //fprintf(stderr, "clicked!\n");
                //cout << menujson[currentMenu][index]["name"] << endl;

                if( menujson[currentMenu][index]["type"] == "menu"){
                    currentMenu = menujson[currentMenu][index]["target"];
                }else if(menujson[currentMenu][index]["type"] == "setting"){
                    settingMenu = menujson[currentMenu][index]["target"];
                }

                updateMenu();

           }
       }else{ //is a setting menu
           if(!settingsjson[settingMenu].is_null()){ //sanity check

               //if setting is boolean
               if(settingsjson[settingMenu]["type"] == "bool"){


                   for(uint i = 0; i < boolLinkArraySize; i++){
                       if (boolLinkArray[i].ID == settingMenu){
                            //1 is name, can't click it. TODO: remove mouse highlight for it

                            if(index == 1){//first line is true

                                *(boolLinkArray[i].ptr) = true; //set true

                            }else if(index == 2){

                                *(boolLinkArray[i].ptr) = false; //set true

                            }else if(index == 3){//3 == "back"

                                settingMenu = "";//exit menu

                            }

                            break; //exit for loop
                       }
                   }

               }
               //else if setting is int
               else if(settingsjson[settingMenu]["type"] == "int"){


               }
               //else if setting is string
               else if(settingsjson[settingMenu]["type"] == "string"){

                   if(index == 2){//2 == "back"
                       textEntryString = "";
                       settingMenu = "";//exit menu
                   }

               }else{
                   //unknown type. ATM, this just returns to the menu above the setting
               }

               updateMenu();
           }
       }
    }
}








