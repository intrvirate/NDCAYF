#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "util/json.hpp"

using json = nlohmann::json;
using namespace std;

bool debugMode = false;

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
    json menujson;
    json settingsjson;
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




    return 0;
}
