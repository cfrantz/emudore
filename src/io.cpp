/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdexcept>
#include <sched.h>
#include <gflags/gflags.h>

#include "imgui.h"
#include "src/io.h"
#include "src/gamecontrollerdb.h"

DEFINE_string(ctrlcfg, "", "Path to the SDL gamecontrollerdb.txt file.");
DEFINE_double(scale, 1.0, "Resolution scale factor.\n");

// clas ctor and dtor //////////////////////////////////////////////////////////

IO::IO(size_t cols, size_t rows, double refresh_rate)
    : cols_(cols), rows_(rows), scale_(FLAGS_scale), refresh_rate_(refresh_rate)
{
  SDL_Init(SDL_INIT_VIDEO |
           SDL_INIT_AUDIO |
           SDL_INIT_TIMER |
           SDL_INIT_JOYSTICK |
           SDL_INIT_GAMECONTROLLER);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  /**
   * We create the window double the original pixel size, 
   * the renderer takes care of upscaling 
   */
  window_ = SDL_CreateWindow(
        "emudore",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        cols_ * scale_,
        rows_ * scale_,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  glcontext_ = SDL_GL_CreateContext(window_);

  /* use a single texture and hardware acceleration */
  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  texture_  = SDL_CreateTexture(renderer_,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                cols_,
                                rows_);
  format_ = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
  /**
   * unfortunately, we need to keep a copy of the rendered frame 
   * in our own memory, there does not seem to be a way around 
   * that would allow manipulating pixels straight on the GPU 
   * memory due to how the image is internally stored, etc..
   *
   * The rendered frame gets uploaded to the GPU on every 
   * screen refresh.
   */
  frame_  = new uint32_t[cols_ * rows_]();
  init_color_palette();
  init_keyboard();
  next_key_event_at_ = 0;
  prev_frame_was_at_ = std::chrono::high_resolution_clock::now();

  controller_callback_ = [](SDL_Event* event) {};
  refresh_callback_ = [](SDL_Renderer* r) {};
  keyboard_callback_ = [this](SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
      handle_keydown(event->key.keysym.scancode);
    } else {
      handle_keyup(event->key.keysym.scancode);
    }
  };

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &nesimg_);
  glBindTexture(GL_TEXTURE_2D, nesimg_);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cols_, rows_, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_);

  ImGuiInit();
}

IO::~IO()
{
  delete [] frame_;
  InvalidateDeviceObjects();
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyTexture(texture_);
  SDL_FreeFormat(format_);
  SDL_Quit();
}

// init io devices  ////////////////////////////////////////////////////////////

/**
 * @brief init keyboard state and keymap
 */
void IO::init_keyboard()
{
  /* init keyboard matrix state */
  for(size_t i=0 ; i < sizeof(keyboard_matrix_) ; i++)
  {
    keyboard_matrix_[i] = 0xff;
  }
  /* character to sdl key map */
  charmap_['A']  = {SDL_SCANCODE_A};
  charmap_['B']  = {SDL_SCANCODE_B};
  charmap_['C']  = {SDL_SCANCODE_C};
  charmap_['D']  = {SDL_SCANCODE_D};
  charmap_['E']  = {SDL_SCANCODE_E};
  charmap_['F']  = {SDL_SCANCODE_F};
  charmap_['G']  = {SDL_SCANCODE_G};
  charmap_['H']  = {SDL_SCANCODE_H};
  charmap_['I']  = {SDL_SCANCODE_I};
  charmap_['J']  = {SDL_SCANCODE_J};
  charmap_['K']  = {SDL_SCANCODE_K};
  charmap_['L']  = {SDL_SCANCODE_L};
  charmap_['M']  = {SDL_SCANCODE_M};
  charmap_['N']  = {SDL_SCANCODE_N};
  charmap_['O']  = {SDL_SCANCODE_O};
  charmap_['P']  = {SDL_SCANCODE_P};
  charmap_['Q']  = {SDL_SCANCODE_Q};
  charmap_['R']  = {SDL_SCANCODE_R};
  charmap_['S']  = {SDL_SCANCODE_S};
  charmap_['T']  = {SDL_SCANCODE_T};
  charmap_['U']  = {SDL_SCANCODE_U};
  charmap_['V']  = {SDL_SCANCODE_V};
  charmap_['W']  = {SDL_SCANCODE_W};
  charmap_['X']  = {SDL_SCANCODE_X};
  charmap_['Y']  = {SDL_SCANCODE_Y};
  charmap_['Z']  = {SDL_SCANCODE_Z};
  charmap_['1']  = {SDL_SCANCODE_1};
  charmap_['2']  = {SDL_SCANCODE_2};
  charmap_['3']  = {SDL_SCANCODE_3};
  charmap_['4']  = {SDL_SCANCODE_4};
  charmap_['5']  = {SDL_SCANCODE_5};
  charmap_['6']  = {SDL_SCANCODE_6};
  charmap_['7']  = {SDL_SCANCODE_7};
  charmap_['8']  = {SDL_SCANCODE_8};
  charmap_['9']  = {SDL_SCANCODE_9};
  charmap_['0']  = {SDL_SCANCODE_0};
  charmap_['\n'] = {SDL_SCANCODE_RETURN};
  charmap_[' ']  = {SDL_SCANCODE_SPACE};
  charmap_[',']  = {SDL_SCANCODE_COMMA};
  charmap_['.']  = {SDL_SCANCODE_PERIOD};
  charmap_['/']  = {SDL_SCANCODE_SLASH};
  charmap_[';']  = {SDL_SCANCODE_SEMICOLON};
  charmap_['=']  = {SDL_SCANCODE_EQUALS};
  charmap_['-']  = {SDL_SCANCODE_MINUS};
  charmap_[':']  = {SDL_SCANCODE_BACKSLASH};
  charmap_['+']  = {SDL_SCANCODE_LEFTBRACKET};
  charmap_['*']  = {SDL_SCANCODE_RIGHTBRACKET};
  charmap_['@']  = {SDL_SCANCODE_APOSTROPHE};
  charmap_['(']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_8};
  charmap_[')']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_9};
  charmap_['<']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_COMMA};
  charmap_['>']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_PERIOD};
  charmap_['"']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_2};
  charmap_['$']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_4};
  /* keymap letters */
  keymap_[SDL_SCANCODE_A] = std::make_pair(1,2);
  keymap_[SDL_SCANCODE_B] = std::make_pair(3,4);
  keymap_[SDL_SCANCODE_C] = std::make_pair(2,4);
  keymap_[SDL_SCANCODE_D] = std::make_pair(2,2);
  keymap_[SDL_SCANCODE_E] = std::make_pair(1,6);
  keymap_[SDL_SCANCODE_F] = std::make_pair(2,5);
  keymap_[SDL_SCANCODE_G] = std::make_pair(3,2);
  keymap_[SDL_SCANCODE_H] = std::make_pair(3,5);
  keymap_[SDL_SCANCODE_I] = std::make_pair(4,1);
  keymap_[SDL_SCANCODE_J] = std::make_pair(4,2);
  keymap_[SDL_SCANCODE_K] = std::make_pair(4,5);
  keymap_[SDL_SCANCODE_L] = std::make_pair(5,2);
  keymap_[SDL_SCANCODE_M] = std::make_pair(4,4);
  keymap_[SDL_SCANCODE_N] = std::make_pair(4,7);
  keymap_[SDL_SCANCODE_O] = std::make_pair(4,6);
  keymap_[SDL_SCANCODE_P] = std::make_pair(5,1);
  keymap_[SDL_SCANCODE_Q] = std::make_pair(7,6);
  keymap_[SDL_SCANCODE_R] = std::make_pair(2,1);
  keymap_[SDL_SCANCODE_S] = std::make_pair(1,5);
  keymap_[SDL_SCANCODE_T] = std::make_pair(2,6);
  keymap_[SDL_SCANCODE_U] = std::make_pair(3,6);
  keymap_[SDL_SCANCODE_V] = std::make_pair(3,7);
  keymap_[SDL_SCANCODE_W] = std::make_pair(1,1);
  keymap_[SDL_SCANCODE_X] = std::make_pair(2,7);
  keymap_[SDL_SCANCODE_Y] = std::make_pair(3,1);
  keymap_[SDL_SCANCODE_Z] = std::make_pair(1,4);
  /* keymap numbers */
  keymap_[SDL_SCANCODE_1] = std::make_pair(7,0);
  keymap_[SDL_SCANCODE_2] = std::make_pair(7,3);
  keymap_[SDL_SCANCODE_3] = std::make_pair(1,0);
  keymap_[SDL_SCANCODE_4] = std::make_pair(1,3);
  keymap_[SDL_SCANCODE_5] = std::make_pair(2,0);
  keymap_[SDL_SCANCODE_6] = std::make_pair(2,3);
  keymap_[SDL_SCANCODE_7] = std::make_pair(3,0);
  keymap_[SDL_SCANCODE_8] = std::make_pair(3,3);
  keymap_[SDL_SCANCODE_9] = std::make_pair(4,0);
  keymap_[SDL_SCANCODE_0] = std::make_pair(4,3);
  /* keymap function keys */
  keymap_[SDL_SCANCODE_F1] = std::make_pair(0,4);
  keymap_[SDL_SCANCODE_F3] = std::make_pair(0,4);
  keymap_[SDL_SCANCODE_F5] = std::make_pair(0,4);
  keymap_[SDL_SCANCODE_F7] = std::make_pair(0,4);
  /* keymap: other */
  keymap_[SDL_SCANCODE_RETURN]    = std::make_pair(0,1);
  keymap_[SDL_SCANCODE_SPACE]     = std::make_pair(7,4);
  keymap_[SDL_SCANCODE_LSHIFT]    = std::make_pair(1,7);
  keymap_[SDL_SCANCODE_RSHIFT]    = std::make_pair(6,4);
  keymap_[SDL_SCANCODE_COMMA]     = std::make_pair(5,7);
  keymap_[SDL_SCANCODE_PERIOD]    = std::make_pair(5,4);
  keymap_[SDL_SCANCODE_SLASH]     = std::make_pair(6,7);
  keymap_[SDL_SCANCODE_SEMICOLON] = std::make_pair(6,2);
  keymap_[SDL_SCANCODE_EQUALS]    = std::make_pair(6,5);
  keymap_[SDL_SCANCODE_BACKSPACE] = std::make_pair(0,0);
  keymap_[SDL_SCANCODE_MINUS]     = std::make_pair(5,3);
  /* keymap: these are mapped to other keys */
  keymap_[SDL_SCANCODE_BACKSLASH]    = std::make_pair(5,5); // : 
  keymap_[SDL_SCANCODE_LEFTBRACKET]  = std::make_pair(5,0); // +
  keymap_[SDL_SCANCODE_RIGHTBRACKET] = std::make_pair(6,1); // *
  keymap_[SDL_SCANCODE_APOSTROPHE]   = std::make_pair(5,6); // @
  keymap_[SDL_SCANCODE_LGUI]         = std::make_pair(7,5); // commodore key
}

/** 
 * @brief init c64 color palette 
 */
void IO::init_color_palette()
{

  color_palette[0]   = SDL_MapRGB(format_, 0x00, 0x00, 0x00);
  color_palette[1]   = SDL_MapRGB(format_, 0xff, 0xff, 0xff);
  color_palette[2]   = SDL_MapRGB(format_, 0xab, 0x31, 0x26);
  color_palette[3]   = SDL_MapRGB(format_, 0x66, 0xda, 0xff);
  color_palette[4]   = SDL_MapRGB(format_, 0xbb, 0x3f, 0xb8);
  color_palette[5]   = SDL_MapRGB(format_, 0x55, 0xce, 0x58);
  color_palette[6]   = SDL_MapRGB(format_, 0x1d, 0x0e, 0x97);
  color_palette[7]   = SDL_MapRGB(format_, 0xea, 0xf5, 0x7c);
  color_palette[8]   = SDL_MapRGB(format_, 0xb9, 0x74, 0x18);
  color_palette[9]   = SDL_MapRGB(format_, 0x78, 0x53, 0x00);
  color_palette[10]  = SDL_MapRGB(format_, 0xdd, 0x93, 0x87);
  color_palette[11]  = SDL_MapRGB(format_, 0x5b, 0x5b, 0x5b);
  color_palette[12]  = SDL_MapRGB(format_, 0x8b, 0x8b, 0x8b);
  color_palette[13]  = SDL_MapRGB(format_, 0xb0, 0xf4, 0xac);
  color_palette[14]  = SDL_MapRGB(format_, 0xaa, 0x9d, 0xef);
  color_palette[15]  = SDL_MapRGB(format_, 0xb8, 0xb8, 0xb8);
}

// emulation /////////////////////////////////////////////////////////////////// 

bool IO::emulate()
{
  SDL_Event event;

  while(SDL_PollEvent(&event)) {
      ProcessEvent(&event);
      if (event.type == SDL_QUIT)
          return false;
  }
  return true;

#if 0
  // FIXME(cfrantz): WTF to do here?
  /* process fake keystrokes if any */
  if(!key_event_queue_.empty() && 
     cpu_->cycles() > next_key_event_at_)
  {
    std::pair<kKeyEvent,SDL_Keycode> &ev = key_event_queue_.front();
    key_event_queue_.pop();
    switch(ev.first)
    {
    case kPress:
      handle_keydown(ev.second);
      break;
    case kRelease:
      handle_keyup(ev.second);
      break;
    }
    next_key_event_at_ = cpu_->cycles() + kWait;
  }
#endif
}

// keyboard handling /////////////////////////////////////////////////////////// 

/**
 * @brief emulate keydown
 */
void IO::handle_keydown(SDL_Keycode k)
{
  try
  {
    uint8_t mask = ~(1 << keymap_.at(k).second);
    keyboard_matrix_[keymap_.at(k).first] &= mask;
  }
  catch(const std::out_of_range){}
}

/**
 * @brief emulate keyup
 */
void IO::handle_keyup(SDL_Keycode k)
{
  try
  {
    uint8_t mask = (1 << keymap_.at(k).second);
    keyboard_matrix_[keymap_.at(k).first] |= mask;
  }
  catch(const std::out_of_range){}  
}

/**
 * @brief fake press a key, monkeys love it
 *
 * Characters are added to a queue and processed within 
 * the emulation loop.
 */
void IO::type_character(char c)
{
  try
  {
    for(SDL_Keycode &k: charmap_.at(toupper(c)))
      key_event_queue_.push(std::make_pair(kPress,k));
    for(SDL_Keycode &k: charmap_.at(toupper(c)))
      key_event_queue_.push(std::make_pair(kRelease,k));
  }
  catch(const std::out_of_range){}   
}

// screen handling /////////////////////////////////////////////////////////////

void IO::screen_draw_rect(int x, int y, int n, int color)
{
  for(int i=0; i < n ; i++)
  {
    screen_update_pixel(x+i,y,color);
  }
}
 
void IO::screen_draw_border(int y, int color)
{
  screen_draw_rect(0,y,cols_,color);
}

void IO::screen_blit(uint32_t* data) {
    memcpy(frame_, data, rows_*cols_*sizeof(uint32_t));
}
 
/**
 * @brief refresh screen 
 *
 * Upload the texture to the GPU 
 */
void IO::screen_refresh()
{
  static bool open = true;
  ImGuiIO& io = ImGui::GetIO();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
  glClearColor(clear_color_.x, clear_color_.y, clear_color_.z, clear_color_.w);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, nesimg_);
  glTexSubImage2D(GL_TEXTURE_2D, 0,
                  0, 0, cols_, rows_,
                  GL_RGBA, GL_UNSIGNED_BYTE, frame_);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); glVertex2f(0, 0);
  glTexCoord2f(1, 0); glVertex2f(cols_ * scale_, 0);
  glTexCoord2f(1, 1); glVertex2f(cols_ * scale_, rows_ * scale_);
  glTexCoord2f(0, 1); glVertex2f(0, rows_ * scale_);
  glEnd();

  NewFrame();
  if (ImGui::Begin("MyDebug", &open, ImGuiWindowFlags_MenuBar)) {
    ImGui::SliderFloat("Zoom", &scale_, 0.0f, 6.0f);
    ImGui::ColorEdit3("Clear Color", (float*)&clear_color_);
    ImGui::Text("Fps: %.1f", io.Framerate);
    refresh_callback_(renderer_);
  }
  ImGui::End();
  ImGui::Render();

  SDL_GL_SwapWindow(window_);
  //vsync();
}


void IO::init_audio(int freq, int chan, int bufsz, SDL_AudioFormat fmt,
                    std::function<void(uint8_t*, int)> callback) {
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = freq;
    want.channels = chan;
    want.samples = bufsz;
    want.format = fmt;
    want.callback = IO::AudioCallback;
    want.userdata = (void*)this;
    audio_callback_ = callback;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                              SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    SDL_PauseAudioDevice(dev, 0);
}


/**
 * @brief vsync
 *
 * vsync() is called at the end of every frame, if we are ahead 
 * of time compared to a real C64 (very likely) we sleep for a bit, 
 * this way we avoid running at full speed allowing the host CPU to 
 * take a little nap before getting back to work.
 *
 * This should also help with performance runing on slow computers, 
 * uploading data to the GPU is a relatively slow operation, doing 
 * more fps obviously has a performance impact.
 *
 * Also, and more importantly, by doing this we emulate the actual 
 * speed of the C64 so visual effects do not look accelerated and 
 * games become playable :)
 */
void IO::vsync()
{
  using namespace std::chrono;
  auto t = high_resolution_clock::now() - prev_frame_was_at_;
  duration<double> rr(refresh_rate_);
  /**
   * Microsoft's chrono is buggy and does not properly handle 
   * doubles, we need to recast to milliseconds.
   */
  auto ttw = duration_cast<milliseconds>(rr - t);
  std::this_thread::sleep_for(ttw);
  prev_frame_was_at_ = std::chrono::high_resolution_clock::now();
}

void IO::AudioCallback(void* userdata, uint8_t* stream, int len) {
    IO* io = (IO*)userdata;
    io->audio_callback_(stream, len);
}

uint64_t IO::clock_micros() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * 1000000 + tp.tv_nsec / 1000;
}

void IO::init_controllers(std::function<void(SDL_Event*)> callback) {
    if (!FLAGS_ctrlcfg.empty()) {
        // User supplied gamecontrollerdb.txt
        SDL_GameControllerAddMappingsFromFile(FLAGS_ctrlcfg.c_str());
    } else {
        // Built-in gamecontrollerdb.txt
        SDL_RWops* f = SDL_RWFromConstMem(kGameControllerDB,
                                          kGameControllerDB_len);
        SDL_GameControllerAddMappingsFromRW(f, 1);
    }

    int controllers = 0;
    char guid[64];
    for(int i=0; i<SDL_NumJoysticks(); ++i) {
        const char *name, *desc;

        SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i),
                                  guid, sizeof(guid));
        if (SDL_IsGameController(i)) {
            controllers++;
            name = SDL_GameControllerNameForIndex(i);
            desc = "Controller";
        } else {
            name = SDL_JoystickNameForIndex(i);
            desc = "Joystick";
        }
        printf("%s %d: %s (guid: %s)\n", desc, i,
                name ? name : "Unknown", guid);
    }
    for(int i=0; i<controllers; i++) {
        printf("Opening controller %d\n", i);
        SDL_GameControllerOpen(i);
    }
    controller_callback_ = callback;
}

void IO::yield() {
    sched_yield();
}


void IO::ImGuiInit() {
    ImGuiIO& io = ImGui::GetIO();

    mousewheel_ = 0;
    fonttexture_ = 0;
    time_ = 0;
    mousebutton_[0] = false;
    mousebutton_[1] = false;
    mousebutton_[2] = false;

    clear_color_ = ImColor(114, 144, 154);

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.RenderDrawListsFn = RenderDrawLists;
    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;

    //CreateDeviceObjects();
}

void IO::CreateDeviceObjects() {
    static int once = 0;
    if (once)
        return;
    once++;
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &fonttexture_);
    glBindTexture(GL_TEXTURE_2D, fonttexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
            GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)fonttexture_;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
}

void IO::InvalidateDeviceObjects() {
    if (fonttexture_) {
        glDeleteTextures(1, &fonttexture_);
        ImGui::GetIO().Fonts->TexID = 0;
        fonttexture_ = 0;
    }
}

void IO::SetClipboardText(const char *text) {
    SDL_SetClipboardText(text);
}

const char* IO::GetClipboardText() {
    return SDL_GetClipboardText();
}

void IO::RenderDrawLists(ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}


void IO::NewFrame() {
    CreateDeviceObjects();
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window_, &w, &h);
    SDL_GL_GetDrawableSize(window_, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    Uint32	time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io.DeltaTime = time_ > 0.0 ? (float)(current_time - time_) : (float)(1.0f/60.0f);
    time_ = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    int mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    else
        io.MousePos = ImVec2(-1,-1);

    io.MouseDown[0] = mousebutton_[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[1] = mousebutton_[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = mousebutton_[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    mousebutton_[0] = mousebutton_[1] = mousebutton_[2] = false;

    io.MouseWheel = mousewheel_;;
    mousewheel_ = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

    // Start the frame
    ImGui::NewFrame();
}



bool IO::ProcessEvent(SDL_Event* event) {
    ImGuiIO& io = ImGui::GetIO();
    int key;

    switch(event->type) {
    case SDL_MOUSEWHEEL:
        if (event->wheel.y > 0)
            mousewheel_ = 1;
        if (event->wheel.y < 0)
            mousewheel_ = -1;
        return true;
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT) mousebutton_[0] = true;
        if (event->button.button == SDL_BUTTON_RIGHT) mousebutton_[1] = true;
        if (event->button.button == SDL_BUTTON_MIDDLE) mousebutton_[2] = true;
        return true;
    case SDL_TEXTINPUT:
        io.AddInputCharactersUTF8(event->text.text);
        return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
        io.KeysDown[key] = (event->type == SDL_KEYDOWN);
        io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl =  ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt =   ((SDL_GetModState() & KMOD_ALT) != 0);
        io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
        keyboard_callback_(event);
        return true;
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
        controller_callback_(event);
        return true;

    }
    return false;
}
