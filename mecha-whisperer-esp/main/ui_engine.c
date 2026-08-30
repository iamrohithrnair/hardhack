#include "ui_engine.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static lv_obj_t *s_scr;
static ui_view_mode_t s_current_view = VIEW_SINE_WAVE_TRANSDUCER;

// View Containers
static lv_obj_t *s_view_objs[VIEW_MAX_COUNT];

// -------------------------------------------------------------
// VIEW 1: CLASSIC FULL STETHOSCOPE DASHBOARD (Original Commit)
// -------------------------------------------------------------
static lv_obj_t *s_health_arc;
static lv_obj_t *s_health_val_label;
static lv_obj_t *s_status_label;
static lv_obj_t *s_f0_rpm_label;
static lv_obj_t *s_osc_chart;
static lv_chart_series_t *s_osc_series;
static lv_obj_t *s_fft_chart;
static lv_chart_series_t *s_fft_series;
static lv_obj_t *s_rms_val_label;
static lv_obj_t *s_kurt_val_label;
static lv_obj_t *s_iso_val_label;

// View 2: 24-Band FFT Harmonic Spectrum
static lv_obj_t *s_fft_peak_label;
static lv_obj_t *s_fft_bars[24];

// View 3: Kurtosis & Impulse Shock History
static lv_obj_t *s_kurt_chart;
static lv_chart_series_t *s_kurt_series;
static lv_obj_t *s_kurt_bold_val;
static lv_obj_t *s_kurt_status_label;

// View 4: Triple Activity Rings & Health Score
static lv_obj_t *s_ring_red;
static lv_obj_t *s_ring_green;
static lv_obj_t *s_ring_blue;
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

// Smoothing state variables
static float s_smooth_rms = 0.065f;
static float s_smooth_kurt = 2.85f;
static float s_smooth_rpm = 1800.0f;
static float s_smooth_f0 = 30.0f;

static void btn_next_view_cb(lv_event_t *e) {
    (void)e;
    ui_engine_next_view();
}

static void btn_calib_event_cb(lv_event_t *e) {
    (void)e;
    dsp_engine_start_calibration(60);
}

static void btn_demo_event_cb(lv_event_t *e) {
    (void)e;
    if (!dsp_engine_is_demo_mode()) {
        dsp_engine_set_demo_mode(true);
        dsp_engine_set_demo_fault(STATE_HEALTHY);
    } else if (dsp_engine_get_demo_fault() == STATE_HEALTHY) {
        dsp_engine_set_demo_fault(STATE_CRITICAL_UNBALANCE);
    } else {
        dsp_engine_set_demo_mode(false);
    }
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
                lv_obj_set_style_width(s_nav_dots[i], 16, LV_PART_MAIN);
            } else {
                lv_obj_set_style_bg_color(s_nav_dots[i], lv_color_hex(0x444444), LV_PART_MAIN);
                lv_obj_set_style_width(s_nav_dots[i], 5, LV_PART_MAIN);
            }
        }
    }

    const char *titles[] = {
        "1/10: MECHA-WHISPERER (FRIDGE)",
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
// VIEW 1: ORIGINAL CLASSIC STETHOSCOPE DASHBOARD
// -------------------------------------------------------------
static void create_view_1_classic(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 4, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_SINE_WAVE_TRANSDUCER] = v;

    // 1. HEALTH SCORE CARD (y: 2, Height: 84px)
    lv_obj_t *health_card = lv_obj_create(v);
    lv_obj_set_size(health_card, 356, 84);
    lv_obj_set_pos(health_card, 2, 2);
    lv_obj_set_style_bg_color(health_card, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(health_card, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(health_card, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(health_card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(health_card, LV_OBJ_FLAG_SCROLLABLE);

    // Circular Health Arc
    s_health_arc = lv_arc_create(health_card);
    lv_obj_set_size(s_health_arc, 72, 72);
    lv_obj_align(s_health_arc, LV_ALIGN_LEFT_MID, 2, 0);
    lv_arc_set_range(s_health_arc, 0, 100);
    lv_arc_set_value(s_health_arc, 98);
    lv_arc_set_bg_angles(s_health_arc, 0, 360);
    lv_obj_set_style_arc_width(s_health_arc, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_health_arc, 7, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(0x00FF66), LV_PART_INDICATOR);
    lv_obj_remove_style(s_health_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(s_health_arc, LV_OBJ_FLAG_CLICKABLE);

    s_health_val_label = lv_label_create(health_card);
    lv_label_set_text(s_health_val_label, "98%");
    lv_obj_set_style_text_color(s_health_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_health_val_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_health_val_label, s_health_arc, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *title_h = lv_label_create(health_card);
    lv_label_set_text(title_h, "MACHINE HEALTH SCORE");
    lv_obj_set_style_text_color(title_h, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_h, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(title_h, 86, 4);

    s_status_label = lv_label_create(health_card);
    lv_label_set_text(s_status_label, "STATUS: HEALTHY NOMINAL");
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_status_label, 86, 24);

    s_f0_rpm_label = lv_label_create(health_card);
    lv_label_set_text(s_f0_rpm_label, "F0: 30.0 Hz (1800 RPM)");
    lv_obj_set_style_text_color(s_f0_rpm_label, lv_color_hex(0x00FF88), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_f0_rpm_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_f0_rpm_label, 86, 48);

    // 2. OSCILLOSCOPE CARD (y: 90, Height: 110px)
    lv_obj_t *osc_card = lv_obj_create(v);
    lv_obj_set_size(osc_card, 356, 110);
    lv_obj_set_pos(osc_card, 2, 90);
    lv_obj_set_style_bg_color(osc_card, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(osc_card, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(osc_card, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(osc_card, 2, LV_PART_MAIN);
    lv_obj_clear_flag(osc_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *osc_title = lv_label_create(osc_card);
    lv_label_set_text(osc_title, "MICRO-VIBRATION STETHOSCOPE");
    lv_obj_set_style_text_color(title_h, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(osc_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(osc_title, LV_ALIGN_TOP_LEFT, 6, 2);

    s_osc_chart = lv_chart_create(osc_card);
    lv_obj_set_size(s_osc_chart, 344, 84);
    lv_obj_align(s_osc_chart, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_chart_set_type(s_osc_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_osc_chart, 64);
    lv_chart_set_range(s_osc_chart, LV_CHART_AXIS_PRIMARY_Y, -120, 120);
    lv_obj_set_style_size(s_osc_chart, 0, 0, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_osc_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_osc_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_osc_chart, 3, LV_PART_ITEMS);
    s_osc_series = lv_chart_add_series(s_osc_chart, lv_color_hex(0x00F0FF), LV_CHART_AXIS_PRIMARY_Y);

    // 3. FFT SPECTRUM CARD (y: 204, Height: 80px)
    lv_obj_t *fft_card = lv_obj_create(v);
    lv_obj_set_size(fft_card, 356, 80);
    lv_obj_set_pos(fft_card, 2, 204);
    lv_obj_set_style_bg_color(fft_card, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(fft_card, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(fft_card, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(fft_card, 2, LV_PART_MAIN);
    lv_obj_clear_flag(fft_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *fft_title = lv_label_create(fft_card);
    lv_label_set_text(fft_title, "24-BAND FFT SPECTRUM");
    lv_obj_set_style_text_color(fft_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(fft_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(fft_title, LV_ALIGN_TOP_LEFT, 6, 2);

    s_fft_chart = lv_chart_create(fft_card);
    lv_obj_set_size(s_fft_chart, 344, 56);
    lv_obj_align(s_fft_chart, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_chart_set_type(s_fft_chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(s_fft_chart, BARS_COUNT);
    lv_chart_set_range(s_fft_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_opa(s_fft_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_fft_chart, 0, LV_PART_MAIN);
    s_fft_series = lv_chart_add_series(s_fft_chart, lv_color_hex(0x00F0FF), LV_CHART_AXIS_PRIMARY_Y);

    // 4. METRICS ROW (y: 288, Height: 44px)
    int card_w = 114;
    lv_obj_t *m1 = lv_obj_create(v);
    lv_obj_set_size(m1, card_w, 44);
    lv_obj_set_pos(m1, 2, 288);
    lv_obj_set_style_bg_color(m1, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(m1, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m1, 2, LV_PART_MAIN);
    lv_obj_clear_flag(m1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l1 = lv_label_create(m1);
    lv_label_set_text(l1, "RMS ACCEL");
    lv_obj_set_style_text_color(l1, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(l1, &lv_font_montserrat_12, LV_PART_MAIN);
    s_rms_val_label = lv_label_create(m1);
    lv_label_set_text(s_rms_val_label, "0.065g");
    lv_obj_set_style_text_color(s_rms_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_rms_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_rms_val_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *m2 = lv_obj_create(v);
    lv_obj_set_size(m2, card_w, 44);
    lv_obj_set_pos(m2, 122, 288);
    lv_obj_set_style_bg_color(m2, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(m2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m2, 2, LV_PART_MAIN);
    lv_obj_clear_flag(m2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l2 = lv_label_create(m2);
    lv_label_set_text(l2, "KURTOSIS");
    lv_obj_set_style_text_color(l2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(l2, &lv_font_montserrat_12, LV_PART_MAIN);
    s_kurt_val_label = lv_label_create(m2);
    lv_label_set_text(s_kurt_val_label, "2.85");
    lv_obj_set_style_text_color(s_kurt_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_kurt_val_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *m3 = lv_obj_create(v);
    lv_obj_set_size(m3, card_w, 44);
    lv_obj_set_pos(m3, 242, 288);
    lv_obj_set_style_bg_color(m3, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(m3, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m3, 2, LV_PART_MAIN);
    lv_obj_clear_flag(m3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l3 = lv_label_create(m3);
    lv_label_set_text(l3, "ISO-10816");
    lv_obj_set_style_text_color(l3, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(l3, &lv_font_montserrat_12, LV_PART_MAIN);
    s_iso_val_label = lv_label_create(m3);
    lv_label_set_text(s_iso_val_label, "CLS A");
    lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0x00FF88), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_iso_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_iso_val_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // 5. BOTTOM TOUCH BUTTONS (y: 334, Height: 34px)
    int btn_w = 82;
    lv_obj_t *btn1 = lv_button_create(v);
    lv_obj_set_size(btn1, btn_w, 32);
    lv_obj_set_pos(btn1, 2, 334);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn1, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn1, btn_calib_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t1 = lv_label_create(btn1);
    lv_label_set_text(t1, "CALIB");
    lv_obj_set_style_text_color(t1, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_center(t1);

    lv_obj_t *btn2 = lv_button_create(v);
    lv_obj_set_size(btn2, btn_w, 32);
    lv_obj_set_pos(btn2, 90, 334);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn2, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn2, btn_next_view_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t2 = lv_label_create(btn2);
    lv_label_set_text(t2, "NEXT");
    lv_obj_set_style_text_color(t2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(t2);

    lv_obj_t *btn3 = lv_button_create(v);
    lv_obj_set_size(btn3, btn_w, 32);
    lv_obj_set_pos(btn3, 178, 334);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn3, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn3, btn_demo_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t3 = lv_label_create(btn3);
    lv_label_set_text(t3, "DEMO");
    lv_obj_set_style_text_color(t3, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_center(t3);

    lv_obj_t *btn4 = lv_button_create(v);
    lv_obj_set_size(btn4, btn_w, 32);
    lv_obj_set_pos(btn4, 266, 334);
    lv_obj_set_style_bg_color(btn4, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn4, 0, LV_PART_MAIN);
    lv_obj_t *t4 = lv_label_create(btn4);
    lv_label_set_text(t4, "AUDIO");
    lv_obj_set_style_text_color(t4, lv_color_hex(0xFF2A54), LV_PART_MAIN);
    lv_obj_center(t4);
}

// -------------------------------------------------------------
// VIEW 2: 24-BAND FFT SPECTRUM
// -------------------------------------------------------------
static void create_view_2_fft(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_FFT_SPECTRUM] = v;

    s_fft_peak_label = lv_label_create(v);
    lv_label_set_text(s_fft_peak_label, "1X Harmonic Peak: 30.0 Hz");
    lv_obj_set_style_text_color(s_fft_peak_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_fft_peak_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_fft_peak_label, 10, 8);

    lv_obj_t *eq_box = lv_obj_create(v);
    lv_obj_set_size(eq_box, 356, 240);
    lv_obj_set_pos(eq_box, 0, 36);
    lv_obj_set_style_bg_color(eq_box, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(eq_box, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(eq_box, 4, LV_PART_MAIN);
    lv_obj_clear_flag(eq_box, LV_OBJ_FLAG_SCROLLABLE);

    int bar_w = 9;
    int spacing = 5;
    for (int i = 0; i < 24; i++) {
        lv_obj_t *bar = lv_bar_create(eq_box);
        lv_obj_set_size(bar, bar_w, 210);
        lv_obj_set_pos(bar, 2 + i * (bar_w + spacing), 10);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, (i == 4) ? 88 : 12 + (i % 6) * 8, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x111111), LV_PART_MAIN);
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
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_KURTOSIS_IMPACT] = v;

    s_kurt_bold_val = lv_label_create(v);
    lv_label_set_text(s_kurt_bold_val, "2.85");
    lv_obj_set_style_text_color(s_kurt_bold_val, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_bold_val, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_kurt_bold_val, 10, 8);

    s_kurt_status_label = lv_label_create(v);
    lv_label_set_text(s_kurt_status_label, "Gaussian Symmetry (Zero Piston Knock)");
    lv_obj_set_style_text_color(s_kurt_status_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_kurt_status_label, 10, 36);

    s_kurt_chart = lv_chart_create(v);
    lv_obj_set_size(s_kurt_chart, 356, 220);
    lv_obj_set_pos(s_kurt_chart, 0, 64);
    lv_chart_set_type(s_kurt_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_kurt_chart, 36);
    lv_chart_set_range(s_kurt_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_color(s_kurt_chart, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_kurt_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_kurt_chart, 3, LV_PART_ITEMS);

    s_kurt_series = lv_chart_add_series(s_kurt_chart, lv_color_hex(0x00FF66), LV_CHART_AXIS_PRIMARY_Y);
}

// -------------------------------------------------------------
// VIEW 4: TRIPLE ACTIVITY RINGS
// -------------------------------------------------------------
static void create_view_4_rings(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_TRIPLE_ACTIVITY_RINGS] = v;

    lv_obj_t *ring_box = lv_obj_create(v);
    lv_obj_set_size(ring_box, 250, 250);
    lv_obj_center(ring_box);
    lv_obj_set_style_bg_color(ring_box, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(ring_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ring_box, LV_OBJ_FLAG_SCROLLABLE);

    s_ring_red = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_red, 230, 230);
    lv_obj_center(s_ring_red);
    lv_arc_set_bg_angles(s_ring_red, 0, 360);
    lv_arc_set_range(s_ring_red, 0, 100);
    lv_arc_set_value(s_ring_red, 98);
    lv_obj_set_style_arc_width(s_ring_red, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_red, lv_color_hex(0x1A0008), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_red, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_red, lv_color_hex(0xFF2A54), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_red, LV_OBJ_FLAG_CLICKABLE);

    s_ring_green = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_green, 180, 180);
    lv_obj_center(s_ring_green);
    lv_arc_set_bg_angles(s_ring_green, 0, 360);
    lv_arc_set_range(s_ring_green, 0, 100);
    lv_arc_set_value(s_ring_green, 92);
    lv_obj_set_style_arc_width(s_ring_green, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_green, lv_color_hex(0x001A08), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_green, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_ring_green, lv_color_hex(0x00FF66), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_ring_green, LV_OBJ_FLAG_CLICKABLE);

    s_ring_blue = lv_arc_create(ring_box);
    lv_obj_set_size(s_ring_blue, 130, 130);
    lv_obj_center(s_ring_blue);
    lv_arc_set_bg_angles(s_ring_blue, 0, 360);
    lv_arc_set_range(s_ring_blue, 0, 100);
    lv_arc_set_value(s_ring_blue, 95);
    lv_obj_set_style_arc_width(s_ring_blue, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_ring_blue, lv_color_hex(0x001420), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_ring_blue, 14, LV_PART_INDICATOR);
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
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_PROGRESS_ARC] = v;

    s_progress_arc = lv_arc_create(v);
    lv_obj_set_size(s_progress_arc, 200, 200);
    lv_obj_align(s_progress_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_bg_angles(s_progress_arc, 135, 405);
    lv_arc_set_angles(s_progress_arc, 135, 405);
    lv_arc_set_range(s_progress_arc, 0, 100);
    lv_arc_set_value(s_progress_arc, 98);
    lv_obj_set_style_arc_width(s_progress_arc, 18, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_progress_arc, lv_color_hex(0x111111), LV_PART_MAIN);
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
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_ROTATION_TACHOMETER] = v;

    s_tacho_arc = lv_arc_create(v);
    lv_obj_set_size(s_tacho_arc, 200, 200);
    lv_obj_align(s_tacho_arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_bg_angles(s_tacho_arc, 135, 405);
    lv_arc_set_angles(s_tacho_arc, 135, 405);
    lv_arc_set_range(s_tacho_arc, 0, 3600);
    lv_arc_set_value(s_tacho_arc, 1800);
    lv_obj_set_style_arc_width(s_tacho_arc, 18, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_tacho_arc, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_tacho_arc, 18, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_tacho_arc, lv_color_hex(0xF59E0B), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_tacho_arc, LV_OBJ_FLAG_CLICKABLE);

    s_tacho_rpm_label = lv_label_create(v);
    lv_label_set_text(s_tacho_rpm_label, "1,800\nRPM");
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
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_FLUID_ENERGY_TANK] = v;

    s_fluid_val_label = lv_label_create(v);
    lv_label_set_text(s_fluid_val_label, "ISO VELOCITY: 0.12 mm/s");
    lv_obj_set_style_text_color(s_fluid_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_fluid_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_fluid_val_label, 10, 8);

    s_fluid_bar = lv_bar_create(v);
    lv_obj_set_size(s_fluid_bar, 356, 120);
    lv_obj_set_pos(s_fluid_bar, 0, 36);
    lv_bar_set_range(s_fluid_bar, 0, 100);
    lv_bar_set_value(s_fluid_bar, 35, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_fluid_bar, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_fluid_bar, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_fluid_bar, 16, LV_PART_MAIN);
    lv_obj_set_style_radius(s_fluid_bar, 16, LV_PART_INDICATOR);

    // 16-Band VU
    for (int i = 0; i < 16; i++) {
        lv_obj_t *bar = lv_bar_create(v);
        lv_obj_set_size(bar, 15, 80);
        lv_obj_set_pos(bar, 4 + i * 22, 170);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 20 + (i % 4) * 20, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x111111), LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_hex((i > 11) ? 0xFF2A54 : (i > 7) ? 0xF59E0B : 0xF5C544), LV_PART_INDICATOR);
        s_fluid_vu_bars[i] = bar;
    }
}

// -------------------------------------------------------------
// VIEW 8: HARMONIC GOAL PILLARS
// -------------------------------------------------------------
static void create_view_8_goal_pillars(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_HARMONIC_GOAL_PILLARS] = v;

    int p_w = 34;
    int p_gap = 14;
    for (int i = 0; i < 7; i++) {
        lv_obj_t *p_bar = lv_bar_create(v);
        lv_obj_set_size(p_bar, p_w, 210);
        lv_obj_set_pos(p_bar, 10 + i * (p_w + p_gap), 26);
        lv_bar_set_range(p_bar, 0, 100);
        lv_bar_set_value(p_bar, (i == 0) ? 85 : 25 + (i * 8), LV_ANIM_OFF);
        lv_obj_set_style_bg_color(p_bar, lv_color_hex(0x111111), LV_PART_MAIN);
        lv_obj_set_style_radius(p_bar, 16, LV_PART_MAIN);
        lv_obj_set_style_radius(p_bar, 16, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(p_bar, lv_color_hex((i == 0) ? 0xF5C544 : 0xEA580C), LV_PART_INDICATOR);
        s_goal_bars[i] = p_bar;

        lv_obj_t *lbl = lv_label_create(v);
        char buf[8];
        snprintf(buf, sizeof(buf), "%dX", i + 1);
        lv_label_set_text(lbl, buf);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_pos(lbl, 18 + i * (p_w + p_gap), 246);
        s_goal_val_labels[i] = lbl;
    }
}

// -------------------------------------------------------------
// VIEW 9: 24H DOT MATRIX HEATMAP
// -------------------------------------------------------------
static void create_view_9_matrix(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_DOT_MATRIX_HEATMAP] = v;

    lv_obj_t *lbl_info = lv_label_create(v);
    lv_label_set_text(lbl_info, "24H ANOMALY OBSERVATION GRID");
    lv_obj_set_style_text_color(lbl_info, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_info, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(lbl_info, 10, 8);

    int dot_size = 44;
    int spacing = 12;
    for (int i = 0; i < 24; i++) {
        int r = i / 6;
        int c = i % 6;
        lv_obj_t *dot = lv_obj_create(v);
        lv_obj_set_size(dot, dot_size, dot_size);
        lv_obj_set_pos(dot, 12 + c * (dot_size + spacing), 40 + r * (dot_size + spacing));
        lv_obj_set_style_bg_color(dot, lv_color_hex(0x001A08), LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(dot, 12, LV_PART_MAIN);
        s_matrix_dots[i] = dot;
    }
}

// -------------------------------------------------------------
// VIEW 10: BOLD AI DIAGNOSTICS
// -------------------------------------------------------------
static void create_view_10_diagnostics(lv_obj_t *parent) {
    lv_obj_t *v = lv_obj_create(parent);
    lv_obj_set_size(v, 368, 370);
    lv_obj_set_pos(v, 0, 36);
    lv_obj_set_style_bg_color(v, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(v, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(v, 6, LV_PART_MAIN);
    lv_obj_clear_flag(v, LV_OBJ_FLAG_SCROLLABLE);
    s_view_objs[VIEW_BOLD_DIAGNOSTICS] = v;

    s_diag_iso_badge = lv_label_create(v);
    lv_label_set_text(s_diag_iso_badge, "ISO 10816 CLASS A (FRIDGE)");
    lv_obj_set_style_text_color(s_diag_iso_badge, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_iso_badge, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_diag_iso_badge, 10, 8);

    s_diag_bold_title = lv_label_create(v);
    lv_label_set_text(s_diag_bold_title, "HEALTHY (NOMINAL)");
    lv_obj_set_style_text_color(s_diag_bold_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_bold_title, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_pos(s_diag_bold_title, 10, 36);

    s_diag_recom_label = lv_label_create(v);
    lv_label_set_text(s_diag_recom_label, "Refrigerator compressor operating in nominal state. Piston harmonics stable with zero valve flutter.");
    lv_label_set_long_mode(s_diag_recom_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_diag_recom_label, 330);
    lv_obj_set_style_text_color(s_diag_recom_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_diag_recom_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_diag_recom_label, 10, 90);
}

void ui_engine_init(void) {
    s_scr = lv_screen_active();
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. TOP CLICKABLE HEADER PILL (y: 2, height: 32)
    lv_obj_t *top_hdr = lv_obj_create(s_scr);
    lv_obj_set_size(top_hdr, 368, 32);
    lv_obj_set_pos(top_hdr, 0, 2);
    lv_obj_set_style_bg_color(top_hdr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(top_hdr, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(top_hdr, 2, LV_PART_MAIN);
    lv_obj_clear_flag(top_hdr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(top_hdr, btn_next_view_cb, LV_EVENT_CLICKED, NULL);

    s_header_pill_label = lv_label_create(top_hdr);
    lv_label_set_text(s_header_pill_label, "1/10: MECHA-WHISPERER (FRIDGE)");
    lv_obj_set_style_text_color(s_header_pill_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_header_pill_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(s_header_pill_label);

    // 2. CREATE ALL 10 VIEW CONTAINERS
    create_view_1_classic(s_scr);
    create_view_2_fft(s_scr);
    create_view_3_kurtosis(s_scr);
    create_view_4_rings(s_scr);
    create_view_5_progress_arc(s_scr);
    create_view_6_tacho(s_scr);
    create_view_7_fluid(s_scr);
    create_view_8_goal_pillars(s_scr);
    create_view_9_matrix(s_scr);
    create_view_10_diagnostics(s_scr);

    // 3. BOTTOM 10-DOT PAGINATION BAR (y: 412, height: 26)
    lv_obj_t *dots_container = lv_obj_create(s_scr);
    lv_obj_set_size(dots_container, 368, 26);
    lv_obj_set_pos(dots_container, 0, 412);
    lv_obj_set_style_bg_color(dots_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(dots_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dots_container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dots_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(dots_container, btn_next_view_cb, LV_EVENT_CLICKED, NULL);

    int start_x = 48;
    for (int i = 0; i < VIEW_MAX_COUNT; i++) {
        lv_obj_t *dot = lv_obj_create(dots_container);
        lv_obj_set_size(dot, (i == 0) ? 16 : 5, 5);
        lv_obj_set_pos(dot, start_x + i * 27, 10);
        lv_obj_set_style_bg_color(dot, lv_color_hex((i == 0) ? 0xFFFFFF : 0x444444), LV_PART_MAIN);
        lv_obj_set_style_radius(dot, 3, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        s_nav_dots[i] = dot;
    }

    // Set View 1 as initial view
    ui_engine_set_view(VIEW_SINE_WAVE_TRANSDUCER);
}

void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count) {
    if (!metrics) return;

    // Fast-reacting low-pass filters so physical shakes produce instant visual response
    s_smooth_rms = s_smooth_rms * 0.70f + metrics->rms_acceleration_g * 0.30f;
    s_smooth_kurt = s_smooth_kurt * 0.80f + metrics->kurtosis * 0.20f;
    s_smooth_rpm = s_smooth_rpm * 0.80f + (float)metrics->estimated_rpm * 0.20f;
    s_smooth_f0 = s_smooth_f0 * 0.80f + (metrics->peak_freq_hz > 5.0f ? metrics->peak_freq_hz : 30.0f) * 0.20f;

    char buf[64];

    // VIEW 1: ORIGINAL CLASSIC STETHOSCOPE DASHBOARD
    if (s_current_view == VIEW_SINE_WAVE_TRANSDUCER) {
        // Health Arc & Score
        lv_arc_set_value(s_health_arc, metrics->health_score);
        uint32_t state_color = (metrics->health_score >= 70) ? 0x00FF66 : (metrics->health_score >= 35) ? 0xF59E0B : 0xFF2A54;
        lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(state_color), LV_PART_INDICATOR);

        snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
        lv_label_set_text(s_health_val_label, buf);

        if (metrics->health_score >= 70) {
            lv_label_set_text(s_status_label, "STATUS: HEALTHY NOMINAL");
        } else if (metrics->health_score >= 35) {
            lv_label_set_text(s_status_label, "STATUS: WARNING ELEVATED");
        } else {
            lv_label_set_text(s_status_label, "STATUS: CRITICAL ANOMALY");
        }

        snprintf(buf, sizeof(buf), "F0: %.1f Hz (%lu RPM)", s_smooth_f0, (unsigned long)s_smooth_rpm);
        lv_label_set_text(s_f0_rpm_label, buf);

        // Real-time Oscilloscope driven directly by live IMU accelerometer buffer
        lv_obj_set_style_line_color(s_osc_chart, lv_color_hex(state_color), LV_PART_ITEMS);
        for (size_t i = 0; i < 64; i++) {
            size_t src_idx = (osc_count > 0) ? (i * osc_count) / 64 : 0;
            int32_t val = 0;
            if (osc_waveform && osc_count > 0) {
                val = (int32_t)(osc_waveform[src_idx] * 80.0f);
            }
            if (val > 120) val = 120;
            if (val < -120) val = -120;
            lv_chart_set_value_by_id(s_osc_chart, s_osc_series, i, val);
        }
        lv_chart_refresh(s_osc_chart);

        // Real-time FFT Bars
        for (int i = 0; i < BARS_COUNT; i++) {
            int32_t b_val = (int32_t)(metrics->visual_spectrum[i] * 100.0f);
            if (b_val > 100) b_val = 100;
            lv_chart_set_value_by_id(s_fft_chart, s_fft_series, i, b_val);
        }
        lv_chart_refresh(s_fft_chart);

        // Metrics
        snprintf(buf, sizeof(buf), "%.3fg", s_smooth_rms);
        lv_label_set_text(s_rms_val_label, buf);

        snprintf(buf, sizeof(buf), "%.2f", s_smooth_kurt);
        lv_label_set_text(s_kurt_val_label, buf);

        if (metrics->iso_vibration_vel < 1.12f) {
            lv_label_set_text(s_iso_val_label, "CLS A");
        } else if (metrics->iso_vibration_vel < 2.8f) {
            lv_label_set_text(s_iso_val_label, "CLS B");
        } else {
            lv_label_set_text(s_iso_val_label, "CLS D");
        }
    }

    // VIEW 2: 24-BAND FFT SPECTRUM
    else if (s_current_view == VIEW_FFT_SPECTRUM) {
        snprintf(buf, sizeof(buf), "1X Harmonic Peak: %.1f Hz", s_smooth_f0);
        lv_label_set_text(s_fft_peak_label, buf);

        for (int i = 0; i < 24; i++) {
            int val = (int)(metrics->visual_spectrum[i] * 100.0f);
            if (s_smooth_rms > 0.2f) val = (int)(val * (1.0f + s_smooth_rms * 1.5f));
            if (val > 100) val = 100;
            lv_bar_set_value(s_fft_bars[i], val, LV_ANIM_OFF);
        }
    }

    // VIEW 3: KURTOSIS IMPACT
    else if (s_current_view == VIEW_KURTOSIS_IMPACT) {
        snprintf(buf, sizeof(buf), "%.2f", s_smooth_kurt);
        lv_label_set_text(s_kurt_bold_val, buf);

        if (s_smooth_kurt > 4.5f) {
            lv_label_set_text(s_kurt_status_label, "IMPACT SHOCK DETECTED (Piston Knock)");
        } else if (s_smooth_kurt > 3.6f) {
            lv_label_set_text(s_kurt_status_label, "Elevated Kurtosis Warning");
        } else {
            lv_label_set_text(s_kurt_status_label, "Gaussian Symmetry (Zero Knock)");
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
                lv_obj_set_style_bg_color(s_matrix_dots[i], lv_color_hex(0x00220A), LV_PART_MAIN);
            }
        }
    }

    // VIEW 10: BOLD AI DIAGNOSTICS
    else if (s_current_view == VIEW_BOLD_DIAGNOSTICS) {
        lv_label_set_text(s_diag_bold_title, metrics->diagnosis_text);
        lv_label_set_text(s_diag_recom_label, metrics->recommendation);
    }
}
