#ifndef EMUDORE_SRC_NES_GFX_H
#define EMUDORE_SRC_NES_GFX_H
#include <string.h>
#include <stdint.h>
#include <SDL2/SDL2_framerate.h>
#include <SDL2/SDL2_gfxPrimitives.h>

namespace sdlutil {
using std::string;

class GFX {
  private:
    SDL_Renderer* renderer_;

  public:
    // Just put this here for convenience.
    class FPSManager {
      public:
        FPSManager() {
            SDL_initFramerate(&mgr_);
        }

        int SetRate(int rate) {
            return SDL_setFramerate(&mgr_, rate);
        }
        int GetRate(int rate) {
            return SDL_getFramerate(&mgr_);
        }
        int Count() {
            return SDL_getFramecount(&mgr_);
        }
        int Delay() {
            return SDL_framerateDelay(&mgr_);
        }
      private:
        FPSmanager mgr_;
    };

    GFX() : renderer_(nullptr) {}
    GFX(SDL_Renderer* r) : renderer_(r) {}
    ~GFX() {}

    // Oh the horror, a global singleton (if you want one).
    static GFX* Global() {
        static GFX* instance = new GFX();
        return instance;
    }

    inline void set_renderer(SDL_Renderer* r) {
        renderer_ = r;
    }
    SDL_Renderer* renderer() {
        return renderer_;
    }

    // static wrappers for all the GFX functions for when you have
    // an SDL_Renderer handy.
    static inline int Pixel(SDL_Renderer* renderer, int16_t x,
                            int16_t y, uint32_t color) {
        return pixelColor(renderer, x, y, color);
    }
    static inline int Pixel(SDL_Renderer* renderer, int16_t x,
                            int16_t y, uint8_t r, uint8_t g,
                            uint8_t b, uint8_t a) {
        return pixelRGBA(renderer, x, y, r, g, b, a);
    }
    static inline int HLine(SDL_Renderer* renderer, int16_t x1,
                            int16_t x2, int16_t y, uint32_t color) {
        return hlineColor(renderer, x1, x2, y, color);
    }
    static inline int HLine(SDL_Renderer* renderer, int16_t x1,
                            int16_t x2, int16_t y, uint8_t r,
                            uint8_t g, uint8_t b, uint8_t a) {
        return hlineRGBA(renderer, x1, x2, y, r, g, b, a);
    }
    static inline int VLine(SDL_Renderer* renderer, int16_t x,
                            int16_t y1, int16_t y2, uint32_t color) {
        return vlineColor(renderer, x, y1, y2, color);
    }
    static inline int VLine(SDL_Renderer* renderer, int16_t x,
                            int16_t y1, int16_t y2, uint8_t r,
                            uint8_t g, uint8_t b, uint8_t a) {
        return vlineRGBA(renderer, x, y1, y2, r, g, b, a);
    }
    static inline int Rectangle(SDL_Renderer* renderer, int16_t x1,
                                int16_t y1, int16_t x2, int16_t y2,
                                uint32_t color) {
        return rectangleColor(renderer, x1, y1, x2, y2, color);
    }
    static inline int RoundedRectangle(SDL_Renderer* renderer,
                                       int16_t x1, int16_t y1,
                                       int16_t x2, int16_t y2,
                                       int16_t rad, uint32_t color) {
        return roundedRectangleColor(renderer, x1, y1, x2, y2, rad,
                                     color);
    }
    static inline int Box(SDL_Renderer* renderer, int16_t x1,
                          int16_t y1, int16_t x2, int16_t y2,
                          uint32_t color) {
        return boxColor(renderer, x1, y1, x2, y2, color);
    }
    static inline int RoundedBox(SDL_Renderer* renderer, int16_t x1,
                                 int16_t y1, int16_t x2, int16_t y2,
                                 int16_t rad, uint32_t color) {
        return roundedBoxColor(renderer, x1, y1, x2, y2, rad, color);
    }
    static inline int Line(SDL_Renderer* renderer, int16_t x1,
                           int16_t y1, int16_t x2, int16_t y2,
                           uint32_t color) {
        return lineColor(renderer, x1, y1, x2, y2, color);
    }
    static inline int AALine(SDL_Renderer* renderer, int16_t x1,
                             int16_t y1, int16_t x2, int16_t y2,
                             uint32_t color) {
        return aalineColor(renderer, x1, y1, x2, y2, color);
    }
    static inline int Circle(SDL_Renderer* renderer, int16_t x,
                             int16_t y, int16_t rad, uint32_t color) {
        return circleColor(renderer, x, y, rad, color);
    }
    static inline int Circle(SDL_Renderer* renderer, int16_t x,
                             int16_t y, int16_t rad, uint8_t r,
                             uint8_t g, uint8_t b, uint8_t a) {
        return circleRGBA(renderer, x, y, rad, r, g, b, a);
    }
    static inline int Arc(SDL_Renderer* renderer, int16_t x,
                          int16_t y, int16_t rad, int16_t start,
                          int16_t end, uint32_t color) {
        return arcColor(renderer, x, y, rad, start, end, color);
    }
    static inline int AACircle(SDL_Renderer* renderer, int16_t x,
                               int16_t y, int16_t rad,
                               uint32_t color) {
        return aacircleColor(renderer, x, y, rad, color);
    }
    static inline int FilledCircle(SDL_Renderer* renderer, int16_t x,
                                   int16_t y, int16_t r,
                                   uint32_t color) {
        return filledCircleColor(renderer, x, y, r, color);
    }
    static inline int Ellipse(SDL_Renderer* renderer, int16_t x,
                              int16_t y, int16_t rx, int16_t ry,
                              uint32_t color) {
        return ellipseColor(renderer, x, y, rx, ry, color);
    }
    static inline int AAEllipse(SDL_Renderer* renderer, int16_t x,
                                int16_t y, int16_t rx, int16_t ry,
                                uint32_t color) {
        return aaellipseColor(renderer, x, y, rx, ry, color);
    }
    static inline int FilledEllipse(SDL_Renderer* renderer, int16_t x,
                                    int16_t y, int16_t rx, int16_t ry,
                                    uint32_t color) {
        return filledEllipseColor(renderer, x, y, rx, ry, color);
    }
    static inline int Trigon(SDL_Renderer* renderer, int16_t x1,
                             int16_t y1, int16_t x2, int16_t y2,
                             int16_t x3, int16_t y3, uint32_t color) {
        return trigonColor(renderer, x1, y1, x2, y2, x3, y3, color);
    }
    static inline int AATrigon(SDL_Renderer* renderer, int16_t x1,
                               int16_t y1, int16_t x2, int16_t y2,
                               int16_t x3, int16_t y3,
                               uint32_t color) {
        return aatrigonColor(renderer, x1, y1, x2, y2, x3, y3, color);
    }
    static inline int FilledTrigon(SDL_Renderer* renderer, int16_t x1,
                                   int16_t y1, int16_t x2, int16_t y2,
                                   int16_t x3, int16_t y3,
                                   uint32_t color) {
        return filledTrigonColor(renderer, x1, y1, x2, y2, x3, y3,
                                 color);
    }
    static inline int Polygon(SDL_Renderer* renderer,
                              const int16_t* vx, const int16_t* vy,
                              int n, uint32_t color) {
        return polygonColor(renderer, vx, vy, n, color);
    }
    static inline int AAPolygon(SDL_Renderer* renderer,
                                const int16_t* vx, const int16_t* vy,
                                int n, uint32_t color) {
        return aapolygonColor(renderer, vx, vy, n, color);
    }
    static inline int FilledPolygon(SDL_Renderer* renderer,
                                    const int16_t* vx,
                                    const int16_t* vy, int n,
                                    uint32_t color) {
        return filledPolygonColor(renderer, vx, vy, n, color);
    }
    static inline int TexturedPolygon(SDL_Renderer* renderer,
                                      const int16_t* vx,
                                      const int16_t* vy, int n,
                                      SDL_Surface* texture,
                                      int texture_dx, int texture_dy) {
        return texturedPolygon(renderer, vx, vy, n, texture,
                               texture_dx, texture_dy);
    }
    static inline int Bezier(SDL_Renderer* renderer,
                             const int16_t* vx, const int16_t* vy,
                             int n, int s, uint32_t color) {
        return bezierColor(renderer, vx, vy, n, s, color);
    }
    static inline void GfxPrimitivesSetFont(const void* fontdata,
                                            uint32_t cw, uint32_t ch) {
        return gfxPrimitivesSetFont(fontdata, cw, ch);
    }
    static inline void GfxPrimitivesSetFontRotation(uint32_t rotation) {
        return gfxPrimitivesSetFontRotation(rotation);
    }
    static inline int Character(SDL_Renderer* renderer, int16_t x,
                                int16_t y, char c, uint32_t color) {
        return characterColor(renderer, x, y, c, color);
    }
    static inline int Character(SDL_Renderer* renderer, int16_t x,
                                int16_t y, char c, uint8_t r,
                                uint8_t g, uint8_t b, uint8_t a) {
        return characterRGBA(renderer, x, y, c, r, g, b, a);
    }
    static inline int String(SDL_Renderer* renderer, int16_t x,
                             int16_t y, const string& s,
                             uint32_t color) {
        return stringColor(renderer, x, y, s.c_str(), color);
    }
    static inline int String(SDL_Renderer* renderer, int16_t x,
                             int16_t y, const string& s, uint8_t r,
                             uint8_t g, uint8_t b, uint8_t a) {
        return stringRGBA(renderer, x, y, s.c_str(), r, g, b, a);
    }

    // Instance wrappers for all the GFX functions.  Make sure you set the
    // renderer in your instance.
    inline int Pixel(int16_t x, int16_t y, uint32_t color) {
        return pixelColor(renderer_, x, y, color);
    }
    inline int Pixel(int16_t x, int16_t y, uint8_t r, uint8_t g,
                     uint8_t b, uint8_t a) {
        return pixelRGBA(renderer_, x, y, r, g, b, a);
    }
    inline int HLine(int16_t x1, int16_t x2, int16_t y, uint32_t color) {
        return hlineColor(renderer_, x1, x2, y, color);
    }
    inline int HLine(int16_t x1, int16_t x2, int16_t y, uint8_t r,
                     uint8_t g, uint8_t b, uint8_t a) {
        return hlineRGBA(renderer_, x1, x2, y, r, g, b, a);
    }
    inline int VLine(int16_t x, int16_t y1, int16_t y2, uint32_t color) {
        return vlineColor(renderer_, x, y1, y2, color);
    }
    inline int VLine(int16_t x, int16_t y1, int16_t y2, uint8_t r,
                     uint8_t g, uint8_t b, uint8_t a) {
        return vlineRGBA(renderer_, x, y1, y2, r, g, b, a);
    }
    inline int Rectangle(int16_t x1, int16_t y1, int16_t x2,
                         int16_t y2, uint32_t color) {
        return rectangleColor(renderer_, x1, y1, x2, y2, color);
    }
    inline int RoundedRectangle(int16_t x1, int16_t y1, int16_t x2,
                                int16_t y2, int16_t rad,
                                uint32_t color) {
        return roundedRectangleColor(renderer_, x1, y1, x2, y2, rad,
                                     color);
    }
    inline int Box(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                   uint32_t color) {
        return boxColor(renderer_, x1, y1, x2, y2, color);
    }
    inline int RoundedBox(int16_t x1, int16_t y1, int16_t x2,
                          int16_t y2, int16_t rad, uint32_t color) {
        return roundedBoxColor(renderer_, x1, y1, x2, y2, rad, color);
    }
    inline int Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                    uint32_t color) {
        return lineColor(renderer_, x1, y1, x2, y2, color);
    }
    inline int AALine(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                      uint32_t color) {
        return aalineColor(renderer_, x1, y1, x2, y2, color);
    }
    inline int Circle(int16_t x, int16_t y, int16_t rad,
                      uint32_t color) {
        return circleColor(renderer_, x, y, rad, color);
    }
    inline int Circle(int16_t x, int16_t y, int16_t rad, uint8_t r,
                      uint8_t g, uint8_t b, uint8_t a) {
        return circleRGBA(renderer_, x, y, rad, r, g, b, a);
    }
    inline int Arc(int16_t x, int16_t y, int16_t rad, int16_t start,
                   int16_t end, uint32_t color) {
        return arcColor(renderer_, x, y, rad, start, end, color);
    }
    inline int AACircle(int16_t x, int16_t y, int16_t rad,
                        uint32_t color) {
        return aacircleColor(renderer_, x, y, rad, color);
    }
    inline int FilledCircle(int16_t x, int16_t y, int16_t r,
                            uint32_t color) {
        return filledCircleColor(renderer_, x, y, r, color);
    }
    inline int Ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry,
                       uint32_t color) {
        return ellipseColor(renderer_, x, y, rx, ry, color);
    }
    inline int AAEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry,
                         uint32_t color) {
        return aaellipseColor(renderer_, x, y, rx, ry, color);
    }
    inline int FilledEllipse(int16_t x, int16_t y, int16_t rx,
                             int16_t ry, uint32_t color) {
        return filledEllipseColor(renderer_, x, y, rx, ry, color);
    }
    inline int Trigon(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                      int16_t x3, int16_t y3, uint32_t color) {
        return trigonColor(renderer_, x1, y1, x2, y2, x3, y3, color);
    }
    inline int AATrigon(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                        int16_t x3, int16_t y3, uint32_t color) {
        return aatrigonColor(renderer_, x1, y1, x2, y2, x3, y3, color);
    }
    inline int FilledTrigon(int16_t x1, int16_t y1, int16_t x2,
                            int16_t y2, int16_t x3, int16_t y3,
                            uint32_t color) {
        return filledTrigonColor(renderer_, x1, y1, x2, y2, x3, y3,
                                 color);
    }
    inline int Polygon(const int16_t* vx, const int16_t* vy, int n,
                       uint32_t color) {
        return polygonColor(renderer_, vx, vy, n, color);
    }
    inline int AAPolygon(const int16_t* vx, const int16_t* vy, int n,
                         uint32_t color) {
        return aapolygonColor(renderer_, vx, vy, n, color);
    }
    inline int FilledPolygon(const int16_t* vx, const int16_t* vy,
                             int n, uint32_t color) {
        return filledPolygonColor(renderer_, vx, vy, n, color);
    }
    inline int TexturedPolygon(const int16_t* vx, const int16_t* vy,
                               int n, SDL_Surface* texture,
                               int texture_dx, int texture_dy) {
        return texturedPolygon(renderer_, vx, vy, n, texture,
                               texture_dx, texture_dy);
    }
    inline int Bezier(const int16_t* vx, const int16_t* vy, int n,
                      int s, uint32_t color) {
        return bezierColor(renderer_, vx, vy, n, s, color);
    }
    inline int Character(int16_t x, int16_t y, char c, uint32_t color) {
        return characterColor(renderer_, x, y, c, color);
    }
    inline int Character(int16_t x, int16_t y, char c, uint8_t r,
                         uint8_t g, uint8_t b, uint8_t a) {
        return characterRGBA(renderer_, x, y, c, r, g, b, a);
    }
    inline int String(int16_t x, int16_t y, const string& s,
                      uint32_t color) {
        return stringColor(renderer_, x, y, s.c_str(), color);
    }
    inline int String(int16_t x, int16_t y, const string& s, uint8_t r,
                      uint8_t g, uint8_t b, uint8_t a) {
        return stringRGBA(renderer_, x, y, s.c_str(), r, g, b, a);
    }
};

} // namespace

#endif // EMUDORE_SRC_NES_GFX_H
