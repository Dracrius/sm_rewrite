#include "types.h"

#include "config.h"
#include "extra_features.h"
#include "sm_rtl.h"

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

#define NK_GAMEPAD_IMPLEMENTATION
#define NK_GAMEPAD_SDL
#include "../third_party/nuklear_console/vendor/nuklear_gamepad/nuklear_gamepad.h"

#define NK_CONSOLE_IMPLEMENTATION
#include "../third_party/nuklear_console/nuklear_console.h"

static struct nk_console* console = NULL;

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

bool hideGraphicsSound = false;
bool unPause = true;
int saveSlotSelected = 0;

enum nkMenuOptions currentMenu = menuRoot;

nk_console* resume;
nk_console* general;
nk_console* graphics;
nk_console* sound;
nk_console* features;
nk_console* saveLoad;
nk_console* reset;
nk_console* quit;

int ammoRechargeStation;
int shinesparkControl;
int shinesparkHealth;
int chainSpark;
int powerBombReveal;
int instantPickups;

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

void nkRootButtons(struct nk_console* button) {
    if (strcmp(nk_console_get_label(button), "Resume") == 0) {
        unPause = true;
    }
    else if (strcmp(nk_console_get_label(button), "Reset") == 0) {
        RtlReset(1);
        unPause = true;
    }
    else if (strcmp(nk_console_get_label(button), "Quit") == 0) {
        currentMenu = menuQuit;
    }
}

void nkUpdateFeature(struct nk_console* checkbox) {
    if (strcmp(nk_console_get_label(checkbox), "AmmoRechargeStation") == 0) {
        enhanced_features0 ^= kFeatures0_AmmoRechargeStation;
    }
    else if (strcmp(nk_console_get_label(checkbox), "ShinesparkControl") == 0) {
        enhanced_features0 ^= kFeatures0_ShinesparkControl;
    }
    else if (strcmp(nk_console_get_label(checkbox), "ShinesparkHealth") == 0) {
        enhanced_features0 ^= kFeatures0_ShinesparkHealth;
    }
    else if (strcmp(nk_console_get_label(checkbox), "ChainSpark") == 0) {
        enhanced_features0 ^= kFeatures0_ChainSpark;
    }
    else if (strcmp(nk_console_get_label(checkbox), "PowerBombReveal") == 0) {
        enhanced_features0 ^= kFeatures0_PowerBombReveal;
    }
    else if (strcmp(nk_console_get_label(checkbox), "InstantPickups") == 0) {
        enhanced_features0 ^= kFeatures0_InstantPickups;
    }

}

void nkSaveLoad(struct nk_console* button) {
    if (strcmp(nk_console_get_label(button), "Save") == 0) {
        RtlSaveLoad(kSaveLoad_Save, saveSlotSelected);
        unPause = true;
    }
    if (strcmp(nk_console_get_label(button), "Load") == 0) {
        RtlSaveLoad(kSaveLoad_Load, saveSlotSelected);
        unPause = true;
    }
    if (strcmp(nk_console_get_label(button), "Replay") == 0) {
        RtlSaveLoad(kSaveLoad_Replay, saveSlotSelected);
        unPause = true;
    }
}

static void nkGeneralMenu()
{
    nk_layout_row_dynamic(ctx, 15, 1);
    nk_style_set_font(ctx, &small_font->handle);

    nk_console_checkbox(general, "Autosave", (int*)&g_config.autosave)->height = 15;
    nk_console_checkbox(general, "DisableFrameDelay", (int*)&g_config.disable_frame_delay)->disabled = nk_true;
    nk_console_checkbox(general, "Hide Graphics and Sound", (int*)&hideGraphicsSound)->height = 15;
    nk_console_button_onclick(general, "Back", nk_console_button_back)->height = 15;
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

static void nkSoundMenu()
{
    int enable_audio = g_config.enable_audio;
    int audio_freq = g_config.audio_freq;
    int audio_channels = g_config.audio_channels;
    int audio_samples = g_config.audio_samples;
    int enable_msu = g_config.enable_msu;

    nk_layout_row_dynamic(ctx, 15, 1);
    nk_style_set_font(ctx, &small_font->handle);

    nk_widget_disable_begin(ctx);
    if (nk_checkbox_label(ctx, "EnableAudio", &enable_audio)) { g_config.enable_audio = !g_config.enable_audio; }
    nk_label(ctx, "AudioFreq", NK_TEXT_LEFT);
    static const int freqOptions[] = { 48000,44100,32000,22050,11025 };
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

static void nkFeaturesMenu()
{
    ammoRechargeStation = enhanced_features0 & kFeatures0_AmmoRechargeStation;
    shinesparkControl = enhanced_features0 & kFeatures0_ShinesparkControl;
    shinesparkHealth = enhanced_features0 & kFeatures0_ShinesparkHealth;
    chainSpark = enhanced_features0 & kFeatures0_ChainSpark;
    powerBombReveal = enhanced_features0 & kFeatures0_PowerBombReveal;
    instantPickups = enhanced_features0 & kFeatures0_InstantPickups;

    nk_layout_row_dynamic(ctx, 15, 1);
    nk_style_set_font(ctx, &small_font->handle);

    nk_console_set_onchange(nk_console_checkbox(features, "AmmoRechargeStation", &ammoRechargeStation), nkUpdateFeature);
    nk_console_set_onchange(nk_console_checkbox(features, "ShinesparkControl", &shinesparkControl), nkUpdateFeature);
    nk_console_set_onchange(nk_console_checkbox(features, "ShinesparkHealth", &shinesparkHealth), nkUpdateFeature);
    nk_console_set_onchange(nk_console_checkbox(features, "ChainSpark", &chainSpark), nkUpdateFeature);
    nk_console_slider_int(features, "LowHealthBeep", 0, (int*)&g_config.low_beep, 100, 1);
    nk_console_set_onchange(nk_console_checkbox(features, "PowerBombReveal", &powerBombReveal), nkUpdateFeature);
    nk_console_set_onchange(nk_console_checkbox(features, "InstantPickups", &instantPickups), nkUpdateFeature);
    nk_console_button_onclick(features, "Back", nk_console_button_back)->height = 15;
}

static void nkSaveLoadMenu()
{
    nk_layout_row_dynamic(ctx, 15, 1);
    nk_style_set_font(ctx, &small_font->handle);

    nk_console_property_int(saveLoad, "Save Slot:", 0, &saveSlotSelected, 10, 1, 1)->height = 15;

    nk_console_button_onclick(saveLoad, "Save", nkSaveLoad)->height = 15;
    nk_console_button_onclick(saveLoad, "Load", nkSaveLoad)->height = 15;
    nk_console_button_onclick(saveLoad, "Replay", nkSaveLoad)->height = 15;
    nk_console_button_onclick(saveLoad, "Back", nk_console_button_back)->height = 15;
}

nk_console* nkMenuInit(SDL_Window* g_window, SDL_Renderer* g_renderer)
{
    ctx = nk_sdl_init(g_window, g_renderer);
    console = nk_console_init(ctx);
    nk_console_set_gamepads(console, nk_gamepad_init(ctx, NULL));

    nkFontSetup();
    nk_style_set_font(ctx, &small_font->handle);

    resume = nk_console_button_onclick(console, "Resume", nkRootButtons);
    general = nk_console_button(console, "General");
    {
        nkGeneralMenu();
    }
    //graphics = nk_console_button(console, "Graphics");
    //{
    //    nkGraphicsMenu();
    //}
    //sound = nk_console_button(console, "Sound");
    //{
    //    nkSoundMenu();
    //}
    features = nk_console_button(console, "Features");
    {
        nkFeaturesMenu();
    }
    saveLoad = nk_console_button(console, "Save and Load");
    {
        nkSaveLoadMenu();
    }
    reset =  nk_console_button_onclick(console, "Reset", nkRootButtons);
    quit = nk_console_button_onclick(console, "Quit", nkRootButtons);

    nk_console_set_height(resume, 15);
    nk_console_set_height(general, 15);
    nk_console_set_height(graphics, 15);
    nk_console_set_height(sound, 15);
    nk_console_set_height(features, 15);
    nk_console_set_height(saveLoad, 15);
    nk_console_set_height(reset, 15);
    nk_console_set_height(quit, 15);

    return console;
}

void nkMenuCleanup() {
    nk_gamepad_free((struct nk_gamepads*)nk_console_get_gamepads(console));
    nk_console_free(console);
}

static bool nkMenuRender()
{
    /*char widgetIndex[10];
    sprintf(widgetIndex, "Index: %d", nk_console_get_widget_index(nk_console_get_active_widget(console)));

    puts(widgetIndex);*/

    unPause = false;
    nk_console_render(console);
    return unPause;
}



bool nkRootMenu()
{
    if (nk_begin(ctx, "Paused_List", nk_rect(28, 74, 200, 100), NK_WINDOW_BACKGROUND))
    {
        nk_style_set_font(ctx, &small_font->handle);
        unPause = nkMenuRender();
    }
    nk_end(ctx);

    char* menuTitle = "Paused";

    /*if (nk_console_get_active_widget(console) == general)
    {
        menuTitle = "General";
    }
    if (nk_console_is_active_widget(graphics))
    {
        menuTitle = "Graphics";
    }
    if (nk_console_is_active_widget(sound))
    {
        menuTitle = "Sound";
    }
    if (nk_console_is_active_widget(features))
    {
        menuTitle = "Features";
    }
    if (nk_console_is_active_widget(saveLoad))
    {
        menuTitle = "Save and Load";
    }
    else
    {
        menuTitle = "Paused";
    }*/


    /*switch (currentMenu)
    {
    case menuRoot:
        menuTitle = "Paused";
        break;
    case menuGeneral:
        menuTitle = "General";
        break;
    case menuGraphics:
        menuTitle = "Graphics";
        break;
    case menuSound:
        menuTitle = "Sound";
        break;
    case menuFeatures:
        menuTitle = "Features";
        break;
    case menuSaveLoad:
        menuTitle = "Save and Load";
        break;
    default:
        menuTitle = "Paused";
        break;
    }*/

    nk_style_set_font(ctx, &large_font->handle);
    if (nk_begin(ctx, menuTitle, nk_rect(28, 50, 200, 40), NK_WINDOW_BACKGROUND))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, menuTitle, NK_TEXT_CENTERED);
    }
    nk_end(ctx);

    return unPause;
}
