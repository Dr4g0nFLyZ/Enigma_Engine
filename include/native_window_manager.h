#ifndef NATIVE_WINDOW_MANAGER_H
#define NATIVE_WINDOW_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class NativeWindowManager {
public:
   NativeWindowManager(unsigned int width, unsigned int height, const char* title)
   : width(width), height(height), title(title), window(nullptr) {}

   ~NativeWindowManager() {
      if (window) glfwDestroyWindow(window);
      glfwTerminate();
   }

   bool init() {
      if (!glfwInit()) return false;

      // Configure OpenGL version and profile
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

      #ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      #endif

      // Specialized hints for "Overlay" style windows
      glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
      glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
      glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
      glfwWindowHint(GLFW_ALPHA_BITS, 8);

      window = glfwCreateWindow(width, height, title, NULL, NULL);
      if (!window) {
         glfwTerminate();
         return false;
      }

      glfwMakeContextCurrent(window);

      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
         return false;
      }

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      return true;
   }

   GLFWwindow* getWindow() const { return window; }
   bool shouldClose() const { return glfwWindowShouldClose(window); }
   void swapBuffers() { glfwSwapBuffers(window); glfwPollEvents(); }

private:
   unsigned int width, height;
   const char* title;
   GLFWwindow* window;
};

#endif
