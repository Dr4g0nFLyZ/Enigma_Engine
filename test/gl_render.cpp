#include <iostream>
#include <vector>
#include <map>
#include <cmath> // For std::ceil
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 640;

// Structure to hold bitmap data for a single character
struct CharBitmap {
   std::vector<unsigned char> data;
   int width;
   int height;
};

// Function to generate a bitmap for a single character
CharBitmap generateBitmap(FT_Face face, char character, int tileSize) {
   FT_UInt glyph_index = FT_Get_Char_Index(face, character);
   FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
   FT_GlyphSlot slot = face->glyph;

   int width = slot->bitmap.width;
   int height = slot->bitmap.rows;

   // Center the character within the tile
   int offsetX = (tileSize - width) / 2;
   int offsetY = (tileSize - height) / 2;

   CharBitmap bitmap;
   bitmap.width = tileSize;
   bitmap.height = tileSize;
   bitmap.data.resize(tileSize * tileSize, 0); // Initialize with background color (black)

   for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
         if (x + offsetX >= 0 && x + offsetX < tileSize && y + offsetY >= 0 && y + offsetY < tileSize) {
            bitmap.data[(y + offsetY) * tileSize + (x + offsetX)] = slot->bitmap.buffer[y * width + x];
         }
      }
   }
   return bitmap;
}

// Function to render the bitmap tileset to the window
void renderTilesetToWindow(const std::map<char, CharBitmap>& tileset, int tileSize, Shader& shader, unsigned int textureID) {
   if (tileset.empty()) {
      std::cerr << "Error: Tileset is empty, cannot render." << std::endl;
      return;
   }

   // Calculate the dimensions of the tileset texture
   int numChars = tileset.size();
   int tilesPerRow = static_cast<int>(std::ceil(std::sqrt(numChars)));
   int numRows = static_cast<int>(std::ceil(static_cast<double>(numChars) / tilesPerRow));
   int textureWidth = tilesPerRow * tileSize;
   int textureHeight = numRows * tileSize;

   std::vector<unsigned char> textureData(textureWidth * textureHeight, 0);

   int tileIndex = 0;
   for (const auto& pair : tileset) {
      const CharBitmap& charBitmap = pair.second;
      int tileX = (tileIndex % tilesPerRow) * tileSize;
      int tileY = (tileIndex / tilesPerRow) * tileSize;

      for (int y = 0; y < charBitmap.height; ++y) {
         for (int x = 0; x < charBitmap.width; ++x) {
            textureData[(tileY + y) * textureWidth + (tileX + x)] = charBitmap.data[y * charBitmap.width + x];
         }
      }
      tileIndex++;
   }

   // Bind the texture
   glBindTexture(GL_TEXTURE_2D, textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, textureData.data());

   // Set texture parameters
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   // Set up vertex data and configure vertex attributes
   float vertices[] = {
      // positions   // texCoords
      -1.0f,  1.0f,   0.0f, 1.0f,
      -1.0f, -1.0f,   0.0f, 0.0f,
      1.0f, -1.0f,   1.0f, 0.0f,

      -1.0f,  1.0f,   0.0f, 1.0f,
      1.0f, -1.0f,   1.0f, 0.0f,
      1.0f,  1.0f,   1.0f, 1.0f
   };

   unsigned int VBO, VAO;
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);

   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   // position attribute
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   // texture coord attribute
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   glEnableVertexAttribArray(1);

   // Activate the shader
   shader.use();
   glUniform1i(glGetUniformLocation(shader.ID, "texture0"), 0);

   // Render the quad
   glBindVertexArray(VAO);
   glDrawArrays(GL_TRIANGLES, 0, 6);

   // Clean up
   glDeleteVertexArrays(1, &VAO);
   glDeleteBuffers(1, &VBO);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
   glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
}

int main() {
   FT_Library library;
   FT_Face face;
   const char* fontPath = "FiraMono-Regular.ttf"; // Replace with your font path
   int fontSize = 48;
   int tileSize = 64; // Size of each square tile

   // Initialize FreeType
   if (FT_Init_FreeType(&library)) {
      std::cerr << "Error: Could not initialize FreeType Library" << std::endl;
      return 1;
   }

   // Load the font face
   if (FT_New_Face(library, fontPath, 0, &face)) {
      std::cerr << "Error: Could not open or load font file: " << fontPath << std::endl;
      FT_Done_FreeType(library);
      return 1;
   }

   // Set the font size (in pixels)
   FT_Set_Pixel_Sizes(face, 0, fontSize);

   // Store the bitmap data for each ASCII character
   std::map<char, CharBitmap> fontTileset;

   // Generate bitmaps for printable ASCII characters (from space ' ' to tilde '~')
   for (char c = 32; c <= 126; ++c) {
      fontTileset[c] = generateBitmap(face, c, tileSize);
      std::cout << "Generated bitmap for character: " << c << std::endl;
   }

   // glfw: initialize and configure
   // ------------------------------
   glfwInit();
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   #ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   #endif

   // glfw window creation
   // --------------------
   GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Font Tileset", NULL, NULL);
   if (window == NULL)
   {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
   }
   glfwMakeContextCurrent(window);
   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

   // glad: load all OpenGL function pointers
   // ---------------------------------------
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
   }

   // build and compile our shader program
   // ------------------------------------
   Shader ourShader("glsl/gl_render.vert", "glsl/gl_render.frag"); // Assuming you have these shader files

   // Create a texture object
   unsigned int tilesetTexture;
   glGenTextures(1, &tilesetTexture);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, tilesetTexture);

   // Render loop
   // -----------
   while (!glfwWindowShouldClose(window))
   {
      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // Render the tileset to the window
      renderTilesetToWindow(fontTileset, tileSize, ourShader, tilesetTexture);

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   // Clean up FreeType resources
   FT_Done_Face(face);
   FT_Done_FreeType(library);

   // glfw: terminate, clearing all previously allocated GLFW resources.
   // ------------------------------------------------------------------
   glfwTerminate();
   return 0;
}
