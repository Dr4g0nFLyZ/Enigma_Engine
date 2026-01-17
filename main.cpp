#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include <iostream>
#include <vector>
#include <fstream>

#include <wasm.h>
#include <wasmtime.h>

#include <ft2build.h>
#include FT_FREETYPE_H

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
   ~MyGLWidget() {
      // Cleanup OpenGL resources safely
      makeCurrent();
      vbo.destroy();
      doneCurrent();
   }

public:
   // Add a way to receive the data
   void setVertexData(float* data, size_t size) {
      wasm_data_ptr = data;
      wasm_data_size = size;
   }

protected:
   void initializeGL() override {
      initializeOpenGLFunctions();
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

      // 1. Define Shaders
      const char *vsrc = 
         "#version 440 core\n"
         "layout (location = 0) in vec3 aPos;\n"
         "layout (location = 1) in vec3 aColor;\n"
         "out vec3 ourColor;\n"
         "void main() {\n"
         "   gl_Position = vec4(aPos, 1.0);\n"
         "   ourColor = aColor;\n"
         "}\n";

      const char *fsrc =
         "#version 440 core\n"
         "out vec4 FragColor;\n"
         "in vec3 ourColor;\n"
         "void main() {\n"
         "   FragColor = vec4(ourColor, 1.0);\n" // Full opacity
         "}\n";

      // 2. Compile and link shader program
      program.addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
      program.addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
      program.link();

      vbo.create();
      vbo.bind();
      vbo.allocate(wasm_data_ptr, wasm_data_size);
   }

    void paintGL() override {
      glClear(GL_COLOR_BUFFER_BIT);

      program.bind();
      vbo.bind();
        
      // Tell OpenGL how the data is structured (Attribute 0)
      program.enableAttributeArray(0);
      program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));

      // Attribute 1: Color (r, g, b) - next 3 floats, offset by 3 floats
      program.enableAttributeArray(1);
      program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

      glDrawArrays(GL_TRIANGLES, 0, 3);

      vbo.release();
      program.release();
   }

private:
   QOpenGLShaderProgram program;
   QOpenGLBuffer vbo;
   float* wasm_data_ptr = nullptr;
   size_t wasm_data_size = 0;
};

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);

   FT_Library ft;
   // 1. Initialize the library
   if (FT_Init_FreeType(&ft)) {
      qCritical() << "Could not init FreeType Library";
      return -1;
   }

   // 2. Load a font face (replace with your actual font path)
   FT_Face face;
   if (FT_New_Face(ft, "FiraMono-Regular.ttf", 0, &face)) {
      qCritical() << "Failed to load font";
      return -1;
   }

   // 3. Set font size (Width, Height in points; 0 lets it scale dynamically)
   FT_Set_Pixel_Sizes(face, 0, 48);

   // 4. Load the glyph for 'A'
   // FT_Get_Char_Index finds the glyph ID, FT_Load_Char combines this and loads it
   if (FT_Load_Char(face, 'A', FT_LOAD_RENDER)) {
      qCritical() << "Failed to load Glyph";
      return -1;
   }

   // 5. Access the bitmap data
   // face->glyph->bitmap contains the raw pixel buffer
   FT_Bitmap& bitmap = face->glyph->bitmap;

   qDebug() << "Successfully loaded 'A'";
   qDebug() << "Width:" << bitmap.width;
   qDebug() << "Rows:" << bitmap.rows;
   qDebug() << "Pitch (Bytes per row):" << bitmap.pitch;

   // Clean up
   FT_Done_Face(face);
   FT_Done_FreeType(ft);

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

   {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
      db.setDatabaseName("app_data.db");

      if (!db.open()) {
         qCritical() << "Error: Connection with database failed:" << db.lastError().text();
      } else {
         qDebug() << "SQLite Database connected successfully!";

         // Basic table setup example
         QSqlQuery query;
         query.exec("CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY, msg TEXT)");
         query.prepare("INSERT INTO logs (msg) VALUES (:msg)");
         query.bindValue(":msg", QString::fromStdString(message));
         query.exec();
      }
   }

    // --- 1. Get Vertex Data from WASM ---
   wasmtime_extern_t get_v_ptr_item;
   wasmtime_instance_export_get(context, &instance, "get_vertex_ptr", 14, &get_v_ptr_item);
   wasmtime_val_t v_ptr_result[1];
   wasmtime_func_call(context, &get_v_ptr_item.of.func, NULL, 0, v_ptr_result, 1, NULL);
   uint32_t v_offset = v_ptr_result[0].of.i32;

   // Calculate size (3 vertices * 6 floats [x,y,z,r,g,b] * 4 bytes)
   size_t data_size = 3 * 6 * sizeof(float);
   float* triangle_data_ptr = (float*)(memory_base + v_offset);

   //QApplication app(argc, argv);
   MyGLWidget window;

   // Pass the WASM memory pointer to the GL Widget
   window.setVertexData(triangle_data_ptr, data_size);

   window.show();
   return app.exec();
}
