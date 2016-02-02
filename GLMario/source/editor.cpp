#include "Editor.h"
#include "renderer.h"
#include "input.h"
#include "console.h"
#include "mathops.h"
#include "utility.h"

static Rectf ScaleRect(Rect rect, Vec2i resolution)
{
    Rectf result;
    result.x = (float)rect.x / (float)resolution.x;
    result.y = (float)rect.y / (float)resolution.y;
    result.w = (float)rect.w / (float)resolution.x;
    result.h = (float)rect.h / (float)resolution.y;
    return result;
}

static Vec2 ScalePoint(Vec2i point, Vec2i resolution)
{
    Vec2 result;
    result.x = (float)point.x / (float)resolution.x;
    result.y = (float)point.y / (float)resolution.y;
    return result;
}

static Rectf ScaleRectToParent(Rectf rect, Rectf parent)
{
    Rectf result;

    result.x = parent.x + rect.x * parent.w;
    result.y = parent.y + rect.y * parent.h;
    result.w = rect.w * parent.w;
    result.h = rect.h * parent.h;

    return result;
}

void SetSize(UIWindow* ui, Rectf ui_pos, float title_bar_height)
{
    ui->window.h = ui_pos.h - title_bar_height;
    ui->window.w = ui_pos.w;
    ui->window.x = ui_pos.x;
    ui->window.y = ui_pos.bot;

    ui->title_bar.h = title_bar_height;
    ui->title_bar.w = ui_pos.w;
    ui->title_bar.x = ui_pos.x;
    ui->title_bar.y = ui->window.Top();
}

void SetTitle(UIWindow* ui, char* title)
{
    size_t slen = strlen(title);
    StrPrintf(ui->title, sizeof(ui->title), "%s", title);
}

void UpdateUIWindow(GameState* game_state, UIWindow* ui)
{
    Renderer* ren = game_state->renderer;

    Vec2i screen_res = ren->frame_resolution;
    Vec2i mouse_raw_pos = MousePosition(game_state->input);

    Vec2 mouse_pos = ScalePoint(mouse_raw_pos, screen_res);
    Vec2 mouse_delta = ScalePoint(MouseDelta(game_state->input), screen_res);

    mouse_pos.y = mouse_pos.y;
    mouse_delta.y = mouse_delta.y;

    bool title_bar_contains = false;
    bool mouse_down = false;

    if (Contains(ui->title_bar, mouse_pos))
    {
        title_bar_contains = true;
        if (OnDown(game_state->input, MouseButton_LEFT))
        {
            mouse_down = true;
            ui->is_dragged = true;
        }
    }

    if (ui->is_dragged)
    {
        if (IsDown(game_state->input, MouseButton_LEFT))
        {
            ui->window.x += mouse_delta.x;
            ui->window.y += mouse_delta.y;
        }
        else
        {
            ui->is_dragged = false;
        }
    }

    Rectf full_window = ui->window;
    full_window.h += ui->title_bar.h;
    bool mouse_inside_ui = Contains(full_window, mouse_pos);
    float alpha = mouse_inside_ui ? 0.9f : 0.4f;

    ui->title_bar.x = ui->window.x;
    ui->title_bar.y = ui->window.Top();

    PrimitiveDrawParams ui_params = {};
    ui_params.line_draw_flags = Draw_ScreenSpace;
    ui_params.line_draw_flags |= PrimitiveDraw_Filled;
    ui_params.draw_layer.layer = DrawLayer_UI;
    ui_params.draw_layer.sub_layer = 0;
    DrawRect(ren, ui->title_bar, MakeColor(cg_gray1, alpha), 0, ui_params);

    Vec4 text_color = MakeColor(cg_green, 1.0f);
    Vec2 text_size = { 0.0075f, 0.015f };

    uint32 hor_alignments[] =
    {
        String_HorAlignLeft,
        String_HorAlignCenter,
        String_HorAlignRight,
    };

    uint32 ver_alignments[] =
    {
        String_VerAlignTop,
        String_VerAlignCenter,
        String_VerAlignBottom,
    };

    static uint32 hor_index = 0;
    static uint32 ver_index = 0;

    uint32 alignment = ver_alignments[ver_index] | hor_alignments[hor_index];

    ui_params.draw_layer.sub_layer++;
    DrawRect(ren, ui->window, MakeColor(cg_gray2, alpha), 0, ui_params);
    ui_params.draw_layer.sub_layer++;
    DrawStringInRect(ren, ui->title, ui->title_bar, text_size * 2.f, text_color, alignment, ui_params.draw_layer);

    // Keep all values in percentage of parent rect coordinates?
    float y_pad = 0.05f;
    float x_pad = 0.1f;
    float button_height = 0.25f;

    // Buttons
    Rectf tilemap_erase_button = { x_pad, 1.0f - y_pad - button_height, 1.0f - x_pad * 2.f, button_height };
    Rectf element_rect = tilemap_erase_button;

    //TODO: Center text in rect
    //      Simplify String drawing

    char silly_string[1024] = "0123456789ABCDEFGHIJKLMN This is a long string to find out how the text parsing is working! \nit \nis quite silly\n\nToo silly";
    ui_params.draw_layer.sub_layer++;
    DrawStringInRect(ren, silly_string, ui->window, text_size, text_color, alignment, ui_params.draw_layer);



    ui_params.draw_layer.sub_layer++;
    Rectf scaled_rect = ScaleRectToParent(tilemap_erase_button, ui->window);
    DrawRect(ren, scaled_rect, MakeColor(cg_dark_green, alpha), 0, ui_params);
    if (Contains(scaled_rect, mouse_pos))
    {
        ui_params.draw_layer.sub_layer++;
        char button_text[64] = "Change Vertical";
        DrawStringInRect(ren, button_text, scaled_rect, text_size, MakeColor(cg_black, 1.0f), alignment, ui_params.draw_layer);

        if (OnDown(game_state->input, MouseButton_LEFT))
        {
            ui->hot_item = 1;
        }
        else if (ui->hot_item == 1 && OnUp(game_state->input, MouseButton_LEFT))
        {
            ++ver_index;
            ver_index %= ArrayCount(ver_alignments);
            ui->hot_item = 0;
        }
    }

    ui_params.draw_layer.sub_layer++;
    tilemap_erase_button.y -= button_height * 1.1f;
    scaled_rect = ScaleRectToParent(tilemap_erase_button, ui->window);
    DrawRect(ren, scaled_rect, MakeColor(cg_dark_green, alpha), 0, ui_params);
    if (Contains(scaled_rect, mouse_pos))
    {
        ui_params.draw_layer.sub_layer++;
        char button_text[64] = "Horizontal";
        DrawStringInRect(ren, button_text, scaled_rect, text_size, MakeColor(cg_black, 1.0f), alignment, ui_params.draw_layer);

        if (OnDown(game_state->input, MouseButton_LEFT))
        {
            ui->hot_item = 2;
        }
        else if (ui->hot_item == 2 && OnUp(game_state->input, MouseButton_LEFT))
        {
            ++hor_index;
            hor_index %= ArrayCount(hor_alignments);
            ui->hot_item = 0;
        }
    }


    element_rect.y -= y_pad;

#if 0
    ui_params.draw_layer.sub_layer++;
    DrawRect(ren, ScaleRectToParent(tilemap_erase_button, ui->window), MakeColor(cg_dark_green, alpha), 0, ui_params);
    DrawString(ren, ui->title, (uint32)strlen(ui->title), ui->title_bar.x, ui->title_bar.y, &color, 1, ui_params.draw_layer);
#endif
}
