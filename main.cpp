#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#include "font_engine.h"
//#include "gl_widget.h"
#include "wasm_manager.h"
#include "database_manager.h"
#include "native_window_manager.h"

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);

   FontEngine fonts;
   if (!fonts.init("FiraMono-Regular.ttf", 48)) {
      return -1;
   }

   if (!fonts.loadAndPrintGlyph('A')) {
      return -1;
   }

   NativeWindowManager nativeWin(SCR_WIDTH, SCR_HEIGHT, "Enigma Engine");
   if (!nativeWin.init()) {
      return -1;
   }

   try {
      WasmManager wasm("main.wasm");

      std::string message = "WebAssembly is excellent!";
      int32_t count = wasm.process_string(message);
      std::cout << "Number of 'e's found by Zig: " << count << std::endl;

      DatabaseManager dbManager("app_data.db");
      if (dbManager.open()) {
         dbManager.logMessage(message);
      }

      uint32_t v_offset = wasm.get_wasm_ptr("get_vertex_ptr");
      float* triangle_data_ptr = static_cast<float*>(wasm.get_memory_ptr(v_offset));
      size_t data_size = 3 * 6 * sizeof(float);
/*
      QWidget mainContainer;
      QVBoxLayout *layout = new QVBoxLayout(&mainContainer);

      QPushButton *btn = new QPushButton("Refresh Wasm Data");
      MyGLWidget *glWidget = new MyGLWidget();

      glWidget->setVertexData(triangle_data_ptr, data_size);

      QObject::connect(btn, &QPushButton::clicked, [glWidget, triangle_data_ptr, data_size]() {
         qDebug() << "Refreshing triangle data...";
         glWidget->setVertexData(triangle_data_ptr, data_size);
         glWidget->update();
      });

      layout->addWidget(btn);
      layout->addWidget(glWidget);

      mainContainer.resize(SCR_WIDTH, SCR_HEIGHT);
      mainContainer.show();

      return app.exec();
*/
   } catch (const std::exception& e) {
      std::cerr << "Fatal Error: " << e.what() << std::endl;
      return 1;
   }
}
