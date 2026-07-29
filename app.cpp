#include <stdio.h>
#include <stdlib.h>
#include <lvgl.h>
#include "app.h"
#include "motor_control.h"

#define PI_VALUE 3.14159265f
#define MIN_DIAMETER_MM 10.0f
#define MAX_DIAMETER_MM 2000.0f
#define MIN_SPEED_MMPS 0.10f
#define MAX_SPEED_MMPS 50.0f
#define MIN_TEST_POWER 5.0f
#define MAX_TEST_POWER 100.0f

enum EditField
{
    EDIT_NONE,
    EDIT_DIAMETER,
    EDIT_SPEED,
    EDIT_POWER
};

static lv_obj_t *labelDiameter = nullptr;
static lv_obj_t *labelSpeed = nullptr;
static lv_obj_t *labelRPM = nullptr;
static lv_obj_t *labelStatus = nullptr;
static lv_obj_t *labelPower = nullptr;
static lv_obj_t *labelDirection = nullptr;
static lv_obj_t *labelActualPower = nullptr;
static lv_obj_t *inputOverlay = nullptr;
static lv_obj_t *inputTextArea = nullptr;
static lv_obj_t *inputErrorLabel = nullptr;
static lv_obj_t *diameterButton = nullptr;
static lv_obj_t *speedButton = nullptr;
static lv_obj_t *powerButton = nullptr;
static lv_obj_t *directionButton = nullptr;
static lv_obj_t *startButton = nullptr;
static lv_obj_t *stopButton = nullptr;
static lv_obj_t *hintLabel = nullptr;

static float diameter = 168.3f;
static float speed = 4.5f;
static float testPower = 25.0f;
static EditField activeField = EDIT_NONE;
static bool machineRunning = false;

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

    snprintf(buffer, sizeof(buffer), "%.0f%%", testPower);
    lv_label_set_text(labelPower, buffer);

    lv_label_set_text(labelDirection,
                      motorControlIsForward() ? "FORWARD" : "REVERSE");
}

static void updateMotorStatus()
{
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "Output %.0f%%",
             motorControlGetActualPercent());
    lv_label_set_text(labelActualPower, buffer);
}

static void motorTimerEvent(lv_timer_t *timer)
{
    (void)timer;
    motorControlUpdate();
    updateMotorStatus();

    if (!machineRunning && motorControlGetActualPercent() <= 0.0f)
    {
        lv_label_set_text(labelStatus, "READY");
        lv_obj_set_style_text_color(labelStatus,
                                    lv_palette_main(LV_PALETTE_GREEN), 0);
    }
}

static void setControlLock(bool locked)
{
    lv_obj_t *controls[] = {
        diameterButton, speedButton, powerButton, directionButton, startButton};

    for (lv_obj_t *control : controls)
    {
        if (locked)
            lv_obj_add_state(control, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(control, LV_STATE_DISABLED);
    }

    if (locked)
        lv_obj_clear_state(stopButton, LV_STATE_DISABLED);
    else
        lv_obj_add_state(stopButton, LV_STATE_DISABLED);
}

static void setMachineState(bool running)
{
    machineRunning = running;

    if (running)
    {
        motorControlSetTargetPercent(testPower);
        motorControlStart();
        lv_label_set_text(labelStatus, "RUNNING");
        lv_obj_set_style_text_color(labelStatus,
                                    lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_label_set_text(hintLabel, "Soft start active - press STOP at any time");
        setControlLock(true);
    }
    else
    {
        motorControlStop();
        lv_label_set_text(labelStatus, "STOPPING");
        lv_obj_set_style_text_color(labelStatus,
                                    lv_palette_main(LV_PALETTE_ORANGE), 0);
        lv_label_set_text(hintLabel, "Soft stop active");
        setControlLock(false);
    }
}

static void showInputError(const char *message)
{
    if (inputErrorLabel == nullptr)
        return;

    lv_label_set_text(inputErrorLabel, message);
    lv_obj_clear_flag(inputErrorLabel, LV_OBJ_FLAG_HIDDEN);
}

static void clearInputError()
{
    if (inputErrorLabel != nullptr)
        lv_obj_add_flag(inputErrorLabel, LV_OBJ_FLAG_HIDDEN);
}

static void closeInputPopup()
{
    if (inputOverlay != nullptr)
    {
        lv_obj_del(inputOverlay);
        inputOverlay = nullptr;
        inputTextArea = nullptr;
        inputErrorLabel = nullptr;
    }

    activeField = EDIT_NONE;
}

static bool validateInputValue(float value)
{
    if (activeField == EDIT_DIAMETER &&
        (value < MIN_DIAMETER_MM || value > MAX_DIAMETER_MM))
    {
        showInputError("Diameter must be 10 to 2000 mm");
        return false;
    }

    if (activeField == EDIT_SPEED &&
        (value < MIN_SPEED_MMPS || value > MAX_SPEED_MMPS))
    {
        showInputError("Speed must be 0.10 to 50.00 mm/s");
        return false;
    }

    if (activeField == EDIT_POWER &&
        (value < MIN_TEST_POWER || value > MAX_TEST_POWER))
    {
        showInputError("Test power must be 5 to 100 percent");
        return false;
    }

    return true;
}

static void applyInputValue()
{
    if (inputTextArea == nullptr)
        return;

    const char *text = lv_textarea_get_text(inputTextArea);
    if (text == nullptr || text[0] == '\0')
    {
        showInputError("Enter a value");
        return;
    }

    char *endPointer = nullptr;
    float newValue = strtof(text, &endPointer);
    if (endPointer == text || *endPointer != '\0')
    {
        showInputError("Enter numbers only");
        return;
    }

    if (!validateInputValue(newValue))
        return;

    if (activeField == EDIT_DIAMETER)
        diameter = newValue;
    else if (activeField == EDIT_SPEED)
        speed = newValue;
    else if (activeField == EDIT_POWER)
    {
        testPower = newValue;
        motorControlSetTargetPercent(testPower);
    }

    updateDisplayValues();
    closeInputPopup();
}

static void keypadEvent(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_VALUE_CHANGED)
        clearInputError();
    else if (code == LV_EVENT_READY)
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
    if (inputOverlay != nullptr || machineRunning)
        return;

    activeField = field;
    inputOverlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(inputOverlay);
    lv_obj_set_size(inputOverlay, 800, 480);
    lv_obj_set_style_bg_color(inputOverlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(inputOverlay, LV_OPA_80, 0);

    lv_obj_t *panel = lv_obj_create(inputOverlay);
    lv_obj_set_size(panel, 700, 450);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1B1B1B), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    const char *titleText = "Enter Test Power (%)";
    if (field == EDIT_DIAMETER)
        titleText = "Enter Pipe Diameter (mm)";
    else if (field == EDIT_SPEED)
        titleText = "Enter Travel Speed (mm/s)";

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, titleText);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    inputTextArea = lv_textarea_create(panel);
    lv_obj_set_size(inputTextArea, 330, 58);
    lv_obj_align(inputTextArea, LV_ALIGN_TOP_MID, 0, 42);
    lv_textarea_set_one_line(inputTextArea, true);
    lv_textarea_set_max_length(inputTextArea, 8);
    lv_obj_set_style_text_font(inputTextArea, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(inputTextArea, LV_TEXT_ALIGN_CENTER, 0);

    char currentValue[24];
    if (field == EDIT_DIAMETER)
        snprintf(currentValue, sizeof(currentValue), "%.1f", diameter);
    else if (field == EDIT_SPEED)
        snprintf(currentValue, sizeof(currentValue), "%.2f", speed);
    else
        snprintf(currentValue, sizeof(currentValue), "%.0f", testPower);
    lv_textarea_set_text(inputTextArea, currentValue);

    inputErrorLabel = lv_label_create(panel);
    lv_obj_set_style_text_color(inputErrorLabel,
                                lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align(inputErrorLabel, LV_ALIGN_TOP_MID, 0, 104);
    lv_obj_add_flag(inputErrorLabel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *keyboard = lv_keyboard_create(panel);
    lv_obj_set_size(keyboard, 650, 240);
    lv_obj_align(keyboard, LV_ALIGN_TOP_MID, 0, 128);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(keyboard, inputTextArea);
    lv_obj_add_event_cb(keyboard, keypadEvent, LV_EVENT_ALL, nullptr);

    lv_obj_t *cancelButton = lv_btn_create(panel);
    lv_obj_set_size(cancelButton, 210, 52);
    lv_obj_align(cancelButton, LV_ALIGN_BOTTOM_LEFT, 32, -7);
    lv_obj_set_style_bg_color(cancelButton, lv_color_hex(0x555555), 0);
    lv_obj_add_event_cb(cancelButton, cancelButtonEvent, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *cancelText = lv_label_create(cancelButton);
    lv_label_set_text(cancelText, "CANCEL");
    lv_obj_center(cancelText);

    lv_obj_t *okButton = lv_btn_create(panel);
    lv_obj_set_size(okButton, 210, 52);
    lv_obj_align(okButton, LV_ALIGN_BOTTOM_RIGHT, -32, -7);
    lv_obj_set_style_bg_color(okButton, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_add_event_cb(okButton, okButtonEvent, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *okText = lv_label_create(okButton);
    lv_label_set_text(okText, "OK");
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

static void powerButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        openInputPopup(EDIT_POWER);
}

static void directionButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED || machineRunning)
        return;

    motorControlSetDirection(!motorControlIsForward());
    updateDisplayValues();
}

static void startButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        setMachineState(true);
}

static void stopButtonEvent(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        setMachineState(false);
}

static lv_obj_t *createValueButton(int x, int y, int width,
                                   lv_event_cb_t callback, lv_obj_t **label)
{
    lv_obj_t *button = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button, width, 58);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x2D5F88), 0);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);
    *label = lv_label_create(button);
    lv_obj_set_style_text_font(*label, &lv_font_montserrat_22, 0);
    lv_obj_center(*label);
    return button;
}

void appCreate()
{
    motorControlBegin();
    motorControlSetTargetPercent(testPower);

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x101010), 0);

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "EFD RC-1");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *version = lv_label_create(lv_scr_act());
    lv_label_set_text(version, "v0.4.0");
    lv_obj_set_style_text_color(version, lv_color_hex(0x888888), 0);
    lv_obj_set_pos(version, 15, 15);

    labelStatus = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_20, 0);
    lv_obj_align(labelStatus, LV_ALIGN_TOP_RIGHT, -20, 18);

    lv_obj_t *diameterTitle = lv_label_create(lv_scr_act());
    lv_label_set_text(diameterTitle, "Pipe Diameter");
    lv_obj_set_pos(diameterTitle, 45, 72);
    diameterButton = createValueButton(45, 96, 290,
                                       diameterButtonEvent, &labelDiameter);

    lv_obj_t *speedTitle = lv_label_create(lv_scr_act());
    lv_label_set_text(speedTitle, "Travel Speed");
    lv_obj_set_pos(speedTitle, 45, 170);
    speedButton = createValueButton(45, 194, 290,
                                    speedButtonEvent, &labelSpeed);

    lv_obj_t *rpmPanel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rpmPanel, 370, 112);
    lv_obj_set_pos(rpmPanel, 385, 78);
    lv_obj_set_style_bg_color(rpmPanel, lv_color_hex(0x181818), 0);
    lv_obj_set_style_border_color(rpmPanel, lv_color_hex(0x444444), 0);
    lv_obj_clear_flag(rpmPanel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rpmTitle = lv_label_create(rpmPanel);
    lv_label_set_text(rpmTitle, "Required Roller RPM");
    lv_obj_align(rpmTitle, LV_ALIGN_TOP_MID, 0, 5);
    labelRPM = lv_label_create(rpmPanel);
    lv_obj_set_style_text_font(labelRPM, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(labelRPM, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align(labelRPM, LV_ALIGN_BOTTOM_MID, 0, -7);

    lv_obj_t *powerTitle = lv_label_create(lv_scr_act());
    lv_label_set_text(powerTitle, "Motor Test Power");
    lv_obj_set_pos(powerTitle, 385, 205);
    powerButton = createValueButton(385, 229, 170,
                                    powerButtonEvent, &labelPower);

    directionButton = createValueButton(575, 229, 180,
                                        directionButtonEvent, &labelDirection);

    labelActualPower = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(labelActualPower, lv_color_hex(0xBBBBBB), 0);
    lv_obj_align(labelActualPower, LV_ALIGN_CENTER, 0, 70);

    hintLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(hintLabel, "Initial test is limited to 25% - verify direction first");
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_MID, 0, -100);

    startButton = lv_btn_create(lv_scr_act());
    lv_obj_set_size(startButton, 245, 68);
    lv_obj_align(startButton, LV_ALIGN_BOTTOM_LEFT, 55, -20);
    lv_obj_set_style_bg_color(startButton, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_add_event_cb(startButton, startButtonEvent, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *startText = lv_label_create(startButton);
    lv_label_set_text(startText, "START MOTOR");
    lv_obj_set_style_text_font(startText, &lv_font_montserrat_22, 0);
    lv_obj_center(startText);

    stopButton = lv_btn_create(lv_scr_act());
    lv_obj_set_size(stopButton, 245, 68);
    lv_obj_align(stopButton, LV_ALIGN_BOTTOM_RIGHT, -55, -20);
    lv_obj_set_style_bg_color(stopButton, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_event_cb(stopButton, stopButtonEvent, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *stopText = lv_label_create(stopButton);
    lv_label_set_text(stopText, "STOP MOTOR");
    lv_obj_set_style_text_font(stopText, &lv_font_montserrat_22, 0);
    lv_obj_center(stopText);

    updateDisplayValues();
    updateMotorStatus();
    machineRunning = false;
    setControlLock(false);
    lv_label_set_text(labelStatus, "READY");
    lv_obj_set_style_text_color(labelStatus,
                                lv_palette_main(LV_PALETTE_GREEN), 0);

    lv_timer_create(motorTimerEvent, 20, nullptr);
}
