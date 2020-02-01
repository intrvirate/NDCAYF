#ifndef RENDER2D_HPP
#define RENDER2D_HPP

void loadTextDataSpacing();
void addTextString(std::string text, float x, float y, float size, glm::vec3 activeColor, glm::vec3 passiveColor);
void clearTextStrings();
void set2DletterQuad(char c, float xPos, float yPos, float xSize, float ySize);
void load2DTextTexture();
void load2DShaders();
void load2DBuffers();
void enableTextMouseColor(bool state);
void setTextMouseColor(glm::vec3 colorIn);
void drawAllText();
void renderLoop2D(GLFWwindow *window);
void updateFPSCounter();


#endif // RENDER2D_HPP
