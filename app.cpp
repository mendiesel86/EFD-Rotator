#include <stdio.h>
#include <stdlib.h>
#include <lvgl.h>
#include "app.h"

#define PI_VALUE 3.14159265f

enum EditField
{
    EDIT_NONE,
    EDIT_DIAMETER,
    EDIT_SPEED
};

static lv_obj_t *labelDiameter = nullptr;
static lv_obj_t *labelSpeed = nullptr;
static lv_obj_t *labelRPM = nullptr;
static lv_obj_t *labelStatus = nullptr;
static lv_obj_t *inputOverlay = nullptr;
static lv_obj_t *inputTextArea = nullptr;

static float diameter = 168.3f;
static float speed = 4.5f;
static EditField activeField = EDIT_NONE;

static float calcRPM()
{
    if (diameter <= 0.0f)
        return 0.0f;

    return (speed * 60.0f) / (PI_VALUE * diameter);
}

static void updateDisplayValues()
{
    char buffer[50];

    snprintf(buffer, sizeof(buffer), "%.1f mm", diameter);
    lv_label_set_text(labelDiameter, buffer);

    snprintf(buffer, sizeof(buffer), "%.2f mm/s", speed);
    lv_label_set_text(labelSpeed, buffer);

    snprintf(buffer, sizeof(buffer), "%.2f RPM", calcRPM());
    lv_label_set_text(labelRPM, buffer);
}

static void closeInputPopup()
{
    if (inputOverlay != nullptr)
    {
        lv_obj_del(inputOverlay);
        inputOverlay = nullptr;
        inputTextArea = nullptr;
    }

    activeField = EDIT_NONE;
}

static void applyInputValue()
{
    if (inputTextArea == nullptr)
        return;

    const char *text = lv_textarea_get_text(inputTextArea);
    float newValue = atof(text);

    if (newValue <= 0.0f)
        return;

    if (activeField == EDIT_DIAMETER)
        diameter = newValue;
    else if (activeField == EDIT_SPEED)
        speed = newValue;

    updateDisplayValues();
    closeInputPopup();
}

static void keypadEvent(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_READY)
        applyInputValue();
    else if (code == LV_EVENT_CANCEL)
        closeInputPopup();
}

static void cancelButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        closeInputPopup();
}

static void okButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        applyInputValue();
}

static void openInputPopup(EditField field)
{
    if (inputOverlay != nullptr)
        return;

    activeField = field;

    inputOverlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(inputOverlay);
    lv_obj_set_size(inputOverlay, 800, 480);
    lv_obj_set_pos(inputOverlay, 0, 0);
    lv_obj_set_style_bg_color(inputOverlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(inputOverlay, LV_OPA_80, 0);
    lv_obj_clear_flag(inputOverlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *panel = lv_obj_create(inputOverlay);
    lv_obj_set_size(panel, 660, 440);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1B1B1B), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x555555), 0);
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title,
                      field == EDIT_DIAMETER
                          ? "Enter Pipe Diameter (mm)"
                          : "Enter Travel Speed (mm/s)");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    inputTextArea = lv_textarea_create(panel);
    lv_obj_set_size(inputTextArea, 300, 55);
    lv_obj_align(inputTextArea, LV_ALIGN_TOP_MID, 0, 45);
    lv_textarea_set_one_line(inputTextArea, true);
    lv_textarea_set_max_length(inputTextArea, 10);
    lv_obj_set_style_text_font(inputTextArea, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(inputTextArea, LV_TEXT_ALIGN_CENTER, 0);

    char currentValue[24];
    if (field == EDIT_DIAMETER)
        snprintf(currentValue, sizeof(currentValue), "%.1f", diameter);
    else
        snprintf(currentValue, sizeof(currentValue), "%.2f", speed);

    lv_textarea_set_text(inputTextArea, currentValue);
    lv_textarea_set_cursor_pos(inputTextArea, LV_TEXTAREA_CURSOR_LAST);

    lv_obj_t *keyboard = lv_keyboard_create(panel);
    lv_obj_set_size(keyboard, 610, 245);
    lv_obj_align(keyboard, LV_ALIGN_TOP_MID, 0, 110);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(keyboard, inputTextArea);
    lv_obj_add_event_cb(keyboard, keypadEvent, LV_EVENT_ALL, nullptr);

    lv_obj_t *cancelButton = lv_btn_create(panel);
    lv_obj_set_size(cancelButton, 180, 52);
    lv_obj_align(cancelButton, LV_ALIGN_BOTTOM_LEFT, 35, -8);
    lv_obj_set_style_bg_color(cancelButton, lv_color_hex(0x555555), 0);
    lv_obj_add_event_cb(cancelButton, cancelButtonEvent, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *cancelText = lv_label_create(cancelButton);
    lv_label_set_text(cancelText, "CANCEL");
    lv_obj_set_style_text_font(cancelText, &lv_font_montserrat_22, 0);
    lv_obj_center(cancelText);

    lv_obj_t *okButton = lv_btn_create(panel);
    lv_obj_set_size(okButton, 180, 52);
    lv_obj_align(okButton, LV_ALIGN_BOTTOM_RIGHT, -35, -8);
    lv_obj_set_style_bg_color(okButton, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_add_event_cb(okButton, okButtonEvent, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *okText = lv_label_create(okButton);
    lv_label_set_text(okText, "OK");
    lv_obj_set_style_text_font(okText, &lv_font_montserrat_22, 0);
    lv_obj_center(okText);
}

static void diameterButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        openInputPopup(EDIT_DIAMETER);
}

static void speedButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        openInputPopup(EDIT_SPEED);
}

static void startButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
        return;

    lv_label_set_text(labelStatus, "RUNNING");
    lv_obj_set_style_text_color(labelStatus,
                                lv_palette_main(LV_PALETTE_GREEN),
                                0);
}

static void stopButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
        return;

    lv_label_set_text(labelStatus, "STOPPED");
    lv_obj_set_style_text_color(labelStatus,
                                lv_palette_main(LV_PALETTE_RED),
                                0);
}

void appCreate()
{
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x101010), 0);

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "EFD RC-1");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    lv_obj_t *version = lv_label_create(lv_scr_act());
    lv_label_set_text(version, "v0.3.1");
    lv_obj_set_style_text_color(version, lv_color_hex(0x888888), 0);
    lv_obj_align(version, LV_ALIGN_TOP_LEFT, 18, 18);

    labelStatus = lv_label_create(lv_scr_act());
    lv_label_set_text(labelStatus, "READY");
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(labelStatus,
                                lv_palette_main(LV_PALETTE_GREEN),
                                0);
    lv_obj_align(labelStatus, LV_ALIGN_TOP_RIGHT, -28, 25);

    lv_obj_t *diameterTitle = lv_label_create(lv_scr_act());
    lv_label_set_text(diameterTitle, "Pipe Diameter");
    lv_obj_set_style_text_font(diameterTitle, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(diameterTitle, lv_color_white(), 0);
    lv_obj_set_pos(diameterTitle, 55, 90);

    lv_obj_t *diameterButton = lv_btn_create(lv_scr_act());
    lv_obj_set_size(diameterButton, 325, 65);
    lv_obj_set_pos(diameterButton, 55, 122);
    lv_obj_set_style_bg_color(diameterButton, lv_color_hex(0x2D5F88), 0);
    lv_obj_add_event_cb(diameterButton,
                        diameterButtonEvent,
                        LV_EVENT_CLICKED,
                        nullptr);

    labelDiameter = lv_label_create(diameterButton);
    lv_obj_set_style_text_font(labelDiameter, &lv_font_montserrat_24, 0);
    lv_obj_center(labelDiameter);

    lv_obj_t *speedTitle = lv_label_create(lv_scr_act());
    lv_label_set_text(speedTitle, "Travel Speed");
    lv_obj_set_style_text_font(speedTitle, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(speedTitle, lv_color_white(), 0);
    lv_obj_set_pos(speedTitle, 55, 215);

    lv_obj_t *speedButton = lv_btn_create(lv_scr_act());
    lv_obj_set_size(speedButton, 325, 65);
    lv_obj_set_pos(speedButton, 55, 247);
    lv_obj_set_style_bg_color(speedButton, lv_color_hex(0x2D5F88), 0);
    lv_obj_add_event_cb(speedButton,
                        speedButtonEvent,
                        LV_EVENT_CLICKED,
                        nullptr);

    labelSpeed = lv_label_create(speedButton);
    lv_obj_set_style_text_font(labelSpeed, &lv_font_montserrat_24, 0);
    lv_obj_center(labelSpeed);

    lv_obj_t *rpmPanel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rpmPanel, 345, 190);
    lv_obj_set_pos(rpmPanel, 420, 105);
    lv_obj_set_style_bg_color(rpmPanel, lv_color_hex(0x181818), 0);
    lv_obj_set_style_border_color(rpmPanel, lv_color_hex(0x444444), 0);
    lv_obj_set_style_border_width(rpmPanel, 2, 0);
    lv_obj_set_style_radius(rpmPanel, 10, 0);
    lv_obj_clear_flag(rpmPanel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rpmTitle = lv_label_create(rpmPanel);
    lv_label_set_text(rpmTitle, "Required Roller RPM");
    lv_obj_set_style_text_font(rpmTitle, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(rpmTitle, lv_color_white(), 0);
    lv_obj_align(rpmTitle, LV_ALIGN_TOP_MID, 0, 18);

    labelRPM = lv_label_create(rpmPanel);
    lv_obj_set_style_text_font(labelRPM, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(labelRPM,
                                lv_palette_main(LV_PALETTE_RED),
                                0);
    lv_obj_align(labelRPM, LV_ALIGN_CENTER, 0, 25);

    lv_obj_t *hint = lv_label_create(lv_scr_act());
    lv_label_set_text(hint, "Touch a blue value box to edit");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -118);

    lv_obj_t *startButton = lv_btn_create(lv_scr_act());
    lv_obj_set_size(startButton, 245, 72);
    lv_obj_align(startButton, LV_ALIGN_BOTTOM_LEFT, 55, -28);
    lv_obj_set_style_bg_color(startButton,
                              lv_palette_main(LV_PALETTE_GREEN),
                              0);
    lv_obj_add_event_cb(startButton,
                        startButtonEvent,
                        LV_EVENT_CLICKED,
                        nullptr);

    lv_obj_t *startText = lv_label_create(startButton);
    lv_label_set_text(startText, "START");
    lv_obj_set_style_text_font(startText, &lv_font_montserrat_24, 0);
    lv_obj_center(startText);

    lv_obj_t *stopButton = lv_btn_create(lv_scr_act());
    lv_obj_set_size(stopButton, 245, 72);
    lv_obj_align(stopButton, LV_ALIGN_BOTTOM_RIGHT, -55, -28);
    lv_obj_set_style_bg_color(stopButton,
                              lv_palette_main(LV_PALETTE_RED),
                              0);
    lv_obj_add_event_cb(stopButton,
                        stopButtonEvent,
                        LV_EVENT_CLICKED,
                        nullptr);

    lv_obj_t *stopText = lv_label_create(stopButton);
    lv_label_set_text(stopText, "STOP");
    lv_obj_set_style_text_font(stopText, &lv_font_montserrat_24, 0);
    lv_obj_center(stopText);

    updateDisplayValues();
}
