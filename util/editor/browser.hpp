#ifndef BROWSER_H
#define BROWSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

#include <iostream>
#include "util/imgui/imgui.h"
#include "util/imgui/imgui_impl_glfw.h"
#include "util/imgui/imgui_impl_opengl3.h"
extern bool hasSaved;
extern string savePath;
extern bool hasOpened;
extern string openPath;

int getFiles();
int drawBrowser(bool saving, string matches);

#endif
