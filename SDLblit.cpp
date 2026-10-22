// This work (SDLblit.cpp, by Cory Bloyd) is free of known copyright restrictions.
// https://creativecommons.org/publicdomain/zero/1.0/
#include <SDL2/SDL.h>
#include <cmath>

inline uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { return (a<<24) | (r << 16) | (g << 8) | (b << 0); }

typedef struct {
  float x, y;
} Point;

void setPixel(uint32_t *pixels, int x, int y, uint32_t color);
void drawLine(uint32_t *pixels, Point p0, Point p1, uint32_t color);
void drawTriangle(uint32_t *pixels, Point p0, Point p1, Point p2, uint32_t color);

void fillTriangle(uint32_t *pixels, Point p0, Point p1, Point p2, uint32_t color);

void sort(float& n1, float& n2, float& n3);
void swap(float& n1, float& n2);
float signed_area(Point p0, Point p1, Point p2);

#define func fillTriangle

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Rect    screenRect = { 0,0,1024,1024 };
    SDL_Window   *window   = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenRect.w, screenRect.h, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture  *texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenRect.w, screenRect.h);
    
    for (int frame = 0; ; ++frame) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
            if (event.type == SDL_QUIT)
                return 0;
    
        int pitch;
        uint32_t *pixels;
        SDL_LockTexture(texture, &screenRect, (void**)&pixels, &pitch);
        uint32_t startTicks = SDL_GetTicks();

        for (int i = 0; i < 1024 * 1024; ++i) pixels[i] = argb(0xFF, 0, 0, 0);

        func(pixels, {100, 120}, {420, 80},  {260, 400}, argb(0xFF, 0xFF, 0x60, 0x60));
        func(pixels, {600, 90},  {940, 200}, {700, 380}, argb(0xFF, 0x60, 0xFF, 0x80));
        func(pixels, {80, 560},  {350, 520}, {200, 900}, argb(0xFF, 0x60, 0xA0, 0xFF));
        func(pixels, {450, 500}, {800, 620}, {520, 960}, argb(0xFF, 0xFF, 0xE0, 0x60));
        func(pixels, {850, 700}, {990, 980}, {700, 900}, argb(0xFF, 0xE0, 0x60, 0xFF));
        func(pixels, {300, 250}, {620, 300}, {480, 520}, argb(0xFF, 0xFF, 0xFF, 0xFF));

        uint32_t endTicks = SDL_GetTicks();
        SDL_UnlockTexture(texture);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, &screenRect, &screenRect);
        SDL_RenderPresent(renderer);

        char title[32];
        SDL_SetWindowTitle(window, SDL_itoa(endTicks - startTicks, title, 10));
    }
}

void setPixel(uint32_t *pixels, int x, int y, uint32_t color) {
  if (x < 0 || x >= 1024 || y < 0 || y >= 1024) return;
  pixels[y*1024 + x] = color;
}
void drawLine(uint32_t *pixels, Point p0, Point p1, uint32_t color) {
  float dy = p1.y - p0.y;
  float dx = p1.x - p0.x;
  float slope;

  Point point = p0;
  Point other = p1;

  if (std::fabs(dx) >= std::fabs(dy)) {
    if (p1.x < p0.x) {
      point = p1;
      other = p0;
    }

    slope = dy / dx;
    //bresehman uses error instead which is more efficient
    //but slightly less intuitive imo
    while (point.x <= other.x) {
      setPixel(pixels, std::round(point.x), std::round(point.y), color);
      point.x += 1;
      point.y += slope;
    }
  }
  else {
    if (p1.y < p0.y) {
      point = p1;
      other = p0;
    }

    slope = dx / dy;

    while (point.y <= other.y) {
      setPixel(pixels, std::round(point.x), std::round(point.y), color);
      point.y += 1;
      point.x += slope;
    }
  }
}

void drawTriangle(uint32_t *pixels, Point p0, Point p1, Point p2, uint32_t color) {
  drawLine(pixels, p0, p1, color);
  drawLine(pixels, p1, p2, color);
  drawLine(pixels, p2, p0, color);
}

void fillTriangle(uint32_t *pixels, Point p0, Point p1, Point p2, uint32_t color) {
  //fill triangle
  //getting bb, i want copies as sorting will 
  //move around the values
  float temp[] = {p0.x, p1.x, p2.x, p0.y, p1.y, p2.y};

  sort(temp[0], temp[1], temp[2]);
  sort(temp[3], temp[4], temp[5]);


  //can be done in parrelel
  for (int i = temp[0]; i <= temp[2]; i++) {
    for (int j = temp[3]; j <= temp[5]; j++) {
      //center of pixel
      Point p = {static_cast<float>(i) + 0.5f, static_cast<float>(j) + 0.5f};

      float a = signed_area(p, p1, p2);
      float b = signed_area(p, p2, p0);
      float c = signed_area(p, p0, p1);
      if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0))
          setPixel(pixels, std::round(p.x), std::round(p.y), color);
    }
  }
}

//bubble sort; is insertion faster?
void sort(float& n1, float& n2, float& n3) {
  if (n1 > n2) swap(n1, n2);
  if (n2 > n3) swap(n2, n3);
  if (n1 > n2) swap(n1, n2);
}

void swap(float& n1, float& n2) {
  float temp = n1;
  n1 = n2;
  n2 = temp;
}

float signed_area(Point p0, Point p1, Point p2) {
  return 0.5 * ((p1.y - p0.y)*(p1.x + p0.x) + 
                (p2.y - p1.y)*(p2.x + p1.x) + 
                (p0.y - p2.y)*(p0.x + p2.x));
}