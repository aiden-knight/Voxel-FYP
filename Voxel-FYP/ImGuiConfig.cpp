#include "ImGuiConfig.h"

ImGuiConfig* ImGuiConfig::GetInstance()
{
    static ImGuiConfig config{};

    return &config;
}
