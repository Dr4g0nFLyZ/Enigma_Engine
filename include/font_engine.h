#include <ft2build.h>
#include FT_FREETYPE_H

class FontEngine {
public:
   FontEngine() : ft(nullptr), face(nullptr) {}

   // Destructor ensures cleanup happens automatically
   ~FontEngine() {
      if (face) FT_Done_Face(face);
      if (ft) FT_Done_FreeType(ft);
   }

   bool init(const char* fontPath, int size) {
      if (FT_Init_FreeType(&ft)) {
         qCritical() << "Could not init FreeType Library";
         return false;
      }
      if (FT_New_Face(ft, fontPath, 0, &face)) {
         qCritical() << "Failed to load font:" << fontPath;
         return false;
      }
      FT_Set_Pixel_Sizes(face, 0, size);
      return true;
   }

   // Method to load a specific character and print its info
   bool loadAndPrintGlyph(char c) {
      if (!face) return false;

      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
         qCritical() << "Failed to load Glyph:" << c;
         return false;
      }

      FT_Bitmap& bitmap = face->glyph->bitmap;
      qDebug() << "Successfully loaded glyph:" << c;
      qDebug() << "Width:" << bitmap.width;
      qDebug() << "Rows:" << bitmap.rows;
      qDebug() << "Pitch:" << bitmap.pitch;

      return true;
   }

   // Getter in case you need raw access to the face (e.g. for texture generation)
   FT_Face getFace() const { return face; }

private:
   FT_Library ft;
   FT_Face face;
};
