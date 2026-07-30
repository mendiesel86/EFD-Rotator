#include <stdio.h>
#include <lvgl.h>
#include "app.h"

static lv_obj_t *labelDiameter;
static lv_obj_t *labelSpeed;
static lv_obj_t *labelRPM;

static float diameter = 168.3f;
static float speed = 4.5f;

static lv_obj_t *splashScreen = nullptr;
static lv_obj_t *shimmer = nullptr;
static lv_obj_t *loadingFill = nullptr;

static float calcRPM()
{
    return (speed * 60.0f) / (3.14159265f * diameter);
}

static void setScreenBackground(lv_obj_t *screen, uint32_t colour)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(colour), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
}

static lv_obj_t *createLogoLetter(lv_obj_t *parent, const char *text, uint32_t colour,
                                  lv_coord_t x, lv_coord_t y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(colour), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_obj_set_pos(label, x, y);
    return label;
}

static void shimmerAnim(void *object, int32_t value)
{
    lv_obj_set_x((lv_obj_t *)object, (lv_coord_t)value);
}

static void loadingAnim(void *object, int32_t value)
{
    lv_obj_set_width((lv_obj_t *)object, (lv_coord_t)value);
}

static void createHomeScreen();

static void finishSplash(lv_timer_t *timer)
{
    lv_timer_del(timer);
    createHomeScreen();
}

static void createSplashScreen()
{
    splashScreen = lv_obj_create(nullptr);
    lv_obj_clear_flag(splashScreen, LV_OBJ_FLAG_SCROLLABLE);
    setScreenBackground(splashScreen, 0x050505);
    lv_scr_load(splashScreen);

    lv_obj_t *logoBox = lv_obj_create(splashScreen);
    lv_obj_set_size(logoBox, 500, 130);
    lv_obj_align(logoBox, LV_ALIGN_TOP_MID, 0, 58);
    lv_obj_set_style_bg_color(logoBox, lv_color_hex(0x050505), 0);
    lv_obj_set_style_bg_opa(logoBox, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(logoBox, 0, 0);
    lv_obj_set_style_radius(logoBox, 0, 0);
    lv_obj_set_style_pad_all(logoBox, 0, 0);
    lv_obj_clear_flag(logoBox, LV_OBJ_FLAG_SCROLLABLE);

    createLogoLetter(logoBox, "E", 0xF2F2F2, 92, 25);
    createLogoLetter(logoBox, "F", 0xED1C24, 215, 25);
    createLogoLetter(logoBox, "D", 0xF2F2F2, 335, 25);

    shimmer = lv_obj_create(logoBox);
    lv_obj_set_size(shimmer, 42, 92);
    lv_obj_set_y(shimmer, 18);
    lv_obj_set_style_bg_color(shimmer, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(shimmer, LV_OPA_40, 0);
    lv_obj_set_style_border_width(shimmer, 0, 0);
    lv_obj_set_style_radius(shimmer, 18, 0);
    lv_obj_set_style_shadow_color(shimmer, lv_color_white(), 0);
    lv_obj_set_style_shadow_opa(shimmer, LV_OPA_50, 0);
    lv_obj_set_style_shadow_width(shimmer, 18, 0);
    lv_obj_clear_flag(shimmer, LV_OBJ_FLAG_SCROLLABLE);

    lv_anim_t shimmerAnimation;
    lv_anim_init(&shimmerAnimation);
    lv_anim_set_var(&shimmerAnimation, shimmer);
    lv_anim_set_exec_cb(&shimmerAnimation, shimmerAnim);
    lv_anim_set_values(&shimmerAnimation, -60, 520);
    lv_anim_set_time(&shimmerAnimation, 1300);
    lv_anim_set_delay(&shimmerAnimation, 350);
    lv_anim_set_repeat_count(&shimmerAnimation, 1);
    lv_anim_set_repeat_delay(&shimmerAnimation, 150);
    lv_anim_set_path_cb(&shimmerAnimation, lv_anim_path_ease_in_out);
    lv_anim_start(&shimmerAnimation);

    lv_obj_t *company = lv_label_create(splashScreen);
    lv_label_set_text(company, "E V O L U T I O N   F A B R I C A T I O N S   A N D   D E V E L O P M E N T S");
    lv_obj_set_style_text_color(company, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_text_font(company, &lv_font_montserrat_14, 0);
    lv_obj_align(company, LV_ALIGN_TOP_MID, 0, 215);

    lv_obj_t *product = lv_label_create(splashScreen);
    lv_label_set_text(product, "R O T A T O R   C O N T R O L L E R");
    lv_obj_set_style_text_color(product, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_text_font(product, &lv_font_montserrat_18, 0);
    lv_obj_align(product, LV_ALIGN_TOP_MID, 0, 255);

    lv_obj_t *loadingTrack = lv_obj_create(splashScreen);
    lv_obj_set_size(loadingTrack, 500, 5);
    lv_obj_align(loadingTrack, LV_ALIGN_TOP_MID, 0, 340);
    lv_obj_set_style_bg_color(loadingTrack, lv_color_hex(0x303030), 0);
    lv_obj_set_style_bg_opa(loadingTrack, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(loadingTrack, 0, 0);
    lv_obj_set_style_radius(loadingTrack, 3, 0);
    lv_obj_set_style_pad_all(loadingTrack, 0, 0);
    lv_obj_clear_flag(loadingTrack, LV_OBJ_FLAG_SCROLLABLE);

    loadingFill = lv_obj_create(loadingTrack);
    lv_obj_set_size(loadingFill, 0, 5);
    lv_obj_align(loadingFill, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(loadingFill, lv_color_hex(0xED1C24), 0);
    lv_obj_set_style_bg_opa(loadingFill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(loadingFill, 0, 0);
    lv_obj_set_style_radius(loadingFill, 3, 0);
    lv_obj_set_style_shadow_color(loadingFill, lv_color_hex(0xED1C24), 0);
    lv_obj_set_style_shadow_opa(loadingFill, LV_OPA_70, 0);
    lv_obj_set_style_shadow_width(loadingFill, 10, 0);
    lv_obj_clear_flag(loadingFill, LV_OBJ_FLAG_SCROLLABLE);

    lv_anim_t loadingAnimation;
    lv_anim_init(&loadingAnimation);
    lv_anim_set_var(&loadingAnimation, loadingFill);
    lv_anim_set_exec_cb(&loadingAnimation, loadingAnim);
    lv_anim_set_values(&loadingAnimation, 0, 500);
    lv_anim_set_time(&loadingAnimation, 2700);
    lv_anim_set_delay(&loadingAnimation, 150);
    lv_anim_set_path_cb(&loadingAnimation, lv_anim_path_ease_in_out);
    lv_anim_start(&loadingAnimation);

    lv_obj_t *loadingText = lv_label_create(splashScreen);
    lv_label_set_text(loadingText, "L O A D I N G . . .");
    lv_obj_set_style_text_color(loadingText, lv_color_hex(0xD8D8D8), 0);
    lv_obj_set_style_text_font(loadingText, &lv_font_montserrat_14, 0);
    lv_obj_align(loadingText, LV_ALIGN_TOP_MID, 0, 370);

    lv_timer_create(finishSplash, 3000, nullptr);
}

static void createHomeScreen()
{
    char buf[50];

    lv_obj_t *homeScreen = lv_obj_create(nullptr);
    lv_obj_clear_flag(homeScreen, LV_OBJ_FLAG_SCROLLABLE);
    setScreenBackground(homeScreen, 0x101010);
    lv_scr_load_anim(homeScreen, LV_SCR_LOAD_ANIM_FADE_ON, 350, 0, true);

    lv_obj_t *title = lv_label_create(homeScreen);
    lv_label_set_text(title, "EFD RC-1");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_set_pos(title, 280, 20);

    lv_obj_t *diameterTitle = lv_label_create(homeScreen);
    lv_label_set_text(diameterTitle, "Pipe Diameter");
    lv_obj_set_style_text_color(diameterTitle, lv_color_white(), 0);
    lv_obj_set_style_text_font(diameterTitle, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(diameterTitle, 60, 90);

    lv_obj_t *diameterBtn = lv_btn_create(homeScreen);
    lv_obj_set_size(diameterBtn, 300, 60);
    lv_obj_set_pos(diameterBtn, 60, 120);

    labelDiameter = lv_label_create(diameterBtn);
    snprintf(buf, sizeof(buf), "%.1f mm", diameter);
    lv_label_set_text(labelDiameter, buf);
    lv_obj_set_style_text_font(labelDiameter, &lv_font_montserrat_24, 0);
    lv_obj_center(labelDiameter);

    lv_obj_t *speedTitle = lv_label_create(homeScreen);
    lv_label_set_text(speedTitle, "Travel Speed");
    lv_obj_set_style_text_color(speedTitle, lv_color_white(), 0);
    lv_obj_set_style_text_font(speedTitle, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(speedTitle, 60, 210);

    lv_obj_t *speedBtn = lv_btn_create(homeScreen);
    lv_obj_set_size(speedBtn, 300, 60);
    lv_obj_set_pos(speedBtn, 60, 240);

    labelSpeed = lv_label_create(speedBtn);
    snprintf(buf, sizeof(buf), "%.2f mm/s", speed);
    lv_label_set_text(labelSpeed, buf);
    lv_obj_set_style_text_font(labelSpeed, &lv_font_montserrat_24, 0);
    lv_obj_center(labelSpeed);

    lv_obj_t *rpmTitle = lv_label_create(homeScreen);
    lv_label_set_text(rpmTitle, "Required RPM");
    lv_obj_set_style_text_color(rpmTitle, lv_color_white(), 0);
    lv_obj_set_style_text_font(rpmTitle, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(rpmTitle, 500, 120);

    labelRPM = lv_label_create(homeScreen);
    snprintf(buf, sizeof(buf), "%.2f RPM", calcRPM());
    lv_label_set_text(labelRPM, buf);
    lv_obj_set_style_text_color(labelRPM, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_text_font(labelRPM, &lv_font_montserrat_38, 0);
    lv_obj_set_pos(labelRPM, 500, 170);

    lv_obj_t *startBtn = lv_btn_create(homeScreen);
    lv_obj_set_size(startBtn, 180, 60);
    lv_obj_align(startBtn, LV_ALIGN_BOTTOM_LEFT, 40, -40);

    lv_obj_t *startTxt = lv_label_create(startBtn);
    lv_label_set_text(startTxt, "START");
    lv_obj_center(startTxt);

    lv_obj_t *stopBtn = lv_btn_create(homeScreen);
    lv_obj_set_size(stopBtn, 180, 60);
    lv_obj_align(stopBtn, LV_ALIGN_BOTTOM_RIGHT, -40, -40);

    lv_obj_t *stopTxt = lv_label_create(stopBtn);
    lv_label_set_text(stopTxt, "STOP");
    lv_obj_center(stopTxt);
}

void appCreate()
{
    createSplashScreen();
}
