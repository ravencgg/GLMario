#pragma once

#include "mathops.h"
#include "types.h"
#include "game_types.h"

struct Renderer;

typedef uint32 UiItemId;

enum EditorItemType
{
    EditorItem_Button,
};

struct EditorItem
{
    UiItemId id;
    char name[32];
    bool same_line;
    Rectf rect;

    Vec3 color;
    Vec3 hot_color;

    Vec3 text_color;
};


struct UIWindow
{
    bool is_dragged;
    Rectf title_bar;
    Rectf window;

    char title[32];
    UiItemId hot_item;
    EditorItem items[16];
};

void CreateEditorUI(UIWindow* ui);

void UpdateUIWindow(GameState*, UIWindow* ui);
void SetSize(UIWindow*, Rectf, float);

void SetTitle(UIWindow*, char*);

