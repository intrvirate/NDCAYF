#ifndef RENDER2D_HPP
#define RENDER2D_HPP

void addTextString(std::string text, float x, float y, float size);
void stringToLeterQuad(char *text);
void set2DletterQuad(char c, float xPos, float yPos, float xSize, float ySize);
void load2DTextTexture();
void load2DShaders();
void load2DBuffers();
void drawAllText();
void renderLoop2D(GLFWwindow *window);
void updateFPSCounter();


#endif // RENDER2D_HPP
