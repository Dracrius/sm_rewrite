#include "types.h"

#include "config.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../third_party/nuklear/nuklear.h"
#include "../third_party/nuklear/nuklear_sdl_renderer.h"

struct nk_context* ctx;
struct nk_colorf bg;

struct nk_font_atlas* default_atlas;
struct nk_font_config default_config;
struct nk_font* default_font;

struct nk_font_atlas* small_atlas;
struct nk_font* small_font;

struct nk_font_atlas* large_atlas;
struct nk_font* large_font;

enum nkMenuOptions
{
    menuRoot,
    menuGeneral,
    menuGraphics,
    menuSound,
    menuFeatures,
    menuSaveLoad,
    menuQuit
};

bool ammoRechargeStation;
bool shinesparkControl;
bool shinesparkHealth;
bool chainSpark;
bool powerBombReveal;
bool instantPickups;
int saveSlotSelected = 0;

enum nkMenuOptions currentMenu = menuRoot;

static void nkFontSetup()
{
    default_config = nk_font_config(0);
    default_config.pixel_snap = true;
    default_config.oversample_h = 1;

    nk_sdl_font_stash_begin(&default_atlas);
    default_font = nk_font_atlas_add_default(default_atlas, 13, &default_config);
    nk_sdl_font_stash_end();

    nk_sdl_font_stash_begin(&small_atlas);
    small_font = nk_font_atlas_add_from_file(small_atlas, "assets/fonts/sm-snes.ttf", 7, &default_config);
    nk_sdl_font_stash_end();

    nk_sdl_font_stash_begin(&large_atlas);
    large_font = nk_font_atlas_add_from_file(large_atlas, "assets/fonts/sm-large-alt.ttf", 14, &default_config);
    nk_sdl_font_stash_end();
}

static void nkMenuInit(SDL_Window* g_window, SDL_Renderer* g_renderer)
{
    ctx = nk_sdl_init(g_window, g_renderer);

    nkFontSetup();
    nk_style_set_font(ctx, &small_font->handle);
}

static bool nkRootMenu()
{
    bool resume = false;
    if (nk_begin(ctx, "Paused_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_style_set_font(ctx, &small_font->handle);
        if (nk_button_label(ctx, "Resume")) {
            resume = true;
        };
        if (nk_button_label(ctx, "General")) {
            currentMenu = menuGeneral;
        };
        if (nk_button_label(ctx, "Graphics")) {
            currentMenu = menuGraphics;
        };
        if (nk_button_label(ctx, "Sound")) {
            currentMenu = menuSound;
        };
        if (nk_button_label(ctx, "Features")) {
            currentMenu = menuFeatures;
        };
        if (nk_button_label(ctx, "Save + Load")) {
            currentMenu = menuSaveLoad;
        };
        if (nk_button_label(ctx, "Reset")) {
            RtlReset(1);
            resume = true;
        };
        if (nk_button_label(ctx, "Quit")) {
            currentMenu = menuQuit;
        };
    }
    nk_end(ctx);

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, "Paused", nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "Paused", NK_TEXT_CENTERED);
    }
    nk_end(ctx);

    return resume;
}

static void nkGeneralMenu()
{
    int autosave = g_config.autosave;
    int disableFrameDelay = g_config.disable_frame_delay;

    if (nk_begin(ctx, "General_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        bool madeSelection = false;
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_style_set_font(ctx, &small_font->handle);

        if (nk_checkbox_label(ctx, "Autosave", &autosave)) { g_config.autosave = !g_config.autosave; }
        nk_widget_disable_begin(ctx);
        if (nk_checkbox_label(ctx, "DisableFrameDelay", &disableFrameDelay)) { g_config.disable_frame_delay = !g_config.disable_frame_delay; }
        nk_widget_disable_end(ctx);

        if (nk_button_label(ctx, "Back")) {
            currentMenu = menuRoot;
        };
    }
    nk_end(ctx);

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, "General", nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "General", NK_TEXT_CENTERED);
    }
    nk_end(ctx);
}

static void nkGraphicsMenu()
{
    int fullscreen = g_config.fullscreen;
    int window_scale = g_config.window_scale;
    int extend_y = g_config.extend_y;
    int new_renderer = g_config.new_renderer;
    int enhanced_mode7 = g_config.enhanced_mode7;
    int ignore_aspect_ratio = g_config.ignore_aspect_ratio;
    int no_sprite_limits = g_config.no_sprite_limits;
    int linear_filtering = g_config.linear_filtering;

    if (nk_begin(ctx, "Graphics_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_style_set_font(ctx, &small_font->handle);

        nk_widget_disable_begin(ctx);
        nk_label(ctx, "Fullscreen", NK_TEXT_LEFT);
        static const char* fullscreenOptions[] = { "Windowed","Fullscreen","Fullscreen Windowed" };
        if (nk_combo_begin_label(ctx, fullscreenOptions[fullscreen], nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 15, 1);
            for (int i = 0; i < 3; ++i)
                if (nk_combo_item_label(ctx, fullscreenOptions[i], NK_TEXT_LEFT))
                {
                    g_config.fullscreen = i;
                    fullscreen = i;
                }
            nk_combo_end(ctx);
        }
        nk_label(ctx, "WindowScale", NK_TEXT_LEFT);
        static const char* scaleOptions[] = { "1x","2x","3x","4x","5x" };
        if (nk_combo_begin_label(ctx, scaleOptions[window_scale - 1], nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 15, 1);
            for (int i = 0; i < 5; ++i)
                if (nk_combo_item_label(ctx, scaleOptions[i], NK_TEXT_LEFT))
                {
                    g_config.window_scale = i + 1;
                    window_scale = i + 1;
                }
            nk_combo_end(ctx);
        }
        if (nk_checkbox_label(ctx, "ExtendedY", &extend_y)) { g_config.extend_y = !g_config.extend_y; }
        if (nk_checkbox_label(ctx, "NewRenderer", &new_renderer)) { g_config.new_renderer = !g_config.new_renderer; }
        if (nk_checkbox_label(ctx, "EnhancedMode7", &enhanced_mode7)) { g_config.enhanced_mode7 = !g_config.enhanced_mode7; }
        if (nk_checkbox_label(ctx, "IgnoreAspectRatio", &ignore_aspect_ratio)) { g_config.ignore_aspect_ratio = !g_config.ignore_aspect_ratio; }
        if (nk_checkbox_label(ctx, "NoSpriteLimits", &no_sprite_limits)) { g_config.no_sprite_limits = !g_config.no_sprite_limits; }
        if (nk_checkbox_label(ctx, "LinearFiltering", &linear_filtering)) { g_config.linear_filtering = !g_config.linear_filtering; }
        nk_widget_disable_end(ctx);

        if (nk_button_label(ctx, "Back")) {
            currentMenu = menuRoot;
        };
    }
    nk_end(ctx);

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, "Graphics", nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "Graphics", NK_TEXT_CENTERED);
    }
    nk_end(ctx);
}

static void nkSoundMenu()
{
    int enable_audio = g_config.enable_audio;
    int audio_freq = g_config.audio_freq;
    int audio_channels = g_config.audio_channels;
    int audio_samples = g_config.audio_samples;
    int enable_msu = g_config.enable_msu;

    if (nk_begin(ctx, "Sound_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_style_set_font(ctx, &small_font->handle);

        nk_widget_disable_begin(ctx);
        if (nk_checkbox_label(ctx, "EnableAudio", &enable_audio)) { g_config.enable_audio = !g_config.enable_audio; }
        nk_label(ctx, "AudioFreq", NK_TEXT_LEFT);
        static const int freqOptions[] = { 48000,44100,32000,22050,11025};
        char currentFreq[5];
        sprintf(currentFreq, "%d", audio_freq);
        if (nk_combo_begin_label(ctx, currentFreq, nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 15, 1);
            for (int i = 0; i < 5; ++i)
            {
                char freqOption[5];
                sprintf(freqOption, "%d", freqOptions[i]);
                if (nk_combo_item_label(ctx, freqOption, NK_TEXT_LEFT))
                {
                    g_config.audio_freq = freqOptions[i];
                    audio_freq = freqOptions[i];
                }
            }
                
            nk_combo_end(ctx);
        }
        nk_label(ctx, "AudioChannels", NK_TEXT_LEFT);
        static const char* channelsOptions[] = { "Mono","Stereo" };
        if (nk_combo_begin_label(ctx, channelsOptions[audio_channels - 1], nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 15, 1);
            for (int i = 0; i < 2; ++i)
                if (nk_combo_item_label(ctx, channelsOptions[i], NK_TEXT_LEFT))
                {
                    g_config.audio_channels = i + 1;
                    audio_channels = i + 1;
                }
            nk_combo_end(ctx);
        }
        nk_label(ctx, "AudioSamples", NK_TEXT_LEFT);
        static const int samplesOptions[] = { 512,1024,2048,4096 };
        char currentsamples[5];
        sprintf(currentsamples, "%d", audio_samples);
        if (nk_combo_begin_label(ctx, currentsamples, nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 15, 1);
            for (int i = 0; i < 4; ++i)
            {
                char samplesOption[5];
                sprintf(samplesOption, "%d", samplesOptions[i]);
                if (nk_combo_item_label(ctx, samplesOption, NK_TEXT_LEFT))
                {
                    g_config.audio_samples = samplesOptions[i];
                    audio_samples = samplesOptions[i];
                }
            }
                
            nk_combo_end(ctx);
        }
        if (nk_checkbox_label(ctx, "EnableMSU", &enable_msu)) { g_config.enable_msu = !g_config.enable_msu; }
        nk_widget_disable_end(ctx);

        if (nk_button_label(ctx, "Back")) {
            currentMenu = menuRoot;
        };
    }
    nk_end(ctx);

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, "Sound", nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "Sound", NK_TEXT_CENTERED);
    }
    nk_end(ctx);
}

static void nkFeaturesMenu()
{
    int ammoRechargeStation = enhanced_features0 & kFeatures0_AmmoRechargeStation;
    int shinesparkControl = enhanced_features0 & kFeatures0_ShinesparkControl;
    int shinesparkHealth = enhanced_features0 & kFeatures0_ShinesparkHealth;
    int chainSpark = enhanced_features0 & kFeatures0_ChainSpark;
    int powerBombReveal = enhanced_features0 & kFeatures0_PowerBombReveal;
    int instantPickups = enhanced_features0 & kFeatures0_InstantPickups;

    if (nk_begin(ctx, "Features_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_style_set_font(ctx, &small_font->handle);

        if (nk_checkbox_label(ctx, "AmmoRechargeStation", &ammoRechargeStation)) { enhanced_features0 ^= kFeatures0_AmmoRechargeStation; }
        if (nk_checkbox_label(ctx, "ShinesparkControl", &shinesparkControl)) { enhanced_features0 ^= kFeatures0_ShinesparkControl; }
        if (nk_checkbox_label(ctx, "ShinesparkHealth", &shinesparkHealth)) { enhanced_features0 ^= kFeatures0_ShinesparkHealth; }
        if (nk_checkbox_label(ctx, "ChainSpark", &chainSpark)) { enhanced_features0 ^= kFeatures0_ChainSpark; }
        //nk_layout_row_dynamic(ctx, 15, 2);
        //nk_label(ctx, "LowHealthBeep", NK_TEXT_ALIGN_LEFT);
        //nk_slider_int(ctx, 0, &g_config.low_beep, 100, 1);
        //nk_layout_row_dynamic(ctx, 15, 1);
        if (nk_checkbox_label(ctx, "PowerBombReveal", &powerBombReveal)) { enhanced_features0 ^= kFeatures0_PowerBombReveal; }
        if (nk_checkbox_label(ctx, "InstantPickups", &instantPickups)) { enhanced_features0 ^= kFeatures0_InstantPickups; }

        if (nk_button_label(ctx, "Back")) {
            currentMenu = menuRoot;
        };
    }
    nk_end(ctx);

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, "Features", nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "Features", NK_TEXT_CENTERED);
    }
    nk_end(ctx);
}

static bool nkSaveLoadMenu()
{
    bool resume = false;
    if (nk_begin(ctx, "SaveLoad_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_style_set_font(ctx, &small_font->handle);

        nk_label(ctx, "Save Slot", NK_TEXT_CENTERED);
        static const char* slotOptions[] = { "Slot 0","Slot 1","Slot 2","Slot 3","Slot 4","Slot 5","Slot 6","Slot 7","Slot 8","Slot 9" };
        if (nk_combo_begin_label(ctx, slotOptions[saveSlotSelected], nk_vec2(nk_widget_width(ctx), 200))) {
            nk_layout_row_dynamic(ctx, 15, 1);
            for (int i = 0; i < 10; ++i)
                if (nk_combo_item_label(ctx, slotOptions[i], NK_TEXT_LEFT))
                {
                    saveSlotSelected = i;
                }
            nk_combo_end(ctx);
        }
        if (nk_button_label(ctx, "Save")) {
            RtlSaveLoad(kSaveLoad_Save, saveSlotSelected);
            resume = true;
        };
        if (nk_button_label(ctx, "Load")) {
            RtlSaveLoad(kSaveLoad_Load, saveSlotSelected);
            resume = true;
        };
        if (nk_button_label(ctx, "Replay")) {
            RtlSaveLoad(kSaveLoad_Replay, saveSlotSelected);
            resume = true;
        };
        if (nk_button_label(ctx, "Back")) {
            currentMenu = menuRoot;
        };
    }
    nk_end(ctx);

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, "SaveLoad", nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "Save and Load", NK_TEXT_CENTERED);
    }
    nk_end(ctx);

    return resume;
}