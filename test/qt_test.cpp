#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[]) {
   // 1. Create the application object.
   QApplication app(argc, argv);

   // 2. Create a QPushButton widget.
   QPushButton button("Hello World!");

   // 3. Set a fixed size for the button (optional, but good for demo).
   button.setFixedSize(200, 50);

   // 4. Show the widget on the screen.
   button.show();

   // 5. Start the event loop.
   return app.exec();
}
