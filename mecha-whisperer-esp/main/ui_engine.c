#include "ui_engine.h"
#include <stdio.h>
#include <string.h>

static lv_obj_t *s_scr;
static ui_view_mode_t s_current_view = VIEW_ACOUSTIC_ECG_VU;

// Top Shared Header
static lv_obj_t *s_header_label;
static lv_obj_t *s_view_pill_label;

// View Containers
static lv_obj_t *s_view_objs[VIEW_MAX_COUNT];

// View 1: Acoustic ECG & Segmented VU Meter
static lv_obj_t *s_ecg_chart;
static lv_chart_series_t *s_ecg_series;
static lv_obj_t *s_ecg_bpm_label;
static lv_obj_t *s_vu_bars[16];

// View 2: Vibration Transducer (Dual Spline + Harmonic Goal Bars)
static lv_obj_t *s_spline_chart;
static lv_chart_series_t *s_spline_series_x;
static lv_chart_series_t *s_spline_series_y;
static lv_obj_t *s_goal_bars[7];

// View 3: Gyroscope Dynamics (Tachometer Arc + Angular Rate)
static lv_obj_t *s_tacho_arc;
static lv_obj_t *s_tacho_rpm_label;
static lv_obj_t *s_gyro_chart;
static lv_chart_series_t *s_gyro_series;

// View 4: Triple Activity Rings & Matrix Heatmap
static lv_obj_t *s_ring_outer;  // Pink (Health)
static lv_obj_t *s_ring_middle; // Blue (Bearing)
static lv_obj_t *s_ring_inner;  // Yellow (Balance)
static lv_obj_t *s_ring_center_label;

// View 5: Bold Typography & Diagnostics Overview
static lv_obj_t *s_diag_arc;
static lv_obj_t *s_bold_metric_label;
static lv_obj_t *s_bold_diag_label;
static lv_obj_t *s_bold_recom_label;

static void btn_next_view_cb(lv_event_t *e) {
    (void)e;
    ui_engine_next_view();
}

static void btn_calib_cb(lv_event_t *e) {
    (void)e;
    dsp_engine_start_calibration(60);
}

void ui_engine_set_view(ui_view_mode_t view) {
    if (view >= VIEW_MAX_COUNT) view = VIEW_ACOUSTIC_ECG_VU;
    s_current_view = view;

    for (int i = 0; i < VIEW_MAX_COUNT; i++) {
        if (s_view_objs[i]) {
            if (i == s_current_view) {
                lv_obj_clear_flag(s_view_objs[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(s_view_objs[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    const char *titles[] = {
        "1/5: ACOUSTIC ECG & VU",
        "2/5: VIBRATION SPLINE",
        "3/5: ROTATION TACHO",
        "4/5: TRIPLE RINGS",
        "5/5: BOLD DIAGNOSTICS"
    };

    if (s_view_pill_label) {
        lv_label_set_text(s_view_pill_label, titles[s_current_view]);
    }
}

void ui_engine_next_view(void) {
    ui_view_mode_t next = (s_current_view + 1) % VIEW_MAX_COUNT;
    ui_engine_set_view(next);
}

ui_view_mode_t ui_engine_get_view(void) {
    return s_current_view;
}

// -------------------------------------------------------------
// VIEW 1: ACOUSTIC ECG PULSE & SEGMENTED VU METER (Chart 8 & Chart 3)
// -------------------------------------------------------------
static void create_view_acoustic(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 356);
    lv_obj_set_pos(v, 6, 44);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x10131A), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_ACOUSTIC_ECG_VU] = v;

    // Top Card: Neon Pink Cardiac / Acoustic Pulse
    lv_obj_t *card_top = lv_obj_create(v);
    lv_obj_set_size(card_top, 340, 160);
    lv_obj_set_pos(card_top, 0, 0);
    lv_obj_set_style_bg_color(card_top, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_top, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_top, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_top, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_top, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_ecg = lv_label_create(card_top);
    lv_label_set_text(lbl_ecg, "ACOUSTIC STETHOSCOPE PULSE");
    lv_obj_set_style_text_color(lbl_ecg, lv_color_hex(0xFF4081), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_ecg, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_ecg, LV_ALIGN_TOP_LEFT, 6, 2);

    s_ecg_bpm_label = lv_label_create(card_top);
    lv_label_set_text(s_ecg_bpm_label, "48.5 Hz");
    lv_obj_set_style_text_color(s_ecg_bpm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_ecg_bpm_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(s_ecg_bpm_label, LV_ALIGN_BOTTOM_LEFT, 6, -2);

    s_ecg_chart = lv_chart_create(card_top);
    lv_obj_set_size(s_ecg_chart, 324, 100);
    lv_obj_set_pos(s_ecg_chart, 0, 24);
    lv_chart_set_type(s_ecg_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_ecg_chart, 48);
    lv_chart_set_range(s_ecg_chart, LV_CHART_AXIS_PRIMARY_Y, -100, 100);
    lv_obj_set_style_bg_opa(s_ecg_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_ecg_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_ecg_chart, 3, LV_PART_ITEMS);
    s_ecg_series = lv_chart_add_series(s_ecg_chart, lv_color_hex(0xFF2E7E), LV_CHART_AXIS_PRIMARY_Y);

    // Bottom Card: Multi-Segment LED VU Meter
    lv_obj_t *card_bot = lv_obj_create(v);
    lv_obj_set_size(card_bot, 340, 168);
    lv_obj_set_pos(card_bot, 0, 168);
    lv_obj_set_style_bg_color(card_bot, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_bot, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_bot, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_bot, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_bot, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_vu = lv_label_create(card_bot);
    lv_label_set_text(lbl_vu, "SEGMENTED SPECTRAL VU EQUALIZER");
    lv_obj_set_style_text_color(lbl_vu, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_vu, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_vu, LV_ALIGN_TOP_LEFT, 6, 2);

    int bar_w = 14;
    int spacing = 6;
    for (int i = 0; i < 16; i++) {
        lv_obj_t *bar = lv_bar_create(card_bot);
        lv_obj_set_size(bar, bar_w, 120);
        lv_obj_set_pos(bar, 6 + i * (bar_w + spacing), 28);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 20 + (i % 4) * 18, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x232936), LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_hex((i > 11) ? 0xF43F5E : (i > 7) ? 0xF59E0B : 0xF5C544), LV_PART_INDICATOR);
        s_vu_bars[i] = bar;
    }
}

// -------------------------------------------------------------
// VIEW 2: VIBRATION DUAL SPLINE & GOAL PILLARS (Chart 9 & Chart 1)
// -------------------------------------------------------------
static void create_view_vibration(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 356);
    lv_obj_set_pos(v, 6, 44);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x10131A), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_VIBRATION_TRANSDUCER] = v;

    // Dual Spline Curve
    lv_obj_t *card_top = lv_obj_create(v);
    lv_obj_set_size(card_top, 340, 160);
    lv_obj_set_pos(card_top, 0, 0);
    lv_obj_set_style_bg_color(card_top, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_top, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_top, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_top, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_top, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_spline = lv_label_create(card_top);
    lv_label_set_text(lbl_spline, "3-AXIS DUAL SPLINE ACCELERATION");
    lv_obj_set_style_text_color(lbl_spline, lv_color_hex(0x38BDF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_spline, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_spline, LV_ALIGN_TOP_LEFT, 6, 2);

    s_spline_chart = lv_chart_create(card_top);
    lv_obj_set_size(s_spline_chart, 324, 114);
    lv_obj_set_pos(s_spline_chart, 0, 24);
    lv_chart_set_type(s_spline_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_spline_chart, 40);
    lv_chart_set_range(s_spline_chart, LV_CHART_AXIS_PRIMARY_Y, -100, 100);
    lv_obj_set_style_bg_opa(s_spline_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_spline_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_spline_chart, 2, LV_PART_ITEMS);

    s_spline_series_x = lv_chart_add_series(s_spline_chart, lv_color_hex(0xF43F5E), LV_CHART_AXIS_PRIMARY_Y); // Red Max Trace
    s_spline_series_y = lv_chart_add_series(s_spline_chart, lv_color_hex(0x38BDF8), LV_CHART_AXIS_PRIMARY_Y); // Blue Min Trace

    // Bottom Card: Vertical Goal Pillars
    lv_obj_t *card_bot = lv_obj_create(v);
    lv_obj_set_size(card_bot, 340, 168);
    lv_obj_set_pos(card_bot, 0, 168);
    lv_obj_set_style_bg_color(card_bot, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_bot, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_bot, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_bot, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_bot, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_bars = lv_label_create(card_bot);
    lv_label_set_text(lbl_bars, "HARMONIC ENERGY PILLARS (1X-7X)");
    lv_obj_set_style_text_color(lbl_bars, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_bars, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_bars, LV_ALIGN_TOP_LEFT, 6, 2);

    int p_w = 24;
    int p_gap = 12;
    for (int i = 0; i < 7; i++) {
        lv_obj_t *p_bar = lv_bar_create(card_bot);
        lv_obj_set_size(p_bar, p_w, 110);
        lv_obj_set_pos(p_bar, 12 + i * (p_w + p_gap), 32);
        lv_bar_set_range(p_bar, 0, 100);
        lv_bar_set_value(p_bar, (i == 3) ? 85 : 30 + (i * 8), LV_ANIM_OFF);
        lv_obj_set_style_bg_color(p_bar, lv_color_hex(0x232936), LV_PART_MAIN);
        lv_obj_set_style_radius(p_bar, 12, LV_PART_MAIN);
        lv_obj_set_style_radius(p_bar, 12, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(p_bar, lv_color_hex((i == 3) ? 0xF5C544 : 0xEA580C), LV_PART_INDICATOR);
        s_goal_bars[i] = p_bar;
    }
}

// -------------------------------------------------------------
// VIEW 3: GYROSCOPE DYNAMICS & TACHOMETER (Chart 6)
// -------------------------------------------------------------
static void create_view_gyro(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 356);
    lv_obj_set_pos(v, 6, 44);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x10131A), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_GYRO_TACHOMETER] = v;

    // Top: Gradient Speedometer Tachometer
    lv_obj_t *card_top = lv_obj_create(v);
    lv_obj_set_size(card_top, 340, 180);
    lv_obj_set_pos(card_top, 0, 0);
    lv_obj_set_style_bg_color(card_top, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_top, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_top, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_top, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_top, LV_OBJ_FLAG_SCROLLABLE);

    s_tacho_arc = lv_arc_create(card_top);
    lv_obj_set_size(s_tacho_arc, 150, 150);
    lv_obj_center(s_tacho_arc);
    lv_arc_set_bg_angles(s_tacho_arc, 135, 405);
    lv_arc_set_angles(s_tacho_arc, 135, 405);
    lv_arc_set_range(s_tacho_arc, 0, 4500);
    lv_arc_set_value(s_tacho_arc, 2910);
    lv_obj_set_style_arc_width(s_tacho_arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_tacho_arc, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_tacho_arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_tacho_arc, lv_color_hex(0xF59E0B), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_tacho_arc, LV_OBJ_FLAG_CLICKABLE);

    s_tacho_rpm_label = lv_label_create(card_top);
    lv_label_set_text(s_tacho_rpm_label, "2,910\nRPM");
    lv_obj_set_style_text_align(s_tacho_rpm_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_tacho_rpm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_tacho_rpm_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(s_tacho_rpm_label);

    // Bottom: 3-Axis Gyro Angular Rate
    lv_obj_t *card_bot = lv_obj_create(v);
    lv_obj_set_size(card_bot, 340, 148);
    lv_obj_set_pos(card_bot, 0, 188);
    lv_obj_set_style_bg_color(card_bot, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_bot, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_bot, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_bot, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_bot, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_gyro = lv_label_create(card_bot);
    lv_label_set_text(lbl_gyro, "GYROSCOPE ANGULAR WOBBLE");
    lv_obj_set_style_text_color(lbl_gyro, lv_color_hex(0x10B981), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_gyro, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_gyro, LV_ALIGN_TOP_LEFT, 6, 2);

    s_gyro_chart = lv_chart_create(card_bot);
    lv_obj_set_size(s_gyro_chart, 324, 100);
    lv_obj_set_pos(s_gyro_chart, 0, 24);
    lv_chart_set_type(s_gyro_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_gyro_chart, 40);
    lv_chart_set_range(s_gyro_chart, LV_CHART_AXIS_PRIMARY_Y, -80, 80);
    lv_obj_set_style_bg_opa(s_gyro_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_gyro_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_gyro_chart, 2, LV_PART_ITEMS);
    s_gyro_series = lv_chart_add_series(s_gyro_chart, lv_color_hex(0x10B981), LV_CHART_AXIS_PRIMARY_Y);
}

// -------------------------------------------------------------
// VIEW 4: TRIPLE ACTIVITY RINGS (Chart 4)
// -------------------------------------------------------------
static void create_view_rings_matrix(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 356);
    lv_obj_set_pos(v, 6, 44);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x10131A), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_TRIPLE_RINGS_MATRIX] = v;

    // Concentric Activity Rings
    lv_obj_t *card_top = lv_obj_create(v);
    lv_obj_set_size(card_top, 340, 336);
    lv_obj_center(card_top);
    lv_obj_set_style_bg_color(card_top, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_top, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_top, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_top, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_top, LV_OBJ_FLAG_SCROLLABLE);

    // Ring 1 (Outer Pink: Health)
    s_ring_outer = lv_arc_create(card_top);
    lv_obj_set_size(s_ring_outer, 220, 220);
    lv_obj_center(s_ring_outer);
    lv_arc_set_bg_angles(s_ring_outer, 0, 360);
    lv_arc_set_range(s_ring_outer, 0, 100);
    lv_arc_set_value(s_ring_outer, 98);
    lv_obj_set_style_arc_width(s_ring_outer, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_outer, lv_color_hex(0x3B1828), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_outer, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_outer, lv_color_hex(0xFA2C56), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_outer, LV_OBJ_FLAG_CLICKABLE);

    // Ring 2 (Middle Blue: Bearing)
    s_ring_middle = lv_arc_create(card_top);
    lv_obj_set_size(s_ring_middle, 180, 180);
    lv_obj_center(s_ring_middle);
    lv_arc_set_bg_angles(s_ring_middle, 0, 360);
    lv_arc_set_range(s_ring_middle, 0, 100);
    lv_arc_set_value(s_ring_middle, 92);
    lv_obj_set_style_arc_width(s_ring_middle, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_middle, lv_color_hex(0x102844), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_middle, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_middle, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_middle, LV_OBJ_FLAG_CLICKABLE);

    // Ring 3 (Inner Yellow: Balance)
    s_ring_inner = lv_arc_create(card_top);
    lv_obj_set_size(s_ring_inner, 140, 140);
    lv_obj_center(s_ring_inner);
    lv_arc_set_bg_angles(s_ring_inner, 0, 360);
    lv_arc_set_range(s_ring_inner, 0, 100);
    lv_arc_set_value(s_ring_inner, 95);
    lv_obj_set_style_arc_width(s_ring_inner, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_inner, lv_color_hex(0x3B3210), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_inner, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_inner, lv_color_hex(0xF5C544), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_inner, LV_OBJ_FLAG_CLICKABLE);

    s_ring_center_label = lv_label_create(card_top);
    lv_label_set_text(s_ring_center_label, "98%");
    lv_obj_set_style_text_color(s_ring_center_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_ring_center_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(s_ring_center_label);
}

// -------------------------------------------------------------
// VIEW 5: BOLD TYPOGRAPHY & DIAGNOSTIC RECOMMENDATION (Chart 10 & Chart 5)
// -------------------------------------------------------------
static void create_view_bold_diagnostics(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 356);
    lv_obj_set_pos(v, 6, 44);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x10131A), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_BOLD_DIAGNOSTICS] = v;

    // Giant Progress Arc (Chart 5)
    s_diag_arc = lv_arc_create(v);
    lv_obj_set_size(s_diag_arc, 160, 160);
    lv_obj_align(s_diag_arc, LV_ALIGN_TOP_MID, 0, 6);
    lv_arc_set_bg_angles(s_diag_arc, 135, 405);
    lv_arc_set_angles(s_diag_arc, 135, 405);
    lv_arc_set_range(s_diag_arc, 0, 100);
    lv_arc_set_value(s_diag_arc, 98);
    lv_obj_set_style_arc_width(s_diag_arc, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_diag_arc, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_diag_arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_diag_arc, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_diag_arc, LV_OBJ_FLAG_CLICKABLE);

    s_bold_metric_label = lv_label_create(v);
    lv_label_set_text(s_bold_metric_label, "98%");
    lv_obj_set_style_text_color(s_bold_metric_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_bold_metric_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_bold_metric_label, s_diag_arc, LV_ALIGN_CENTER, 0, -4);

    // Bold Giant Typography Card (Chart 10)
    lv_obj_t *card_diag = lv_obj_create(v);
    lv_obj_set_size(card_diag, 340, 160);
    lv_obj_set_pos(card_diag, 0, 176);
    lv_obj_set_style_bg_color(card_diag, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_diag, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(card_diag, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card_diag, 14, LV_PART_MAIN);
    lv_obj_clear_flag(card_diag, LV_OBJ_FLAG_SCROLLABLE);

    s_bold_diag_label = lv_label_create(card_diag);
    lv_label_set_text(s_bold_diag_label, "NOMINAL HARMONIC");
    lv_obj_set_style_text_color(s_bold_diag_label, lv_color_hex(0x10B981), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_bold_diag_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(s_bold_diag_label, LV_ALIGN_TOP_MID, 0, 12);

    s_bold_recom_label = lv_label_create(card_diag);
    lv_label_set_text(s_bold_recom_label, "Machine is operating within ISO 10816 Class A. Harmonic vibration signatures show optimal rotor balance and zero bearing spalling.");
    lv_label_set_long_mode(s_bold_recom_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_bold_recom_label, 310);
    lv_obj_set_style_text_color(s_bold_recom_label, lv_color_hex(0x9CA3AF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_bold_recom_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_bold_recom_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_bold_recom_label, LV_ALIGN_CENTER, 0, 20);
}

void ui_engine_init(void) {
    s_scr = lv_screen_active();
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0x0A0D14), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. TOP HEADER (Height: 34px) - Clickable Pill to cycle views
    lv_obj_t *header = lv_obj_create(s_scr);
    lv_obj_set_size(header, 356, 34);
    lv_obj_set_pos(header, 6, 6);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(header, lv_color_hex(0x283042), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 17, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 4, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(header, btn_next_view_cb, LV_EVENT_CLICKED, NULL);

    s_header_label = lv_label_create(header);
    lv_label_set_text(s_header_label, "MECHA-WHISPERER");
    lv_obj_set_style_text_color(s_header_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_header_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(s_header_label, LV_ALIGN_LEFT_MID, 8, 0);

    s_view_pill_label = lv_label_create(header);
    lv_label_set_text(s_view_pill_label, "1/5: ACOUSTIC ECG & VU");
    lv_obj_set_style_text_color(s_view_pill_label, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_view_pill_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(s_view_pill_label, LV_ALIGN_RIGHT_MID, -8, 0);

    // 2. CREATE ALL 5 VIEWS
    create_view_acoustic(s_scr);
    create_view_vibration(s_scr);
    create_view_gyro(s_scr);
    create_view_rings_matrix(s_scr);
    create_view_bold_diagnostics(s_scr);

    // 3. BOTTOM TOUCH ACTION PILLS (y: 404, Height: 38px)
    int btn_w = 82;
    
    // View Switch Button
    lv_obj_t *btn_view = lv_button_create(s_scr);
    lv_obj_set_size(btn_view, btn_w, 36);
    lv_obj_set_pos(btn_view, 6, 404);
    lv_obj_set_style_bg_color(btn_view, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_view, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_view, 18, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_view, btn_next_view_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t1 = lv_label_create(btn_view);
    lv_label_set_text(t1, "VIEW");
    lv_obj_set_style_text_color(t1, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(t1, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t1);

    // Calibrate Button
    lv_obj_t *btn_calib = lv_button_create(s_scr);
    lv_obj_set_size(btn_calib, btn_w, 36);
    lv_obj_set_pos(btn_calib, 94, 404);
    lv_obj_set_style_bg_color(btn_calib, lv_color_hex(0x1C1F26), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_calib, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_calib, 18, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_calib, btn_calib_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t2 = lv_label_create(btn_calib);
    lv_label_set_text(t2, "CALIB");
    lv_obj_set_style_text_color(t2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(t2, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t2);

    // Demo Button
    lv_obj_t *btn_demo = lv_button_create(s_scr);
    lv_obj_set_size(btn_demo, btn_w, 36);
    lv_obj_set_pos(btn_demo, 182, 404);
    lv_obj_set_style_bg_color(btn_demo, lv_color_hex(0x232936), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_demo, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_demo, 18, LV_PART_MAIN);
    lv_obj_t *t3 = lv_label_create(btn_demo);
    lv_label_set_text(t3, "DEMO");
    lv_obj_set_style_text_color(t3, lv_color_hex(0xF59E0B), LV_PART_MAIN);
    lv_obj_set_style_text_font(t3, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t3);

    // Wi-Fi Button
    lv_obj_t *btn_wifi = lv_button_create(s_scr);
    lv_obj_set_size(btn_wifi, btn_w, 36);
    lv_obj_set_pos(btn_wifi, 270, 404);
    lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x161B24), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn_wifi, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_wifi, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_wifi, 18, LV_PART_MAIN);
    lv_obj_t *t4 = lv_label_create(btn_wifi);
    lv_label_set_text(t4, "WI-FI");
    lv_obj_set_style_text_color(t4, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(t4, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t4);

    // Initial View
    ui_engine_set_view(VIEW_ACOUSTIC_ECG_VU);
}

void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count) {
    if (!metrics) return;

    char buf[64];

    // Update View 1 (Acoustic ECG & VU)
    if (s_current_view == VIEW_ACOUSTIC_ECG_VU) {
        snprintf(buf, sizeof(buf), "%.1f Hz", metrics->peak_freq_hz);
        lv_label_set_text(s_ecg_bpm_label, buf);

        for (size_t i = 0; i < 48; i++) {
            size_t idx = (i * osc_count) / 48;
            int32_t val = (int32_t)(osc_waveform[idx] * 90.0f);
            lv_chart_set_value_by_id(s_ecg_chart, s_ecg_series, i, val);
        }

        for (int i = 0; i < 16; i++) {
            int val = (int)(metrics->visual_spectrum[i] * 100.0f);
            if (val > 100) val = 100;
            lv_bar_set_value(s_vu_bars[i], val, LV_ANIM_OFF);
        }
    }

    // Update View 2 (Vibration Spline & Goal Bars)
    else if (s_current_view == VIEW_VIBRATION_TRANSDUCER) {
        for (size_t i = 0; i < 40; i++) {
            size_t idx = (i * osc_count) / 40;
            int32_t val_x = (int32_t)(osc_waveform[idx] * 80.0f);
            int32_t val_y = (int32_t)(osc_waveform[(idx + 8) % osc_count] * 60.0f);
            lv_chart_set_value_by_id(s_spline_chart, s_spline_series_x, i, val_x);
            lv_chart_set_value_by_id(s_spline_chart, s_spline_series_y, i, val_y);
        }

        for (int i = 0; i < 7; i++) {
            int val = (int)(metrics->visual_spectrum[i * 2] * 100.0f);
            lv_bar_set_value(s_goal_bars[i], val, LV_ANIM_OFF);
        }
    }

    // Update View 3 (Gyro Tachometer & Angular Osc)
    else if (s_current_view == VIEW_GYRO_TACHOMETER) {
        lv_arc_set_value(s_tacho_arc, metrics->estimated_rpm);
        snprintf(buf, sizeof(buf), "%lu\nRPM", (unsigned long)metrics->estimated_rpm);
        lv_label_set_text(s_tacho_rpm_label, buf);

        for (size_t i = 0; i < 40; i++) {
            size_t idx = (i * osc_count) / 40;
            int32_t val = (int32_t)(osc_waveform[idx] * 50.0f);
            lv_chart_set_value_by_id(s_gyro_chart, s_gyro_series, i, val);
        }
    }

    // Update View 4 (Triple Activity Rings)
    else if (s_current_view == VIEW_TRIPLE_RINGS_MATRIX) {
        lv_arc_set_value(s_ring_outer, metrics->health_score);
        int bearing_val = (metrics->kurtosis > 5.0f) ? 35 : (metrics->kurtosis > 3.8f) ? 65 : 95;
        lv_arc_set_value(s_ring_middle, bearing_val);
        int balance_val = (metrics->rms_acceleration_g > 0.3f) ? 25 : (metrics->rms_acceleration_g > 0.15f) ? 60 : 98;
        lv_arc_set_value(s_ring_inner, balance_val);

        snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
        lv_label_set_text(s_ring_center_label, buf);
    }

    // Update View 5 (Bold Typography & Diagnostics)
    else if (s_current_view == VIEW_BOLD_DIAGNOSTICS) {
        lv_arc_set_value(s_diag_arc, metrics->health_score);
        snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
        lv_label_set_text(s_bold_metric_label, buf);
        lv_label_set_text(s_bold_diag_label, metrics->diagnosis_text);
        lv_label_set_text(s_bold_recom_label, metrics->recommendation);
    }
}
