#include "ui_engine.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static lv_obj_t *s_scr;
static ui_view_mode_t s_current_view = VIEW_SINE_WAVE_TRANSDUCER;

// View Containers
static lv_obj_t *s_view_objs[VIEW_MAX_COUNT];

// View 1: Multi-Chromatic Sine Wave Transducer (Hero View)
static lv_obj_t *s_sine_chart;
static lv_chart_series_t *s_sine_series_cyan;  // Cyan (Trace 1)
static lv_chart_series_t *s_sine_series_green; // Lime Green (Trace 2)
static lv_chart_series_t *s_sine_series_red;   // Hot Red/Pink (Trace 3)
static lv_obj_t *s_sine_bold_rpm_label;
static lv_obj_t *s_sine_sub_label;
static lv_obj_t *s_sine_rms_pill;
static lv_obj_t *s_sine_kurt_pill;

// View 2: 24-Band FFT Harmonic Spectrum
static lv_obj_t *s_fft_peak_label;
static lv_obj_t *s_fft_bars[24];

// View 3: Kurtosis & Impulse Shock History
static lv_obj_t *s_kurt_chart;
static lv_chart_series_t *s_kurt_series;
static lv_obj_t *s_kurt_bold_val;
static lv_obj_t *s_kurt_status_label;

// View 4: Triple Activity Rings & Health Score
static lv_obj_t *s_ring_red;    // Outer Red (Health)
static lv_obj_t *s_ring_green;  // Middle Green (Bearing)
static lv_obj_t *s_ring_blue;   // Inner Blue (Balance)
static lv_obj_t *s_ring_score_label;

// View 5: Bold Diagnostics & Asset Overview
static lv_obj_t *s_diag_progress_arc;
static lv_obj_t *s_diag_bold_score;
static lv_obj_t *s_diag_bold_title;
static lv_obj_t *s_diag_recom_label;

// Shared Pagination Dots (5 dots at bottom: ● ○ ○ ○ ○)
static lv_obj_t *s_nav_dots[VIEW_MAX_COUNT];

static void btn_next_view_cb(lv_event_t *e) {
    (void)e;
    ui_engine_next_view();
}

static void btn_calib_cb(lv_event_t *e) {
    (void)e;
    dsp_engine_start_calibration(60);
}

void ui_engine_set_view(ui_view_mode_t view) {
    if (view >= VIEW_MAX_COUNT) view = VIEW_SINE_WAVE_TRANSDUCER;
    s_current_view = view;

    for (int i = 0; i < VIEW_MAX_COUNT; i++) {
        if (s_view_objs[i]) {
            if (i == s_current_view) {
                lv_obj_clear_flag(s_view_objs[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(s_view_objs[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
        // Update pagination dots
        if (s_nav_dots[i]) {
            if (i == s_current_view) {
                lv_obj_set_style_bg_color(s_nav_dots[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_width(s_nav_dots[i], 16, LV_PART_MAIN);
            } else {
                lv_obj_set_style_bg_color(s_nav_dots[i], lv_color_hex(0x3A4150), LV_PART_MAIN);
                lv_obj_set_style_width(s_nav_dots[i], 6, LV_PART_MAIN);
            }
        }
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
// VIEW 1: MULTI-CHROMATIC SINE WAVE TRANSDUCER (Hero Default)
// -------------------------------------------------------------
static void create_view_sine_wave(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 388);
    lv_obj_set_pos(v, 6, 6);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x0C0E14), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x1F2430), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 28, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 10, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_SINE_WAVE_TRANSDUCER] = v;

    // Header: ‹ Vibration Stream 🔊
    lv_obj_t *top_row = lv_obj_create(v);
    lv_obj_set_size(top_row, 334, 32);
    lv_obj_set_pos(top_row, 0, 0);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(top_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(top_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(top_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_title = lv_label_create(top_row);
    lv_label_set_text(lbl_title, "‹  Vibration Stream");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 4, 0);

    lv_obj_t *lbl_audio = lv_label_create(top_row);
    lv_label_set_text(lbl_audio, "250Hz ●");
    lv_obj_set_style_text_color(lbl_audio, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_audio, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_audio, LV_ALIGN_RIGHT_MID, -4, 0);

    // Multi-Chromatic Glowing Sine Waves
    s_sine_chart = lv_chart_create(v);
    lv_obj_set_size(s_sine_chart, 334, 140);
    lv_obj_set_pos(s_sine_chart, 0, 36);
    lv_chart_set_type(s_sine_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_sine_chart, 54);
    lv_chart_set_range(s_sine_chart, LV_CHART_AXIS_PRIMARY_Y, -120, 120);
    lv_obj_set_style_bg_opa(s_sine_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_sine_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_sine_chart, 3, LV_PART_ITEMS);

    s_sine_series_cyan  = lv_chart_add_series(s_sine_chart, lv_color_hex(0x00F0FF), LV_CHART_AXIS_PRIMARY_Y); // Neon Cyan (X)
    s_sine_series_green = lv_chart_add_series(s_sine_chart, lv_color_hex(0x00FF66), LV_CHART_AXIS_PRIMARY_Y); // Lime Green (Y)
    s_sine_series_red   = lv_chart_add_series(s_sine_chart, lv_color_hex(0xFF2A54), LV_CHART_AXIS_PRIMARY_Y); // Hot Red/Pink (Z)

    // Center Big Bold Readout (Like "3402" in Steps design)
    s_sine_bold_rpm_label = lv_label_create(v);
    lv_label_set_text(s_sine_bold_rpm_label, "2,910");
    lv_obj_set_style_text_color(s_sine_bold_rpm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_bold_rpm_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_sine_bold_rpm_label, 12, 184);

    s_sine_sub_label = lv_label_create(v);
    lv_label_set_text(s_sine_sub_label, "Rotor RPM · 48.5 Hz");
    lv_obj_set_style_text_color(s_sine_sub_label, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_sub_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_sine_sub_label, 12, 216);

    // Pill Metrics (RMS & Kurtosis badges like flame/calorie pills)
    lv_obj_t *pills_row = lv_obj_create(v);
    lv_obj_set_size(pills_row, 334, 40);
    lv_obj_set_pos(pills_row, 0, 244);
    lv_obj_set_style_bg_opa(pills_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(pills_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(pills_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(pills_row, LV_OBJ_FLAG_SCROLLABLE);

    s_sine_rms_pill = lv_label_create(pills_row);
    lv_label_set_text(s_sine_rms_pill, "⚡ 0.082g RMS");
    lv_obj_set_style_text_color(s_sine_rms_pill, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_rms_pill, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(s_sine_rms_pill, LV_ALIGN_LEFT_MID, 8, 0);

    s_sine_kurt_pill = lv_label_create(pills_row);
    lv_label_set_text(s_sine_kurt_pill, "🔥 Kurt: 2.94");
    lv_obj_set_style_text_color(s_sine_kurt_pill, lv_color_hex(0xFF2A54), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_kurt_pill, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(s_sine_kurt_pill, LV_ALIGN_RIGHT_MID, -8, 0);

    // Capsule Action Button: "Calibrate & Details"
    lv_obj_t *btn_details = lv_button_create(v);
    lv_obj_set_size(btn_details, 334, 44);
    lv_obj_set_pos(btn_details, 0, 296);
    lv_obj_set_style_bg_color(btn_details, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_radius(btn_details, 22, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_details, btn_calib_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_btn = lv_label_create(btn_details);
    lv_label_set_text(lbl_btn, "Calibrate Physical Baseline");
    lv_obj_set_style_text_color(lbl_btn, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_btn, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(lbl_btn);
}

// -------------------------------------------------------------
// VIEW 2: 24-BAND FFT SPECTRUM EQUALIZER
// -------------------------------------------------------------
static void create_view_fft(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 388);
    lv_obj_set_pos(v, 6, 6);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x0C0E14), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x1F2430), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 28, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 10, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_FFT_SPECTRUM] = v;

    // Header
    lv_obj_t *top_row = lv_obj_create(v);
    lv_obj_set_size(top_row, 334, 32);
    lv_obj_set_pos(top_row, 0, 0);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(top_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(top_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(top_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_title = lv_label_create(top_row);
    lv_label_set_text(lbl_title, "‹  FFT Harmonics (0-500Hz)");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 4, 0);

    s_fft_peak_label = lv_label_create(top_row);
    lv_label_set_text(s_fft_peak_label, "1X: 48.5Hz");
    lv_obj_set_style_text_color(s_fft_peak_label, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_fft_peak_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(s_fft_peak_label, LV_ALIGN_RIGHT_MID, -4, 0);

    // 24 Equalizer Bars Container
    lv_obj_t *eq_box = lv_obj_create(v);
    lv_obj_set_size(eq_box, 334, 210);
    lv_obj_set_pos(eq_box, 0, 40);
    lv_obj_set_style_bg_color(eq_box, lv_color_hex(0x12151E), LV_PART_MAIN);
    lv_obj_set_style_border_color(eq_box, lv_color_hex(0x202634), LV_PART_MAIN);
    lv_obj_set_style_border_width(eq_box, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(eq_box, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_all(eq_box, 6, LV_PART_MAIN);
    lv_obj_clear_flag(eq_box, LV_OBJ_FLAG_SCROLLABLE);

    int bar_w = 8;
    int spacing = 5;
    for (int i = 0; i < 24; i++) {
        lv_obj_t *bar = lv_bar_create(eq_box);
        lv_obj_set_size(bar, bar_w, 180);
        lv_obj_set_pos(bar, 6 + i * (bar_w + spacing), 10);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, (i == 4) ? 88 : (i == 8) ? 42 : 12 + (i % 6) * 8, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x1B202D), LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
        uint32_t c = (i > 18) ? 0xFF2A54 : (i > 12) ? 0xF59E0B : (i > 6) ? 0xF5C544 : 0x00F0FF;
        lv_obj_set_style_bg_color(bar, lv_color_hex(c), LV_PART_INDICATOR);
        s_fft_bars[i] = bar;
    }

    // Bottom Stats
    lv_obj_t *lbl_info = lv_label_create(v);
    lv_label_set_text(lbl_info, "Fundamental F0 = 48.5 Hz | 2X = 97.0 Hz | 3X = 145.5 Hz");
    lv_obj_set_style_text_color(lbl_info, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_info, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_info, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// -------------------------------------------------------------
// VIEW 3: KURTOSIS & SHOCK SPIKE ANALYSIS
// -------------------------------------------------------------
static void create_view_kurtosis(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 388);
    lv_obj_set_pos(v, 6, 6);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x0C0E14), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x1F2430), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 28, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 10, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_KURTOSIS_IMPACT] = v;

    // Header
    lv_obj_t *lbl_title = lv_label_create(v);
    lv_label_set_text(lbl_title, "‹  Kurtosis Shock Analysis");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_LEFT, 4, 4);

    // Large Bold Kurtosis Readout
    s_kurt_bold_val = lv_label_create(v);
    lv_label_set_text(s_kurt_bold_val, "2.94");
    lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_bold_val, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_kurt_bold_val, 12, 40);

    s_kurt_status_label = lv_label_create(v);
    lv_label_set_text(s_kurt_status_label, "Gaussian Normal (Zero Shock Spalling)");
    lv_obj_set_style_text_color(s_kurt_status_label, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_status_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_kurt_status_label, 12, 70);

    // Kurtosis History Timeline Chart
    s_kurt_chart = lv_chart_create(v);
    lv_obj_set_size(s_kurt_chart, 334, 170);
    lv_obj_set_pos(s_kurt_chart, 0, 100);
    lv_chart_set_type(s_kurt_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_kurt_chart, 40);
    lv_chart_set_range(s_kurt_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_color(s_kurt_chart, lv_color_hex(0x12151E), LV_PART_MAIN);
    lv_obj_set_style_border_color(s_kurt_chart, lv_color_hex(0x202634), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_kurt_chart, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(s_kurt_chart, 16, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_kurt_chart, 3, LV_PART_ITEMS);

    s_kurt_series = lv_chart_add_series(s_kurt_chart, lv_color_hex(0x00FF66), LV_CHART_AXIS_PRIMARY_Y);

    lv_obj_t *lbl_thresh = lv_label_create(v);
    lv_label_set_text(lbl_thresh, "Threshold: Kurtosis > 4.0 indicates bearing race micro-cracks");
    lv_obj_set_style_text_color(lbl_thresh, lv_color_hex(0xF59E0B), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_thresh, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_thresh, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// -------------------------------------------------------------
// VIEW 4: TRIPLE ACTIVITY RINGS (Apple Watch Activity Style)
// -------------------------------------------------------------
static void create_view_rings(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 388);
    lv_obj_set_pos(v, 6, 6);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x0C0E14), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x1F2430), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 28, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 10, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_TRIPLE_ACTIVITY_RINGS] = v;

    lv_obj_t *lbl_title = lv_label_create(v);
    lv_label_set_text(lbl_title, "Activity & Health Rings");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 4);

    // Triple Rings Container
    lv_obj_t *ring_box = lv_obj_create(v);
    lv_obj_set_size(ring_box, 240, 240);
    lv_obj_center(ring_box);
    lv_obj_set_style_bg_opa(ring_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(ring_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ring_box, LV_OBJ_FLAG_SCROLLABLE);

    // Ring 1 (Outer Red: Health)
    s_ring_red = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_red, 220, 220);
    lv_obj_center(s_ring_red);
    lv_arc_set_bg_angles(s_ring_red, 0, 360);
    lv_arc_set_range(s_ring_red, 0, 100);
    lv_arc_set_value(s_ring_red, 98);
    lv_obj_set_style_arc_width(s_ring_red, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_red, lv_color_hex(0x38121C), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_red, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_red, lv_color_hex(0xFF2A54), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_red, LV_OBJ_FLAG_CLICKABLE);

    // Ring 2 (Middle Green: Bearing)
    s_ring_green = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_green, 176, 176);
    lv_obj_center(s_ring_green);
    lv_arc_set_bg_angles(s_ring_green, 0, 360);
    lv_arc_set_range(s_ring_green, 0, 100);
    lv_arc_set_value(s_ring_green, 92);
    lv_obj_set_style_arc_width(s_ring_green, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_green, lv_color_hex(0x0C331E), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_green, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_green, lv_color_hex(0x00FF66), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_green, LV_OBJ_FLAG_CLICKABLE);

    // Ring 3 (Inner Blue: Balance)
    s_ring_blue = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_blue, 132, 132);
    lv_obj_center(s_ring_blue);
    lv_arc_set_bg_angles(s_ring_blue, 0, 360);
    lv_arc_set_range(s_ring_blue, 0, 100);
    lv_arc_set_value(s_ring_blue, 95);
    lv_obj_set_style_arc_width(s_ring_blue, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_blue, lv_color_hex(0x0E243A), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_blue, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_blue, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_blue, LV_OBJ_FLAG_CLICKABLE);

    s_ring_score_label = lv_label_create(ring_box);
    lv_label_set_text(s_ring_score_label, "98%");
    lv_obj_set_style_text_color(s_ring_score_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_ring_score_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(s_ring_score_label);

    lv_obj_t *lbl_legend = lv_label_create(v);
    lv_label_set_text(lbl_legend, "● Health (98%)  ● Bearing (Good)  ● Balance (Optimal)");
    lv_obj_set_style_text_color(lbl_legend, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_legend, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(lbl_legend, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// -------------------------------------------------------------
// VIEW 5: BOLD DIAGNOSTICS & ISO 10816 EVALUATION
// -------------------------------------------------------------
static void create_view_diagnostics(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 388);
    lv_obj_set_pos(v, 6, 6);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x0C0E14), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x1F2430), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 28, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 10, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_BOLD_DIAGNOSTICS] = v;

    // Cyan Progress Arc
    s_diag_progress_arc = lv_arc_create(v);
    lv_obj_set_size(s_diag_progress_arc, 160, 160);
    lv_obj_align(s_diag_progress_arc, LV_ALIGN_TOP_MID, 0, 10);
    lv_arc_set_bg_angles(s_diag_progress_arc, 135, 405);
    lv_arc_set_angles(s_diag_progress_arc, 135, 405);
    lv_arc_set_range(s_diag_progress_arc, 0, 100);
    lv_arc_set_value(s_diag_progress_arc, 98);
    lv_obj_set_style_arc_width(s_diag_progress_arc, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_diag_progress_arc, lv_color_hex(0x1F2430), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_diag_progress_arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_diag_progress_arc, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_diag_progress_arc, LV_OBJ_FLAG_CLICKABLE);

    s_diag_bold_score = lv_label_create(v);
    lv_label_set_text(s_diag_bold_score, "98%");
    lv_obj_set_style_text_color(s_diag_bold_score, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_bold_score, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_diag_bold_score, s_diag_progress_arc, LV_ALIGN_CENTER, 0, -4);

    // Diagnosis Text Card
    lv_obj_t *diag_box = lv_obj_create(v);
    lv_obj_set_size(diag_box, 334, 130);
    lv_obj_set_pos(diag_box, 0, 184);
    lv_obj_set_style_bg_color(diag_box, lv_color_hex(0x12151E), LV_PART_MAIN);
    lv_obj_set_style_border_color(diag_box, lv_color_hex(0x202634), LV_PART_MAIN);
    lv_obj_set_style_border_width(diag_box, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(diag_box, 16, LV_PART_MAIN);
    lv_obj_clear_flag(diag_box, LV_OBJ_FLAG_SCROLLABLE);

    s_diag_bold_title = lv_label_create(diag_box);
    lv_label_set_text(s_diag_bold_title, "NOMINAL HARMONIC");
    lv_obj_set_style_text_color(s_diag_bold_title, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_bold_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_diag_bold_title, LV_ALIGN_TOP_MID, 0, 8);

    s_diag_recom_label = lv_label_create(diag_box);
    lv_label_set_text(s_diag_recom_label, "Machine is operating within ISO 10816 Class A. Harmonic vibration signatures show optimal rotor balance and zero bearing spalling.");
    lv_label_set_long_mode(s_diag_recom_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_diag_recom_label, 300);
    lv_obj_set_style_text_color(s_diag_recom_label, lv_color_hex(0x9CA3AF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_recom_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_diag_recom_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_diag_recom_label, LV_ALIGN_CENTER, 0, 16);
}

void ui_engine_init(void) {
    s_scr = lv_screen_active();
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. CREATE ALL 5 VIEW CONTAINERS
    create_view_sine_wave(s_scr);
    create_view_fft(s_scr);
    create_view_kurtosis(s_scr);
    create_view_rings(s_scr);
    create_view_diagnostics(s_scr);

    // 2. BOTTOM PAGINATION DOTS (● ○ ○ ○ ○) at y: 412
    lv_obj_t *dots_container = lv_obj_create(s_scr);
    lv_obj_set_size(dots_container, 356, 30);
    lv_obj_set_pos(dots_container, 6, 404);
    lv_obj_set_style_bg_opa(dots_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(dots_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dots_container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dots_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(dots_container, btn_next_view_cb, LV_EVENT_CLICKED, NULL);

    int start_x = 126;
    for (int i = 0; i < VIEW_MAX_COUNT; i++) {
        lv_obj_t *dot = lv_obj_create(dots_container);
        lv_obj_set_size(dot, (i == 0) ? 16 : 6, 6);
        lv_obj_set_pos(dot, start_x + i * 16, 12);
        lv_obj_set_style_bg_color(dot, lv_color_hex((i == 0) ? 0xFFFFFF : 0x3A4150), LV_PART_MAIN);
        lv_obj_set_style_radius(dot, 3, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        s_nav_dots[i] = dot;
    }

    // Set View 1 (Sine Wave Transducer) as initial default view
    ui_engine_set_view(VIEW_SINE_WAVE_TRANSDUCER);
}

void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count) {
    if (!metrics) return;

    char buf[64];

    // 1. UPDATE VIEW 1: SINE WAVE TRANSDUCER
    if (s_current_view == VIEW_SINE_WAVE_TRANSDUCER) {
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)metrics->estimated_rpm);
        lv_label_set_text(s_sine_bold_rpm_label, buf);

        snprintf(buf, sizeof(buf), "Rotor RPM · %.1f Hz Fundamental", metrics->peak_freq_hz);
        lv_label_set_text(s_sine_sub_label, buf);

        snprintf(buf, sizeof(buf), "⚡ %.3fg RMS", metrics->rms_acceleration_g);
        lv_label_set_text(s_sine_rms_pill, buf);

        snprintf(buf, sizeof(buf), "🔥 Kurt: %.2f", metrics->kurtosis);
        lv_label_set_text(s_sine_kurt_pill, buf);

        for (size_t i = 0; i < 54; i++) {
            size_t idx = (i * osc_count) / 54;
            int32_t val_x = (int32_t)(osc_waveform[idx] * 110.0f);
            int32_t val_y = (int32_t)(osc_waveform[(idx + 10) % osc_count] * 80.0f);
            int32_t val_z = (int32_t)(osc_waveform[(idx + 20) % osc_count] * 95.0f);
            lv_chart_set_value_by_id(s_sine_chart, s_sine_series_cyan, i, val_x);
            lv_chart_set_value_by_id(s_sine_chart, s_sine_series_green, i, val_y);
            lv_chart_set_value_by_id(s_sine_chart, s_sine_series_red, i, val_z);
        }
    }

    // 2. UPDATE VIEW 2: 24-BAND FFT SPECTRUM
    else if (s_current_view == VIEW_FFT_SPECTRUM) {
        snprintf(buf, sizeof(buf), "1X: %.1fHz", metrics->peak_freq_hz);
        lv_label_set_text(s_fft_peak_label, buf);

        for (int i = 0; i < 24; i++) {
            int val = (int)(metrics->visual_spectrum[i] * 100.0f);
            if (val > 100) val = 100;
            lv_bar_set_value(s_fft_bars[i], val, LV_ANIM_OFF);
        }
    }

    // 3. UPDATE VIEW 3: KURTOSIS IMPACT
    else if (s_current_view == VIEW_KURTOSIS_IMPACT) {
        snprintf(buf, sizeof(buf), "%.2f", metrics->kurtosis);
        lv_label_set_text(s_kurt_bold_val, buf);

        if (metrics->kurtosis > 4.5f) {
            lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0xFF2A54), LV_PART_MAIN);
            lv_label_set_text(s_kurt_status_label, "IMPACT SHOCK DETECTED (Bearing Spalling)");
        } else if (metrics->kurtosis > 3.6f) {
            lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0xF59E0B), LV_PART_MAIN);
            lv_label_set_text(s_kurt_status_label, "Elevated Kurtosis Warning");
        } else {
            lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0x00FF66), LV_PART_MAIN);
            lv_label_set_text(s_kurt_status_label, "Gaussian Normal (Optimal Health)");
        }

        for (size_t i = 0; i < 40; i++) {
            size_t idx = (i * osc_count) / 40;
            int32_t val = (int32_t)(fabsf(osc_waveform[idx]) * 100.0f);
            lv_chart_set_value_by_id(s_kurt_chart, s_kurt_series, i, val);
        }
    }

    // 4. UPDATE VIEW 4: TRIPLE ACTIVITY RINGS
    else if (s_current_view == VIEW_TRIPLE_ACTIVITY_RINGS) {
        lv_arc_set_value(s_ring_red, metrics->health_score);
        int bearing_val = (metrics->kurtosis > 5.0f) ? 30 : (metrics->kurtosis > 3.8f) ? 65 : 95;
        lv_arc_set_value(s_ring_green, bearing_val);
        int balance_val = (metrics->rms_acceleration_g > 0.3f) ? 25 : (metrics->rms_acceleration_g > 0.15f) ? 60 : 98;
        lv_arc_set_value(s_ring_blue, balance_val);

        snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
        lv_label_set_text(s_ring_score_label, buf);
    }

    // 5. UPDATE VIEW 5: BOLD DIAGNOSTICS
    else if (s_current_view == VIEW_BOLD_DIAGNOSTICS) {
        lv_arc_set_value(s_diag_progress_arc, metrics->health_score);
        snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
        lv_label_set_text(s_diag_bold_score, buf);
        lv_label_set_text(s_diag_bold_title, metrics->diagnosis_text);
        lv_label_set_text(s_diag_recom_label, metrics->recommendation);
    }
}
