#ifndef EMUDORE_SRC_NES_DEBUG_CONSOLE_H
#define EMUDORE_SRC_NES_DEBUG_CONSOLE_H
#include <map>
#include <functional>
#include "imgui.h"

class DebugConsole {
  public:
    DebugConsole();
    ~DebugConsole();
    void RegisterCommand(const char* cmd, const char* shorthelp,
                         std::function<void(int argc, char **argv)> fn);

    void ClearLog();
    void AddLog(const char* fmt, ...) IM_PRINTFARGS(2);
    void Draw(const char* title, bool* p_open);
    void ExecCommand(const char* command_line);
  private:
    int TextEditCallback(ImGuiTextEditCallbackData* data);

    static int TextEditCallbackStub(ImGuiTextEditCallbackData* data);

    char inputbuf_[256];
    ImVector<char*> items_;
    bool scroll_to_bottom_;
    ImVector<char*> history_;
    // -1: new line, 0..history_.Size-1 browsing history.
    int history_pos_;
    std::map<const char*, const char*> shorthelp_;
    std::map<const char*, std::function<void(int argc, char **argv)>> commands_;
};

#endif // EMUDORE_SRC_NES_DEBUG_CONSOLE_H
