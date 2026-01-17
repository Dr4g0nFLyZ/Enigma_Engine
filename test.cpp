#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <fstream>

#include <wasm.h>
#include <wasmtime.h>

#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
   // glfw: initialize and configure
   // ------------------------------
   glfwInit();
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   #ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   #endif

   glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // 1. Removes the window border/title bar (Borderless)
   glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // 2. Enables a transparent window framebuffer
   glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // 3. Forces the window to always be on top (Always On Top)

   glfwWindowHint(GLFW_DEPTH_BITS, 24); // Good practice
   glfwWindowHint(GLFW_STENCIL_BITS, 8); // Good practice

   // Add these to explicitly request an 8-bit alpha buffer
   glfwWindowHint(GLFW_ALPHA_BITS, 8);

   // glfw window creation
   // --------------------
   GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Enigma Engine", NULL, NULL);
   if (window == NULL)
   {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
   }

   glfwSetWindowAttrib(window, GLFW_FLOATING, GLFW_TRUE);
   glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

   glfwMakeContextCurrent(window);
   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

   // glad: load all OpenGL function pointers
   // ---------------------------------------
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
   }

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // build and compile our shader program
   // ------------------------------------
   Shader ourShader("main.vert", "main.frag"); // you can name your shader files however you like

   unsigned int VAO;
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
   // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
   // glBindVertexArray(0);

   // --- Setup the Engine ---
   wasm_config_t* config = wasm_config_new();
   wasm_engine_t* engine = wasm_engine_new_with_config(config);
   wasmtime_store_t* store = wasmtime_store_new(engine, NULL, NULL);
   wasmtime_context_t* context = wasmtime_store_context(store);

   // --- Load the WASM File ---
   std::ifstream file("main.wasm", std::ios::binary | std::ios::ate);
   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);
   std::vector<uint8_t> binary(size);
   file.read((char*)binary.data(), size);

   // --- Compile the Module ---
   wasmtime_module_t* module = NULL;
   wasmtime_error_t* error = wasmtime_module_new(engine, binary.data(), binary.size(), &module);
   if (error) return 1;

   // --- Instantiate the Module ---
   wasmtime_instance_t instance;
   error = wasmtime_instance_new(context, module, NULL, 0, &instance, NULL);
   if (error) return 1;

   // 1. Get the 'get_buffer_pointer' function
   wasmtime_extern_t get_ptr_item;
   wasmtime_instance_export_get(context, &instance, "get_buffer_pointer", 18, &get_ptr_item);
    
   // 2. Call it to find out WHERE to write in WASM memory
   wasmtime_val_t ptr_result[1];
   wasmtime_func_call(context, &get_ptr_item.of.func, NULL, 0, ptr_result, 1, NULL);
   uint32_t wasm_buffer_offset = ptr_result[0].of.i32;

   // 3. Get the Memory Export and write the string
   wasmtime_extern_t mem_item;
   wasmtime_instance_export_get(context, &instance, "memory", 6, &mem_item);
   uint8_t* memory_base = wasmtime_memory_data(context, &mem_item.of.memory);

   std::string message = "WebAssembly is excellent!";
   memcpy(memory_base + wasm_buffer_offset, message.c_str(), message.length());

   // 4. Call 'process_string' with the length of the string we just wrote
   wasmtime_extern_t process_item;
   wasmtime_instance_export_get(context, &instance, "process_string", 14, &process_item);

   wasmtime_val_t args[1];
   args[0].kind = WASMTIME_I32;
   args[0].of.i32 = message.length();

   wasmtime_val_t results[1];
   wasmtime_func_call(context, &process_item.of.func, args, 1, results, 1, NULL);

   std::cout << "Number of 'e's found by Zig: " << results[0].of.i32 << std::endl;
 
   // render loop
   // -----------
   while (!glfwWindowShouldClose(window))
   {
      // input
      // -----
      processInput(window);

      // render
      // ------
      //glClearColor(0.2f, 0.3f, 0.3f, 0.0f);
      //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // render the triangle
      ourShader.use();
      glBindVertexArray(VAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   // optional: de-allocate all resources once they've outlived their purpose:
   // ------------------------------------------------------------------------
   glDeleteVertexArrays(1, &VAO);

   // glfw: terminate, clearing all previously allocated GLFW resources.
   // ------------------------------------------------------------------
   glfwTerminate();

   // --- Cleanup ---
   wasmtime_module_delete(module);
   wasmtime_store_delete(store);
   wasm_engine_delete(engine);

   return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
   // make sure the viewport matches the new window dimensions; note that width and
   // height will be significantly larger than specified on retina displays.
   glViewport(0, 0, width, height);
}
