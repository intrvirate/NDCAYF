#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>

unsigned int loadCubemap(std::vector<std::string> faces);
void initializeSkybox();
void renderSkybox();


#endif // SKYBOX_H
