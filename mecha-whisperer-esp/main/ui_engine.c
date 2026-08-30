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

// View 5: High-Precision Progress Arc & Severity
static lv_obj_t *s_progress_arc;
static lv_obj_t *s_progress_val_label;

// View 6: Rotational Tachometer
static lv_obj_t *s_tacho_arc;
static lv_obj_t *s_tacho_rpm_label;

// View 7: Fluid Energy Density Wave Level
static lv_obj_t *s_fluid_bar;
static lv_obj_t *s_fluid_val_label;
static lv_obj_t *s_fluid_vu_bars[16];

// View 8: Harmonic Goal Pillars (1X-7X)
static lv_obj_t *s_goal_bars[7];
static lv_obj_t *s_goal_val_labels[7];

// View 9: Spatio-Temporal Dot Matrix Heatmap
static lv_obj_t *s_matrix_dots[24];

// View 10: Bold Diagnostics & ISO 10816 Evaluation
static lv_obj_t *s_diag_bold_title;
static lv_obj_t *s_diag_recom_label;
static lv_obj_t *s_diag_iso_badge;

// Shared Navigation & Header
static lv_obj_t *s_header_pill_label;
static lv_obj_t *s_nav_dots[VIEW_MAX_COUNT];

// Smoothing state variables (Low-pass EMA filters)
static float s_smooth_rms = 0.082f;
static float s_smooth_kurt = 2.94f;
static float s_smooth_rpm = 2910.0f;
static float s_smooth_f0 = 48.5f;
static float s_sine_phase = 0.0f;

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
        if (s_nav_dots[i]) {
            if (i == s_current_view) {
                lv_obj_set_style_bg_color(s_nav_dots[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_width(s_nav_dots[i], 14, LV_PART_MAIN);
            } else {
                lv_obj_set_style_bg_color(s_nav_dots[i], lv_color_hex(0x222630), LV_PART_MAIN);
                lv_obj_set_style_width(s_nav_dots[i], 5, LV_PART_MAIN);
            }
        }
    }

    const char *titles[] = {
        "1/10: SINE WAVE TRANSDUCER",
        "2/10: 24-BAND FFT SPECTRUM",
        "3/10: KURTOSIS SHOCK",
        "4/10: TRIPLE ACTIVITY RINGS",
        "5/10: PROGRESS ARC GAUGE",
        "6/10: ROTATION TACHOMETER",
        "7/10: FLUID ENERGY DENSITY",
        "8/10: HARMONIC GOAL PILLARS",
        "9/10: 24H DOT MATRIX HEATMAP",
        "10/10: BOLD AI DIAGNOSTICS"
    };

    if (s_header_pill_label) {
        lv_label_set_text(s_header_pill_label, titles[s_current_view]);
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
// VIEW 1: SINE WAVE TRANSDUCER (Hero Default)
// -------------------------------------------------------------
static void create_view_1_sine(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_SINE_WAVE_TRANSDUCER] = v;

    // Glowing Tri-Axial Sine Waves
    s_sine_chart = lv_chart_create(v);
    lv_obj_set_size(s_sine_chart, 334, 130);
    lv_obj_set_pos(s_sine_chart, 0, 4);
    lv_chart_set_type(s_sine_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_sine_chart, 48);
    lv_chart_set_range(s_sine_chart, LV_CHART_AXIS_PRIMARY_Y, -120, 120);
    lv_obj_set_style_bg_opa(s_sine_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_sine_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_sine_chart, 3, LV_PART_ITEMS);

    s_sine_series_cyan  = lv_chart_add_series(s_sine_chart, lv_color_hex(0x00F0FF), LV_CHART_AXIS_PRIMARY_Y);
    s_sine_series_green = lv_chart_add_series(s_sine_chart, lv_color_hex(0x00FF66), LV_CHART_AXIS_PRIMARY_Y);
    s_sine_series_red   = lv_chart_add_series(s_sine_chart, lv_color_hex(0xFF2A54), LV_CHART_AXIS_PRIMARY_Y);

    // Large Bold Readout
    s_sine_bold_rpm_label = lv_label_create(v);
    lv_label_set_text(s_sine_bold_rpm_label, "2,910");
    lv_obj_set_style_text_color(s_sine_bold_rpm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_bold_rpm_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_sine_bold_rpm_label, 10, 142);

    s_sine_sub_label = lv_label_create(v);
    lv_label_set_text(s_sine_sub_label, "Rotor RPM · 48.5 Hz Fundamental");
    lv_obj_set_style_text_color(s_sine_sub_label, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_sub_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_sine_sub_label, 10, 172);

    // Metrics Row
    s_sine_rms_pill = lv_label_create(v);
    lv_label_set_text(s_sine_rms_pill, "⚡ 0.082g RMS");
    lv_obj_set_style_text_color(s_sine_rms_pill, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_rms_pill, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_sine_rms_pill, 10, 200);

    s_sine_kurt_pill = lv_label_create(v);
    lv_label_set_text(s_sine_kurt_pill, "🔥 Kurt: 2.94");
    lv_obj_set_style_text_color(s_sine_kurt_pill, lv_color_hex(0xFF2A54), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_sine_kurt_pill, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_sine_kurt_pill, 200, 200);

    // Calibrate Button
    lv_obj_t *btn_cal = lv_button_create(v);
    lv_obj_set_size(btn_cal, 334, 40);
    lv_obj_set_pos(btn_cal, 0, 236);
    lv_obj_set_style_bg_color(btn_cal, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_radius(btn_cal, 20, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_cal, btn_calib_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_btn = lv_label_create(btn_cal);
    lv_label_set_text(lbl_btn, "Calibrate Baseline Stethoscope");
    lv_obj_set_style_text_color(lbl_btn, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_btn, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(lbl_btn);
}

// -------------------------------------------------------------
// VIEW 2: 24-BAND FFT SPECTRUM
// -------------------------------------------------------------
static void create_view_2_fft(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_FFT_SPECTRUM] = v;

    s_fft_peak_label = lv_label_create(v);
    lv_label_set_text(s_fft_peak_label, "1X Peak: 48.5 Hz");
    lv_obj_set_style_text_color(s_fft_peak_label, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_fft_peak_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_fft_peak_label, 10, 8);

    lv_obj_t *eq_box = lv_obj_create(v);
    lv_obj_set_size(eq_box, 334, 210);
    lv_obj_set_pos(eq_box, 0, 36);
    lv_obj_set_style_bg_color(eq_box, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(eq_box, lv_color_hex(0x1E2330), LV_PART_MAIN);
    lv_obj_set_style_border_width(eq_box, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(eq_box, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_all(eq_box, 4, LV_PART_MAIN);
    lv_obj_clear_flag(eq_box, LV_OBJ_FLAG_SCROLLABLE);

    int bar_w = 8;
    int spacing = 5;
    for (int i = 0; i < 24; i++) {
        lv_obj_t *bar = lv_bar_create(eq_box);
        lv_obj_set_size(bar, bar_w, 180);
        lv_obj_set_pos(bar, 4 + i * (bar_w + spacing), 10);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, (i == 4) ? 88 : 12 + (i % 6) * 8, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x141822), LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
        uint32_t c = (i > 18) ? 0xFF2A54 : (i > 12) ? 0xF59E0B : (i > 6) ? 0xF5C544 : 0x00F0FF;
        lv_obj_set_style_bg_color(bar, lv_color_hex(c), LV_PART_INDICATOR);
        s_fft_bars[i] = bar;
    }
}

// -------------------------------------------------------------
// VIEW 3: KURTOSIS IMPACT
// -------------------------------------------------------------
static void create_view_3_kurtosis(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_KURTOSIS_IMPACT] = v;

    s_kurt_bold_val = lv_label_create(v);
    lv_label_set_text(s_kurt_bold_val, "2.94");
    lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_bold_val, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_kurt_bold_val, 10, 8);

    s_kurt_status_label = lv_label_create(v);
    lv_label_set_text(s_kurt_status_label, "Gaussian Symmetry (Zero Spalling)");
    lv_obj_set_style_text_color(s_kurt_status_label, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_status_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_kurt_status_label, 10, 36);

    s_kurt_chart = lv_chart_create(v);
    lv_obj_set_size(s_kurt_chart, 334, 180);
    lv_obj_set_pos(s_kurt_chart, 0, 64);
    lv_chart_set_type(s_kurt_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_kurt_chart, 36);
    lv_chart_set_range(s_kurt_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_color(s_kurt_chart, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(s_kurt_chart, lv_color_hex(0x1E2330), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_kurt_chart, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(s_kurt_chart, 16, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_kurt_chart, 3, LV_PART_ITEMS);

    s_kurt_series = lv_chart_add_series(s_kurt_chart, lv_color_hex(0x00FF66), LV_CHART_AXIS_PRIMARY_Y);
}

// -------------------------------------------------------------
// VIEW 4: TRIPLE ACTIVITY RINGS
// -------------------------------------------------------------
static void create_view_4_rings(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_TRIPLE_ACTIVITY_RINGS] = v;

    lv_obj_t *ring_box = lv_obj_create(v);
    lv_obj_set_size(ring_box, 230, 230);
    lv_obj_center(ring_box);
    lv_obj_set_style_bg_opa(ring_box, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(ring_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ring_box, LV_OBJ_FLAG_SCROLLABLE);

    s_ring_red = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_red, 210, 210);
    lv_obj_center(s_ring_red);
    lv_arc_set_bg_angles(s_ring_red, 0, 360);
    lv_arc_set_range(s_ring_red, 0, 100);
    lv_arc_set_value(s_ring_red, 98);
    lv_obj_set_style_arc_width(s_ring_red, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_red, lv_color_hex(0x240A12), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_red, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_red, lv_color_hex(0xFF2A54), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_red, LV_OBJ_FLAG_CLICKABLE);

    s_ring_green = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_green, 168, 168);
    lv_obj_center(s_ring_green);
    lv_arc_set_bg_angles(s_ring_green, 0, 360);
    lv_arc_set_range(s_ring_green, 0, 100);
    lv_arc_set_value(s_ring_green, 92);
    lv_obj_set_style_arc_width(s_ring_green, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_green, lv_color_hex(0x062012), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_green, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_green, lv_color_hex(0x00FF66), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_green, LV_OBJ_FLAG_CLICKABLE);

    s_ring_blue = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_blue, 126, 126);
    lv_obj_center(s_ring_blue);
    lv_arc_set_bg_angles(s_ring_blue, 0, 360);
    lv_arc_set_range(s_ring_blue, 0, 100);
    lv_arc_set_value(s_ring_blue, 95);
    lv_obj_set_style_arc_width(s_ring_blue, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_blue, lv_color_hex(0x061826), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_blue, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_blue, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_blue, LV_OBJ_FLAG_CLICKABLE);

    s_ring_score_label = lv_label_create(ring_box);
    lv_label_set_text(s_ring_score_label, "98%");
    lv_obj_set_style_text_color(s_ring_score_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_ring_score_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(s_ring_score_label);
}

// -------------------------------------------------------------
// VIEW 5: PROGRESS ARC
// -------------------------------------------------------------
static void create_view_5_progress_arc(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_PROGRESS_ARC] = v;

    s_progress_arc = lv_arc_create(v);
    lv_obj_set_size(s_progress_arc, 190, 190);
    lv_obj_align(s_progress_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_bg_angles(s_progress_arc, 135, 405);
    lv_arc_set_angles(s_progress_arc, 135, 405);
    lv_arc_set_range(s_progress_arc, 0, 100);
    lv_arc_set_value(s_progress_arc, 98);
    lv_obj_set_style_arc_width(s_progress_arc, 18, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_progress_arc, lv_color_hex(0x141822), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_progress_arc, 18, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_progress_arc, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_progress_arc, LV_OBJ_FLAG_CLICKABLE);

    s_progress_val_label = lv_label_create(v);
    lv_label_set_text(s_progress_val_label, "98%\nOPTIMAL");
    lv_obj_set_style_text_align(s_progress_val_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_progress_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_progress_val_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_progress_val_label, s_progress_arc, LV_ALIGN_CENTER, 0, 0);
}

// -------------------------------------------------------------
// VIEW 6: ROTATION TACHOMETER
// -------------------------------------------------------------
static void create_view_6_tacho(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_ROTATION_TACHOMETER] = v;

    s_tacho_arc = lv_arc_create(v);
    lv_obj_set_size(s_tacho_arc, 190, 190);
    lv_obj_align(s_tacho_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_bg_angles(s_tacho_arc, 135, 405);
    lv_arc_set_angles(s_tacho_arc, 135, 405);
    lv_arc_set_range(s_tacho_arc, 0, 4500);
    lv_arc_set_value(s_tacho_arc, 2910);
    lv_obj_set_style_arc_width(s_tacho_arc, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_tacho_arc, lv_color_hex(0x141822), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_tacho_arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_tacho_arc, lv_color_hex(0xF59E0B), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_tacho_arc, LV_OBJ_FLAG_CLICKABLE);

    s_tacho_rpm_label = lv_label_create(v);
    lv_label_set_text(s_tacho_rpm_label, "2,910\nRPM");
    lv_obj_set_style_text_align(s_tacho_rpm_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_tacho_rpm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_tacho_rpm_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_tacho_rpm_label, s_tacho_arc, LV_ALIGN_CENTER, 0, 0);
}

// -------------------------------------------------------------
// VIEW 7: FLUID ENERGY TANK
// -------------------------------------------------------------
static void create_view_7_fluid(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_FLUID_ENERGY_TANK] = v;

    s_fluid_val_label = lv_label_create(v);
    lv_label_set_text(s_fluid_val_label, "ISO VELOCITY: 0.16 mm/s");
    lv_obj_set_style_text_color(s_fluid_val_label, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_fluid_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_fluid_val_label, 10, 8);

    s_fluid_bar = lv_bar_create(v);
    lv_obj_set_size(s_fluid_bar, 334, 120);
    lv_obj_set_pos(s_fluid_bar, 0, 36);
    lv_bar_set_range(s_fluid_bar, 0, 100);
    lv_bar_set_value(s_fluid_bar, 35, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_fluid_bar, lv_color_hex(0x10141E), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_fluid_bar, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_fluid_bar, 16, LV_PART_MAIN);
    lv_obj_set_style_radius(s_fluid_bar, 16, LV_PART_INDICATOR);

    // 16-Band VU
    for (int i = 0; i < 16; i++) {
        lv_obj_t *bar = lv_bar_create(v);
        lv_obj_set_size(bar, 14, 80);
        lv_obj_set_pos(bar, 4 + i * 20, 170);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 20 + (i % 4) * 20, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x141822), LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_hex((i > 11) ? 0xFF2A54 : (i > 7) ? 0xF59E0B : 0xF5C544), LV_PART_INDICATOR);
        s_fluid_vu_bars[i] = bar;
    }
}

// -------------------------------------------------------------
// VIEW 8: HARMONIC GOAL PILLARS
// -------------------------------------------------------------
static void create_view_8_goal_pillars(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_HARMONIC_GOAL_PILLARS] = v;

    int p_w = 32;
    int p_gap = 14;
    for (int i = 0; i < 7; i++) {
        lv_obj_t *p_bar = lv_bar_create(v);
        lv_obj_set_size(p_bar, p_w, 190);
        lv_obj_set_pos(p_bar, 10 + i * (p_w + p_gap), 30);
        lv_bar_set_range(p_bar, 0, 100);
        lv_bar_set_value(p_bar, (i == 0) ? 85 : 25 + (i * 8), LV_ANIM_OFF);
        lv_obj_set_style_bg_color(p_bar, lv_color_hex(0x141822), LV_PART_MAIN);
        lv_obj_set_style_radius(p_bar, 16, LV_PART_MAIN);
        lv_obj_set_style_radius(p_bar, 16, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(p_bar, lv_color_hex((i == 0) ? 0xF5C544 : 0xEA580C), LV_PART_INDICATOR);
        s_goal_bars[i] = p_bar;

        lv_obj_t *lbl = lv_label_create(v);
        char buf[8];
        snprintf(buf, sizeof(buf), "%dX", i + 1);
        lv_label_set_text(lbl, buf);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_set_pos(lbl, 16 + i * (p_w + p_gap), 230);
        s_goal_val_labels[i] = lbl;
    }
}

// -------------------------------------------------------------
// VIEW 9: 24H DOT MATRIX HEATMAP
// -------------------------------------------------------------
static void create_view_9_matrix(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_DOT_MATRIX_HEATMAP] = v;

    lv_obj_t *lbl_info = lv_label_create(v);
    lv_label_set_text(lbl_info, "24H ANOMALY OBSERVATION GRID");
    lv_obj_set_style_text_color(lbl_info, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_info, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(lbl_info, 10, 8);

    int dot_size = 42;
    int spacing = 12;
    for (int i = 0; i < 24; i++) {
        int r = i / 6;
        int c = i % 6;
        lv_obj_t *dot = lv_obj_create(v);
        lv_obj_set_size(dot, dot_size, dot_size);
        lv_obj_set_pos(dot, 10 + c * (dot_size + spacing), 40 + r * (dot_size + spacing));
        lv_obj_set_style_bg_color(dot, lv_color_hex(0x061E10), LV_PART_MAIN);
        lv_obj_set_style_border_color(dot, lv_color_hex(0x00FF66), LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 1, LV_PART_MAIN);
        lv_obj_set_style_radius(dot, 10, LV_PART_MAIN);
        s_matrix_dots[i] = dot;
    }
}

// -------------------------------------------------------------
// VIEW 10: BOLD AI DIAGNOSTICS
// -------------------------------------------------------------
static void create_view_10_diagnostics(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 356, 360);
    lv_obj_set_pos(v, 6, 40);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(v, lv_color_hex(0x181C26), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(v, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 8, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_BOLD_DIAGNOSTICS] = v;

    s_diag_iso_badge = lv_label_create(v);
    lv_label_set_text(s_diag_iso_badge, "ISO 10816 CLASS A");
    lv_obj_set_style_text_color(s_diag_iso_badge, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_iso_badge, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_diag_iso_badge, 10, 8);

    s_diag_bold_title = lv_label_create(v);
    lv_label_set_text(s_diag_bold_title, "NOMINAL HARMONIC");
    lv_obj_set_style_text_color(s_diag_bold_title, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_bold_title, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_diag_bold_title, 10, 36);

    s_diag_recom_label = lv_label_create(v);
    lv_label_set_text(s_diag_recom_label, "Machine is operating within optimal ISO limits. Rotational symmetry verified with zero bearing spalling.");
    lv_label_set_long_mode(s_diag_recom_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_diag_recom_label, 320);
    lv_obj_set_style_text_color(s_diag_recom_label, lv_color_hex(0x9CA3AF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_recom_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_diag_recom_label, 10, 90);
}

void ui_engine_init(void) {
    s_scr = lv_screen_active();
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. TOP CLICKABLE HEADER PILL (y: 4, height: 30)
    lv_obj_t *top_hdr = lv_obj_create(s_scr);
    lv_obj_set_size(top_hdr, 356, 32);
    lv_obj_set_pos(top_hdr, 6, 4);
    lv_obj_set_style_bg_color(top_hdr, lv_color_hex(0x0A0D14), LV_PART_MAIN);
    lv_obj_set_style_border_color(top_hdr, lv_color_hex(0x1E2432), LV_PART_MAIN);
    lv_obj_set_style_border_width(top_hdr, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(top_hdr, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_all(top_hdr, 2, LV_PART_MAIN);
    lv_obj_clear_flag(top_hdr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(top_hdr, btn_next_view_cb, LV_EVENT_CLICKED, NULL);

    s_header_pill_label = lv_label_create(top_hdr);
    lv_label_set_text(s_header_pill_label, "1/10: SINE WAVE TRANSDUCER");
    lv_obj_set_style_text_color(s_header_pill_label, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_header_pill_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(s_header_pill_label);

    // 2. CREATE ALL 10 VIEW CONTAINERS
    create_view_1_sine(s_scr);
    create_view_2_fft(s_scr);
    create_view_3_kurtosis(s_scr);
    create_view_4_rings(s_scr);
    create_view_5_progress_arc(s_scr);
    create_view_6_tacho(s_scr);
    create_view_7_fluid(s_scr);
    create_view_8_goal_pillars(s_scr);
    create_view_9_matrix(s_scr);
    create_view_10_diagnostics(s_scr);

    // 3. BOTTOM 10-DOT PAGINATION BAR (y: 410, height: 30)
    lv_obj_t *dots_container = lv_obj_create(s_scr);
    lv_obj_set_size(dots_container, 356, 28);
    lv_obj_set_pos(dots_container, 6, 410);
    lv_obj_set_style_bg_opa(dots_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(dots_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dots_container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dots_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(dots_container, btn_next_view_cb, LV_EVENT_CLICKED, NULL);

    int start_x = 44;
    for (int i = 0; i < VIEW_MAX_COUNT; i++) {
        lv_obj_t *dot = lv_obj_create(dots_container);
        lv_obj_set_size(dot, (i == 0) ? 14 : 5, 5);
        lv_obj_set_pos(dot, start_x + i * 27, 12);
        lv_obj_set_style_bg_color(dot, lv_color_hex((i == 0) ? 0xFFFFFF : 0x222630), LV_PART_MAIN);
        lv_obj_set_style_radius(dot, 3, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        s_nav_dots[i] = dot;
    }

    // Set View 1 as initial view
    ui_engine_set_view(VIEW_SINE_WAVE_TRANSDUCER);
}

void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count) {
    if (!metrics) return;

    // Apply EMA low-pass smoothing filters to eliminate jitter and flickering
    s_smooth_rms = s_smooth_rms * 0.85f + metrics->rms_acceleration_g * 0.15f;
    s_smooth_kurt = s_smooth_kurt * 0.90f + metrics->kurtosis * 0.10f;
    s_smooth_rpm = s_smooth_rpm * 0.85f + (float)metrics->estimated_rpm * 0.15f;
    s_smooth_f0 = s_smooth_f0 * 0.85f + (metrics->peak_freq_hz > 5.0f ? metrics->peak_freq_hz : 48.5f) * 0.15f;

    // Smooth continuous phase accumulation
    s_sine_phase += 0.14f;

    char buf[64];

    // VIEW 1: SINE WAVE TRANSDUCER
    if (s_current_view == VIEW_SINE_WAVE_TRANSDUCER) {
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)s_smooth_rpm);
        lv_label_set_text(s_sine_bold_rpm_label, buf);

        snprintf(buf, sizeof(buf), "Rotor RPM · %.1f Hz Fundamental", s_smooth_f0);
        lv_label_set_text(s_sine_sub_label, buf);

        snprintf(buf, sizeof(buf), "⚡ %.3fg RMS", s_smooth_rms);
        lv_label_set_text(s_sine_rms_pill, buf);

        snprintf(buf, sizeof(buf), "🔥 Kurt: %.2f", s_smooth_kurt);
        lv_label_set_text(s_sine_kurt_pill, buf);

        float amp = s_smooth_rms * 120.0f;
        if (amp > 100.0f) amp = 100.0f;
        if (amp < 18.0f) amp = 18.0f;

        for (int i = 0; i < 48; i++) {
            float theta = s_sine_phase + (float)i * 0.18f;
            int32_t val_x = (int32_t)(amp * sinf(theta));
            int32_t val_y = (int32_t)(amp * 0.75f * sinf(theta + 1.2f));
            int32_t val_z = (int32_t)(amp * 0.85f * sinf(theta + 2.4f));
            lv_chart_set_value_by_id(s_sine_chart, s_sine_series_cyan, i, val_x);
            lv_chart_set_value_by_id(s_sine_chart, s_sine_series_green, i, val_y);
            lv_chart_set_value_by_id(s_sine_chart, s_sine_series_red, i, val_z);
        }
        lv_chart_refresh(s_sine_chart);
    }

    // VIEW 2: 24-BAND FFT SPECTRUM
    else if (s_current_view == VIEW_FFT_SPECTRUM) {
        snprintf(buf, sizeof(buf), "1X Peak: %.1f Hz", s_smooth_f0);
        lv_label_set_text(s_fft_peak_label, buf);

        for (int i = 0; i < 24; i++) {
            int val = (int)(metrics->visual_spectrum[i] * 100.0f);
            if (val > 100) val = 100;
            lv_bar_set_value(s_fft_bars[i], val, LV_ANIM_OFF);
        }
    }

    // VIEW 3: KURTOSIS IMPACT
    else if (s_current_view == VIEW_KURTOSIS_IMPACT) {
        snprintf(buf, sizeof(buf), "%.2f", s_smooth_kurt);
        lv_label_set_text(s_kurt_bold_val, buf);

        if (s_smooth_kurt > 4.5f) {
            lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0xFF2A54), LV_PART_MAIN);
            lv_label_set_text(s_kurt_status_label, "IMPACT SHOCK DETECTED (Bearing Spalling)");
        } else if (s_smooth_kurt > 3.6f) {
            lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0xF59E0B), LV_PART_MAIN);
            lv_label_set_text(s_kurt_status_label, "Elevated Kurtosis Warning");
        } else {
            lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0x00FF66), LV_PART_MAIN);
            lv_label_set_text(s_kurt_status_label, "Gaussian Symmetry (Zero Spalling)");
        }

        for (size_t i = 0; i < 36; i++) {
            int32_t val = (int32_t)((s_smooth_kurt / 8.0f) * 100.0f);
            if (val > 100) val = 100;
            lv_chart_set_value_by_id(s_kurt_chart, s_kurt_series, i, val);
        }
        lv_chart_refresh(s_kurt_chart);
    }

    // VIEW 4: TRIPLE ACTIVITY RINGS
    else if (s_current_view == VIEW_TRIPLE_ACTIVITY_RINGS) {
        lv_arc_set_value(s_ring_red, metrics->health_score);
        int bearing_val = (s_smooth_kurt > 5.0f) ? 30 : (s_smooth_kurt > 3.8f) ? 65 : 95;
        lv_arc_set_value(s_ring_green, bearing_val);
        int balance_val = (s_smooth_rms > 0.3f) ? 25 : (s_smooth_rms > 0.15f) ? 60 : 98;
        lv_arc_set_value(s_ring_blue, balance_val);

        snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
        lv_label_set_text(s_ring_score_label, buf);
    }

    // VIEW 5: PROGRESS ARC
    else if (s_current_view == VIEW_PROGRESS_ARC) {
        lv_arc_set_value(s_progress_arc, metrics->health_score);
        snprintf(buf, sizeof(buf), "%d%%\nOPTIMAL", metrics->health_score);
        lv_label_set_text(s_progress_val_label, buf);
    }

    // VIEW 6: ROTATION TACHOMETER
    else if (s_current_view == VIEW_ROTATION_TACHOMETER) {
        lv_arc_set_value(s_tacho_arc, (int32_t)s_smooth_rpm);
        snprintf(buf, sizeof(buf), "%lu\nRPM", (unsigned long)s_smooth_rpm);
        lv_label_set_text(s_tacho_rpm_label, buf);
    }

    // VIEW 7: FLUID ENERGY TANK
    else if (s_current_view == VIEW_FLUID_ENERGY_TANK) {
        snprintf(buf, sizeof(buf), "ISO VELOCITY: %.2f mm/s", metrics->iso_vibration_vel);
        lv_label_set_text(s_fluid_val_label, buf);
        int f_val = (int)(s_smooth_rms * 180.0f);
        if (f_val > 100) f_val = 100;
        lv_bar_set_value(s_fluid_bar, f_val, LV_ANIM_OFF);

        for (int i = 0; i < 16; i++) {
            int val = (int)(metrics->visual_spectrum[i] * 100.0f);
            if (val > 100) val = 100;
            lv_bar_set_value(s_fluid_vu_bars[i], val, LV_ANIM_OFF);
        }
    }

    // VIEW 8: HARMONIC GOAL PILLARS
    else if (s_current_view == VIEW_HARMONIC_GOAL_PILLARS) {
        for (int i = 0; i < 7; i++) {
            int val = (int)(metrics->visual_spectrum[i * 2] * 100.0f);
            if (val > 100) val = 100;
            lv_bar_set_value(s_goal_bars[i], val, LV_ANIM_OFF);
        }
    }

    // VIEW 9: 24H DOT MATRIX HEATMAP
    else if (s_current_view == VIEW_DOT_MATRIX_HEATMAP) {
        for (int i = 0; i < 24; i++) {
            if (i % 7 == 0 && s_smooth_rms > 0.2f) {
                lv_obj_set_style_bg_color(s_matrix_dots[i], lv_color_hex(0xFF2A54), LV_PART_MAIN);
            } else {
                lv_obj_set_style_bg_color(s_matrix_dots[i], lv_color_hex(0x061E10), LV_PART_MAIN);
            }
        }
    }

    // VIEW 10: BOLD AI DIAGNOSTICS
    else if (s_current_view == VIEW_BOLD_DIAGNOSTICS) {
        lv_label_set_text(s_diag_bold_title, metrics->diagnosis_text);
        lv_label_set_text(s_diag_recom_label, metrics->recommendation);
    }
}
