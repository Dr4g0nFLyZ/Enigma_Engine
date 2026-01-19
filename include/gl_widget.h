#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

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
