#include <cstdint>
#include"Core/Settings.h"

struct OverlayPalettes
{
#define SETTING_ARGB(_var, _inistring, _defaultval) \
    unsigned int _var
#include "overlay_palette.def"
#undef SETTING_ARGB
};

struct OverlayMiscSettings
{
#define SETTING_FLOAT(_var, _inistring, _defaultval) \
    float _var;
#include "overlay_misc.def"
#undef SETTING_FLOAT
};