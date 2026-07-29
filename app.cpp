#include <stdio.h>
#include <lvgl.h>
#include "app.h"

static lv_obj_t *labelDiameter;
static lv_obj_t *labelSpeed;
static lv_obj_t *labelRPM;

static float diameter = 168.3f;
static float speed = 4.5f;

static float calcRPM()
{
    return (speed * 60.0f) / (3.14159265f * diameter);
}

void appCreate()
{
    char buf[50];

    lv_obj_clean(lv_scr_act());

    lv_obj_set_style_bg_color(lv_scr_act(),
                              lv_color_hex(0x101010),0);

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "EFD RC-1");

    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);

    lv_obj_set_pos(title, 280, 20);

// Diameter Title
lv_obj_t *diameterTitle = lv_label_create(lv_scr_act());
lv_label_set_text(diameterTitle, "Pipe Diameter");
lv_obj_set_style_text_color(diameterTitle, lv_color_white(), 0);
lv_obj_set_style_text_font(diameterTitle, &lv_font_montserrat_22, 0);
lv_obj_set_pos(diameterTitle, 60, 90);

// Diameter Button
lv_obj_t *diameterBtn = lv_btn_create(lv_scr_act());
lv_obj_set_size(diameterBtn, 300, 60);
lv_obj_set_pos(diameterBtn, 60, 120);

labelDiameter = lv_label_create(diameterBtn);

snprintf(buf, sizeof(buf), "%.1f mm", diameter);
lv_label_set_text(labelDiameter, buf);

lv_obj_set_style_text_font(labelDiameter, &lv_font_montserrat_24, 0);
lv_obj_center(labelDiameter);

lv_obj_t *speedTitle = lv_label_create(lv_scr_act());
lv_label_set_text(speedTitle, "Travel Speed");
lv_obj_set_style_text_color(speedTitle, lv_color_white(), 0);
lv_obj_set_style_text_font(speedTitle, &lv_font_montserrat_22, 0);
lv_obj_set_pos(speedTitle, 60, 210);

lv_obj_t *speedBtn = lv_btn_create(lv_scr_act());
lv_obj_set_size(speedBtn, 300, 60);
lv_obj_set_pos(speedBtn, 60, 240);

labelSpeed = lv_label_create(speedBtn);

snprintf(buf, sizeof(buf), "%.2f mm/s", speed);
lv_label_set_text(labelSpeed, buf);

lv_obj_set_style_text_font(labelSpeed, &lv_font_montserrat_24, 0);
lv_obj_center(labelSpeed);

lv_obj_t *rpmTitle = lv_label_create(lv_scr_act());
lv_label_set_text(rpmTitle, "Required RPM");
lv_obj_set_style_text_color(rpmTitle, lv_color_white(), 0);
lv_obj_set_style_text_font(rpmTitle, &lv_font_montserrat_22, 0);
lv_obj_set_pos(rpmTitle, 500, 120);

labelRPM = lv_label_create(lv_scr_act());

snprintf(buf, sizeof(buf), "%.2f RPM", calcRPM());
lv_label_set_text(labelRPM, buf);

lv_obj_set_style_text_color(labelRPM, lv_palette_main(LV_PALETTE_RED), 0);
lv_obj_set_style_text_font(labelRPM, &lv_font_montserrat_38, 0);
lv_obj_set_pos(labelRPM, 500, 170);

    lv_obj_t *startBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(startBtn,180,60);
    lv_obj_align(startBtn,LV_ALIGN_BOTTOM_LEFT,40,-40);

    lv_obj_t *startTxt = lv_label_create(startBtn);
    lv_label_set_text(startTxt,"START");
    lv_obj_center(startTxt);

    lv_obj_t *stopBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(stopBtn,180,60);
    lv_obj_align(stopBtn,LV_ALIGN_BOTTOM_RIGHT,-40,-40);

    lv_obj_t *stopTxt = lv_label_create(stopBtn);
    lv_label_set_text(stopTxt,"STOP");
    lv_obj_center(stopTxt);
}