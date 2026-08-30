#include "ui_engine.h"
#include <stdio.h>
#include <string.h>

static lv_obj_t *s_scr;
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
static lv_obj_t *s_fps_label;

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

void ui_engine_init(void) {
    s_scr = lv_screen_active();
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0x06090F), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. TOP HEADER (Height: 32px)
    lv_obj_t *header = lv_obj_create(s_scr);
    lv_obj_set_size(header, 360, 32);
    lv_obj_set_pos(header, 4, 4);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x111622), LV_PART_MAIN);
    lv_obj_set_style_border_color(header, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 4, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "MECHA-WHISPERER");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 6, 0);

    s_fps_label = lv_label_create(header);
    lv_label_set_text(s_fps_label, "WI-FI: ON");
    lv_obj_set_style_text_color(s_fps_label, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_fps_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(s_fps_label, LV_ALIGN_RIGHT_MID, -6, 0);

    // 2. HEALTH SCORE CARD (y: 40, Height: 96px)
    lv_obj_t *health_card = lv_obj_create(s_scr);
    lv_obj_set_size(health_card, 360, 96);
    lv_obj_set_pos(health_card, 4, 40);
    lv_obj_set_style_bg_color(health_card, lv_color_hex(0x0C121E), LV_PART_MAIN);
    lv_obj_set_style_border_color(health_card, lv_color_hex(0x1F293D), LV_PART_MAIN);
    lv_obj_set_style_border_width(health_card, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(health_card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(health_card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(health_card, LV_OBJ_FLAG_SCROLLABLE);

    // Circular Health Arc
    s_health_arc = lv_arc_create(health_card);
    lv_obj_set_size(s_health_arc, 78, 78);
    lv_obj_align(s_health_arc, LV_ALIGN_LEFT_MID, 4, 0);
    lv_arc_set_range(s_health_arc, 0, 100);
    lv_arc_set_value(s_health_arc, 98);
    lv_arc_set_bg_angles(s_health_arc, 0, 360);
    lv_obj_set_style_arc_width(s_health_arc, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(0x1E2433), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_health_arc, 7, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_health_arc, LV_OBJ_FLAG_CLICKABLE);

    s_health_val_label = lv_label_create(health_card);
    lv_label_set_text(s_health_val_label, "98%");
    lv_obj_set_style_text_color(s_health_val_label, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_health_val_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_health_val_label, s_health_arc, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *title_h = lv_label_create(health_card);
    lv_label_set_text(title_h, "MACHINE HEALTH SCORE");
    lv_obj_set_style_text_color(title_h, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_h, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(title_h, 94, 6);

    s_status_label = lv_label_create(health_card);
    lv_label_set_text(s_status_label, "STATUS: HEALTHY NOMINAL");
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_status_label, 94, 30);

    s_f0_rpm_label = lv_label_create(health_card);
    lv_label_set_text(s_f0_rpm_label, "F0: 48.5 Hz (2910 RPM)");
    lv_obj_set_style_text_color(s_f0_rpm_label, lv_color_hex(0x00FF88), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_f0_rpm_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(s_f0_rpm_label, 94, 56);

    // 3. OSCILLOSCOPE CARD (y: 140, Height: 114px)
    lv_obj_t *osc_card = lv_obj_create(s_scr);
    lv_obj_set_size(osc_card, 360, 114);
    lv_obj_set_pos(osc_card, 4, 140);
    lv_obj_set_style_bg_color(osc_card, lv_color_hex(0x0A0D14), LV_PART_MAIN);
    lv_obj_set_style_border_color(osc_card, lv_color_hex(0x1F293D), LV_PART_MAIN);
    lv_obj_set_style_border_width(osc_card, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(health_card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(osc_card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(osc_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *osc_title = lv_label_create(osc_card);
    lv_label_set_text(osc_title, "MICRO-VIBRATION STETHOSCOPE");
    lv_obj_set_style_text_color(osc_title, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(osc_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(osc_title, LV_ALIGN_TOP_LEFT, 6, 2);

    s_osc_chart = lv_chart_create(osc_card);
    lv_obj_set_size(s_osc_chart, 348, 80);
    lv_obj_align(s_osc_chart, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_chart_set_type(s_osc_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_osc_chart, 64);
    lv_chart_set_range(s_osc_chart, LV_CHART_AXIS_PRIMARY_Y, -120, 120);
    lv_obj_set_style_bg_opa(s_osc_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_osc_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_osc_chart, 2, LV_PART_ITEMS);
    s_osc_series = lv_chart_add_series(s_osc_chart, lv_color_hex(0x00F0FF), LV_CHART_AXIS_PRIMARY_Y);

    // 4. FFT SPECTRUM CARD (y: 258, Height: 80px)
    lv_obj_t *fft_card = lv_obj_create(s_scr);
    lv_obj_set_size(fft_card, 360, 80);
    lv_obj_set_pos(fft_card, 4, 258);
    lv_obj_set_style_bg_color(fft_card, lv_color_hex(0x0A0D14), LV_PART_MAIN);
    lv_obj_set_style_border_color(fft_card, lv_color_hex(0x1F293D), LV_PART_MAIN);
    lv_obj_set_style_border_width(fft_card, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(fft_card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(fft_card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(fft_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *fft_title = lv_label_create(fft_card);
    lv_label_set_text(fft_title, "VIBRATION FFT SPECTRUM (0 - 500 Hz)");
    lv_obj_set_style_text_color(fft_title, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(fft_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(fft_title, LV_ALIGN_TOP_LEFT, 6, 2);

    s_fft_chart = lv_chart_create(fft_card);
    lv_obj_set_size(s_fft_chart, 348, 52);
    lv_obj_align(s_fft_chart, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_chart_set_type(s_fft_chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(s_fft_chart, BARS_COUNT);
    lv_chart_set_range(s_fft_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_opa(s_fft_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_fft_chart, 0, LV_PART_MAIN);
    s_fft_series = lv_chart_add_series(s_fft_chart, lv_color_hex(0x00F0FF), LV_CHART_AXIS_PRIMARY_Y);

    // 5. METRICS ROW (y: 342, Height: 48px)
    int card_w = 116;
    
    lv_obj_t *m1 = lv_obj_create(s_scr);
    lv_obj_set_size(m1, card_w, 48);
    lv_obj_set_pos(m1, 4, 342);
    lv_obj_set_style_bg_color(m1, lv_color_hex(0x141822), LV_PART_MAIN);
    lv_obj_set_style_border_width(m1, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(m1, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m1, 4, LV_PART_MAIN);
    lv_obj_clear_flag(m1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *l1 = lv_label_create(m1);
    lv_label_set_text(l1, "RMS ACCEL");
    lv_obj_set_style_text_color(l1, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(l1, &lv_font_montserrat_12, LV_PART_MAIN);
    s_rms_val_label = lv_label_create(m1);
    lv_label_set_text(s_rms_val_label, "0.08g");
    lv_obj_set_style_text_color(s_rms_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_rms_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_rms_val_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *m2 = lv_obj_create(s_scr);
    lv_obj_set_size(m2, card_w, 48);
    lv_obj_set_pos(m2, 124, 342);
    lv_obj_set_style_bg_color(m2, lv_color_hex(0x141822), LV_PART_MAIN);
    lv_obj_set_style_border_width(m2, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(m2, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m2, 4, LV_PART_MAIN);
    lv_obj_clear_flag(m2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *l2 = lv_label_create(m2);
    lv_label_set_text(l2, "KURTOSIS");
    lv_obj_set_style_text_color(l2, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(l2, &lv_font_montserrat_12, LV_PART_MAIN);
    s_kurt_val_label = lv_label_create(m2);
    lv_label_set_text(s_kurt_val_label, "2.9");
    lv_obj_set_style_text_color(s_kurt_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_kurt_val_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *m3 = lv_obj_create(s_scr);
    lv_obj_set_size(m3, card_w, 48);
    lv_obj_set_pos(m3, 244, 342);
    lv_obj_set_style_bg_color(m3, lv_color_hex(0x141822), LV_PART_MAIN);
    lv_obj_set_style_border_width(m3, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(m3, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m3, 4, LV_PART_MAIN);
    lv_obj_clear_flag(m3, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *l3 = lv_label_create(m3);
    lv_label_set_text(l3, "ISO-10816");
    lv_obj_set_style_text_color(l3, lv_color_hex(0x8B98AD), LV_PART_MAIN);
    lv_obj_set_style_text_font(l3, &lv_font_montserrat_12, LV_PART_MAIN);
    s_iso_val_label = lv_label_create(m3);
    lv_label_set_text(s_iso_val_label, "CLS A");
    lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0x00FF88), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_iso_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_iso_val_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // 6. BOTTOM TOUCH BUTTONS (y: 396, Height: 44px)
    int btn_w = 84;
    
    lv_obj_t *btn1 = lv_button_create(s_scr);
    lv_obj_set_size(btn1, btn_w, 42);
    lv_obj_set_pos(btn1, 4, 396);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x1A2233), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn1, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn1, 1, LV_PART_MAIN);
    lv_obj_add_event_cb(btn1, btn_calib_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t1 = lv_label_create(btn1);
    lv_label_set_text(t1, "CALIB");
    lv_obj_set_style_text_color(t1, lv_color_hex(0x00F0FF), LV_PART_MAIN);
    lv_obj_center(t1);

    lv_obj_t *btn2 = lv_button_create(s_scr);
    lv_obj_set_size(btn2, btn_w, 42);
    lv_obj_set_pos(btn2, 94, 396);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x1A2233), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn2, 1, LV_PART_MAIN);
    lv_obj_t *t2 = lv_label_create(btn2);
    lv_label_set_text(t2, "MODE");
    lv_obj_set_style_text_color(t2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(t2);

    lv_obj_t *btn3 = lv_button_create(s_scr);
    lv_obj_set_size(btn3, btn_w, 42);
    lv_obj_set_pos(btn3, 184, 396);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0x1A2233), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn3, lv_color_hex(0xFF9100), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn3, 1, LV_PART_MAIN);
    lv_obj_add_event_cb(btn3, btn_demo_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t3 = lv_label_create(btn3);
    lv_label_set_text(t3, "DEMO");
    lv_obj_set_style_text_color(t3, lv_color_hex(0xFF9100), LV_PART_MAIN);
    lv_obj_center(t3);

    lv_obj_t *btn4 = lv_button_create(s_scr);
    lv_obj_set_size(btn4, btn_w, 42);
    lv_obj_set_pos(btn4, 274, 396);
    lv_obj_set_style_bg_color(btn4, lv_color_hex(0x1A2233), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn4, lv_color_hex(0xFF00FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn4, 1, LV_PART_MAIN);
    lv_obj_t *t4 = lv_label_create(btn4);
    lv_label_set_text(t4, "AUDIO");
    lv_obj_set_style_text_color(t4, lv_color_hex(0xFF00FF), LV_PART_MAIN);
    lv_obj_center(t4);
}

void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count) {
    if (!metrics) return;

    // Determine state colors
    uint32_t state_hex = 0x00F0FF;
    if (metrics->health_score < 30) {
        state_hex = 0xFF1744; // Alert Red
    } else if (metrics->health_score < 60) {
        state_hex = 0xFF9100; // Warning Amber
    }

    lv_color_t color = lv_color_hex(state_hex);

    // Update Health Arc & Value
    lv_arc_set_value(s_health_arc, metrics->health_score);
    lv_obj_set_style_arc_color(s_health_arc, color, LV_PART_INDICATOR);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
    lv_label_set_text(s_health_val_label, buf);
    lv_obj_set_style_text_color(s_health_val_label, color, LV_PART_MAIN);

    lv_label_set_text(s_status_label, metrics->diagnosis_text);
    lv_obj_set_style_text_color(s_status_label, (metrics->health_score < 30) ? lv_color_hex(0xFF1744) : lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    snprintf(buf, sizeof(buf), "F0: %.1f Hz (%lu RPM)", metrics->peak_freq_hz, (unsigned long)metrics->estimated_rpm);
    lv_label_set_text(s_f0_rpm_label, buf);

    // Update Oscilloscope
    lv_obj_set_style_line_color(s_osc_chart, color, LV_PART_ITEMS);
    for (size_t i = 0; i < 64; i++) {
        size_t src_idx = (i * osc_count) / 64;
        int32_t val = (int32_t)(osc_waveform[src_idx] * 80.0f);
        if (val > 120) val = 120;
        if (val < -120) val = -120;
        lv_chart_set_value_by_id(s_osc_chart, s_osc_series, i, val);
    }

    // Update FFT Bars
    lv_obj_set_style_border_color(s_fft_chart, color, LV_PART_MAIN);
    for (int i = 0; i < BARS_COUNT; i++) {
        int32_t b_val = (int32_t)(metrics->visual_spectrum[i] * 100.0f);
        if (b_val > 100) b_val = 100;
        lv_chart_set_value_by_id(s_fft_chart, s_fft_series, i, b_val);
    }

    // Update Metrics
    snprintf(buf, sizeof(buf), "%.2fg", metrics->rms_acceleration_g);
    lv_label_set_text(s_rms_val_label, buf);

    snprintf(buf, sizeof(buf), "%.1f", metrics->kurtosis);
    lv_label_set_text(s_kurt_val_label, buf);
    lv_obj_set_style_text_color(s_kurt_val_label, (metrics->kurtosis > 4.0f) ? lv_color_hex(0xFF9100) : lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    if (metrics->iso_vibration_vel < 1.12f) {
        lv_label_set_text(s_iso_val_label, "CLS A");
        lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0x00FF88), LV_PART_MAIN);
    } else if (metrics->iso_vibration_vel < 2.8f) {
        lv_label_set_text(s_iso_val_label, "CLS B");
        lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0xFFD600), LV_PART_MAIN);
    } else {
        lv_label_set_text(s_iso_val_label, "CLS D");
        lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0xFF1744), LV_PART_MAIN);
    }
}
