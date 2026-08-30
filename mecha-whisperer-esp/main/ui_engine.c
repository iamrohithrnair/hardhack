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
static lv_obj_t *s_status_pill_label;

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
    // Warm Champagne / Bone Cream Background matching design palette (#F5EFE6)
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(0xF5EFE6), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. TOP HEADER (Height: 34px) - Clean Luxury Navigation Capsule
    lv_obj_t *header = lv_obj_create(s_scr);
    lv_obj_set_size(header, 356, 34);
    lv_obj_set_pos(header, 6, 6);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(header, lv_color_hex(0xE5DEC9), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 17, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 4, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Gold Dot Brand Accent
    lv_obj_t *dot = lv_obj_create(header);
    lv_obj_set_size(dot, 8, 8);
    lv_obj_align(dot, LV_ALIGN_LEFT_MID, 6, 0);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(dot, 4, LV_PART_MAIN);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "MechaWhisperer");
    lv_obj_set_style_text_color(title, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 20, 0);

    // Dark Charcoal Status Pill
    lv_obj_t *status_pill = lv_obj_create(header);
    lv_obj_set_size(status_pill, 90, 24);
    lv_obj_align(status_pill, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_bg_color(status_pill, lv_color_hex(0x1C1F26), LV_PART_MAIN);
    lv_obj_set_style_border_width(status_pill, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(status_pill, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_pill, 0, LV_PART_MAIN);
    lv_obj_clear_flag(status_pill, LV_OBJ_FLAG_SCROLLABLE);

    s_status_pill_label = lv_label_create(status_pill);
    lv_label_set_text(s_status_pill_label, "LIVE · 250Hz");
    lv_obj_set_style_text_color(s_status_pill_label, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_status_pill_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(s_status_pill_label);

    // 2. HERO HEALTH SCORE CARD (y: 44, Height: 98px) - Pure White Luxury Card
    lv_obj_t *health_card = lv_obj_create(s_scr);
    lv_obj_set_size(health_card, 356, 98);
    lv_obj_set_pos(health_card, 6, 44);
    lv_obj_set_style_bg_color(health_card, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(health_card, lv_color_hex(0xE8E2D5), LV_PART_MAIN);
    lv_obj_set_style_border_width(health_card, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(health_card, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(health_card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(health_card, LV_OBJ_FLAG_SCROLLABLE);

    // Circular Sunglow Gold Radial Gauge
    s_health_arc = lv_arc_create(health_card);
    lv_obj_set_size(s_health_arc, 80, 80);
    lv_obj_align(s_health_arc, LV_ALIGN_LEFT_MID, 4, 0);
    lv_arc_set_range(s_health_arc, 0, 100);
    lv_arc_set_value(s_health_arc, 98);
    lv_arc_set_bg_angles(s_health_arc, 0, 360);
    lv_obj_set_style_arc_width(s_health_arc, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(0xF2EDE4), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_health_arc, 7, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_health_arc, lv_color_hex(0xF5C544), LV_PART_INDICATOR);
    lv_obj_clear_flag(s_health_arc, LV_OBJ_FLAG_CLICKABLE);

    s_health_val_label = lv_label_create(health_card);
    lv_label_set_text(s_health_val_label, "98%");
    lv_obj_set_style_text_color(s_health_val_label, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_health_val_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(s_health_val_label, s_health_arc, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *title_h = lv_label_create(health_card);
    lv_label_set_text(title_h, "EQUIPMENT HEALTH");
    lv_obj_set_style_text_color(title_h, lv_color_hex(0x7E869E), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_h, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(title_h, 96, 6);

    s_status_label = lv_label_create(health_card);
    lv_label_set_text(s_status_label, "NOMINAL HARMONIC");
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(s_status_label, 96, 28);

    // Warm Muted Capsule for F0 / RPM
    lv_obj_t *f0_pill = lv_obj_create(health_card);
    lv_obj_set_size(f0_pill, 240, 26);
    lv_obj_set_pos(f0_pill, 96, 56);
    lv_obj_set_style_bg_color(f0_pill, lv_color_hex(0xF4EDE2), LV_PART_MAIN);
    lv_obj_set_style_border_width(f0_pill, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(f0_pill, 13, LV_PART_MAIN);
    lv_obj_set_style_pad_all(f0_pill, 0, LV_PART_MAIN);
    lv_obj_clear_flag(f0_pill, LV_OBJ_FLAG_SCROLLABLE);

    s_f0_rpm_label = lv_label_create(f0_pill);
    lv_label_set_text(s_f0_rpm_label, "F0: 48.5 Hz · 2,910 RPM");
    lv_obj_set_style_text_color(s_f0_rpm_label, lv_color_hex(0x8C6B10), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_f0_rpm_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(s_f0_rpm_label);

    // 3. CONTRAST OBSIDIAN TRANSDUCER & FFT CARD (y: 146, Height: 188px)
    lv_obj_t *transducer_card = lv_obj_create(s_scr);
    lv_obj_set_size(transducer_card, 356, 188);
    lv_obj_set_pos(transducer_card, 6, 146);
    lv_obj_set_style_bg_color(transducer_card, lv_color_hex(0x1C1F26), LV_PART_MAIN);
    lv_obj_set_style_border_color(transducer_card, lv_color_hex(0x2E3440), LV_PART_MAIN);
    lv_obj_set_style_border_width(transducer_card, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(transducer_card, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(transducer_card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(transducer_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *osc_title = lv_label_create(transducer_card);
    lv_label_set_text(osc_title, "MICRO-VIBRATION TRANSDUCER");
    lv_obj_set_style_text_color(osc_title, lv_color_hex(0x9CA3AF), LV_PART_MAIN);
    lv_obj_set_style_text_font(osc_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(osc_title, LV_ALIGN_TOP_LEFT, 6, 2);

    lv_obj_t *gain_badge = lv_label_create(transducer_card);
    lv_label_set_text(gain_badge, "GAIN: 8X");
    lv_obj_set_style_text_color(gain_badge, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_text_font(gain_badge, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(gain_badge, LV_ALIGN_TOP_RIGHT, -6, 2);

    // Live Oscilloscope Chart
    s_osc_chart = lv_chart_create(transducer_card);
    lv_obj_set_size(s_osc_chart, 344, 76);
    lv_obj_set_pos(s_osc_chart, 0, 22);
    lv_chart_set_type(s_osc_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_osc_chart, 64);
    lv_chart_set_range(s_osc_chart, LV_CHART_AXIS_PRIMARY_Y, -120, 120);
    lv_obj_set_style_bg_opa(s_osc_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_osc_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_osc_chart, 2, LV_PART_ITEMS);
    s_osc_series = lv_chart_add_series(s_osc_chart, lv_color_hex(0x38BDF8), LV_CHART_AXIS_PRIMARY_Y);

    // Divider Line
    lv_obj_t *divider = lv_obj_create(transducer_card);
    lv_obj_set_size(divider, 344, 1);
    lv_obj_set_pos(divider, 0, 102);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0x2E3440), LV_PART_MAIN);
    lv_obj_set_style_border_width(divider, 0, LV_PART_MAIN);

    // 24-Band FFT Spectral Chart
    lv_obj_t *fft_title = lv_label_create(transducer_card);
    lv_label_set_text(fft_title, "FFT SPECTRUM (0 - 500 Hz)");
    lv_obj_set_style_text_color(fft_title, lv_color_hex(0x9CA3AF), LV_PART_MAIN);
    lv_obj_set_style_text_font(fft_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_pos(fft_title, 6, 108);

    s_fft_chart = lv_chart_create(transducer_card);
    lv_obj_set_size(s_fft_chart, 344, 52);
    lv_obj_align(s_fft_chart, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_chart_set_type(s_fft_chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(s_fft_chart, BARS_COUNT);
    lv_chart_set_range(s_fft_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_opa(s_fft_chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_fft_chart, 0, LV_PART_MAIN);
    s_fft_series = lv_chart_add_series(s_fft_chart, lv_color_hex(0xF5C544), LV_CHART_AXIS_PRIMARY_Y);

    // 4. METRICS ROW (y: 338, Height: 50px) - 3 Clean White Boxes
    int card_w = 114;
    
    // Metric 1: RMS Accel
    lv_obj_t *m1 = lv_obj_create(s_scr);
    lv_obj_set_size(m1, card_w, 50);
    lv_obj_set_pos(m1, 6, 338);
    lv_obj_set_style_bg_color(m1, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(m1, lv_color_hex(0xE8E2D5), LV_PART_MAIN);
    lv_obj_set_style_border_width(m1, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(m1, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m1, 4, LV_PART_MAIN);
    lv_obj_clear_flag(m1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l1 = lv_label_create(m1);
    lv_label_set_text(l1, "RMS ACCEL");
    lv_obj_set_style_text_color(l1, lv_color_hex(0x7E869E), LV_PART_MAIN);
    lv_obj_set_style_text_font(l1, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(l1, LV_ALIGN_TOP_LEFT, 2, 0);

    s_rms_val_label = lv_label_create(m1);
    lv_label_set_text(s_rms_val_label, "0.082g");
    lv_obj_set_style_text_color(s_rms_val_label, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_rms_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_rms_val_label, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    // Metric 2: Kurtosis
    lv_obj_t *m2 = lv_obj_create(s_scr);
    lv_obj_set_size(m2, card_w, 50);
    lv_obj_set_pos(m2, 126, 338);
    lv_obj_set_style_bg_color(m2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(m2, lv_color_hex(0xE8E2D5), LV_PART_MAIN);
    lv_obj_set_style_border_width(m2, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(m2, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m2, 4, LV_PART_MAIN);
    lv_obj_clear_flag(m2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l2 = lv_label_create(m2);
    lv_label_set_text(l2, "KURTOSIS");
    lv_obj_set_style_text_color(l2, lv_color_hex(0x7E869E), LV_PART_MAIN);
    lv_obj_set_style_text_font(l2, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(l2, LV_ALIGN_TOP_LEFT, 2, 0);

    s_kurt_val_label = lv_label_create(m2);
    lv_label_set_text(s_kurt_val_label, "2.94");
    lv_obj_set_style_text_color(s_kurt_val_label, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_kurt_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_kurt_val_label, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    // Metric 3: ISO-10816
    lv_obj_t *m3 = lv_obj_create(s_scr);
    lv_obj_set_size(m3, card_w, 50);
    lv_obj_set_pos(m3, 246, 338);
    lv_obj_set_style_bg_color(m3, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(m3, lv_color_hex(0xE8E2D5), LV_PART_MAIN);
    lv_obj_set_style_border_width(m3, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(m3, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m3, 4, LV_PART_MAIN);
    lv_obj_clear_flag(m3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l3 = lv_label_create(m3);
    lv_label_set_text(l3, "ISO-10816");
    lv_obj_set_style_text_color(l3, lv_color_hex(0x7E869E), LV_PART_MAIN);
    lv_obj_set_style_text_font(l3, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(l3, LV_ALIGN_TOP_LEFT, 2, 0);

    s_iso_val_label = lv_label_create(m3);
    lv_label_set_text(s_iso_val_label, "CLS A");
    lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0x10B981), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_iso_val_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_iso_val_label, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    // 5. BOTTOM TOUCH ACTION PILLS (y: 394, Height: 46px) - Matching bottom capsules
    int btn_w = 82;
    
    // CALIB Button (Sunglow Gold)
    lv_obj_t *btn1 = lv_button_create(s_scr);
    lv_obj_set_size(btn1, btn_w, 44);
    lv_obj_set_pos(btn1, 6, 394);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn1, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn1, 22, LV_PART_MAIN);
    lv_obj_add_event_cb(btn1, btn_calib_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t1 = lv_label_create(btn1);
    lv_label_set_text(t1, "CALIB");
    lv_obj_set_style_text_color(t1, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(t1, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t1);

    // MODE Button (Dark Charcoal)
    lv_obj_t *btn2 = lv_button_create(s_scr);
    lv_obj_set_size(btn2, btn_w, 44);
    lv_obj_set_pos(btn2, 94, 394);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x1C1F26), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn2, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn2, 22, LV_PART_MAIN);
    lv_obj_t *t2 = lv_label_create(btn2);
    lv_label_set_text(t2, "MODE");
    lv_obj_set_style_text_color(t2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(t2, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t2);

    // DEMO Button (Soft Warm Cream)
    lv_obj_t *btn3 = lv_button_create(s_scr);
    lv_obj_set_size(btn3, btn_w, 44);
    lv_obj_set_pos(btn3, 182, 394);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0xEDE3D4), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn3, lv_color_hex(0xD8CBB8), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn3, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(btn3, 22, LV_PART_MAIN);
    lv_obj_add_event_cb(btn3, btn_demo_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *t3 = lv_label_create(btn3);
    lv_label_set_text(t3, "DEMO");
    lv_obj_set_style_text_color(t3, lv_color_hex(0x78350F), LV_PART_MAIN);
    lv_obj_set_style_text_font(t3, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t3);

    // AUDIO Button (Pure White Pill)
    lv_obj_t *btn4 = lv_button_create(s_scr);
    lv_obj_set_size(btn4, btn_w, 44);
    lv_obj_set_pos(btn4, 270, 394);
    lv_obj_set_style_bg_color(btn4, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn4, lv_color_hex(0xF5C544), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn4, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(btn4, 22, LV_PART_MAIN);
    lv_obj_t *t4 = lv_label_create(btn4);
    lv_label_set_text(t4, "AUDIO");
    lv_obj_set_style_text_color(t4, lv_color_hex(0x12141A), LV_PART_MAIN);
    lv_obj_set_style_text_font(t4, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(t4);
}

void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count) {
    if (!metrics) return;

    // Determine state colors
    uint32_t state_hex = 0xF5C544; // Sunglow Gold for healthy
    if (metrics->health_score < 40) {
        state_hex = 0xF43F5E; // Alert Rose/Red
    } else if (metrics->health_score < 70) {
        state_hex = 0xF59E0B; // Warning Amber
    }

    lv_color_t color = lv_color_hex(state_hex);

    // Update Health Arc & Value
    lv_arc_set_value(s_health_arc, metrics->health_score);
    lv_obj_set_style_arc_color(s_health_arc, color, LV_PART_INDICATOR);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", metrics->health_score);
    lv_label_set_text(s_health_val_label, buf);

    lv_label_set_text(s_status_label, metrics->diagnosis_text);
    lv_obj_set_style_text_color(s_status_label, (metrics->health_score < 40) ? lv_color_hex(0xF43F5E) : lv_color_hex(0x12141A), LV_PART_MAIN);

    snprintf(buf, sizeof(buf), "F0: %.1f Hz · %lu RPM", metrics->peak_freq_hz, (unsigned long)metrics->estimated_rpm);
    lv_label_set_text(s_f0_rpm_label, buf);

    // Update Oscilloscope
    lv_obj_set_style_line_color(s_osc_chart, (metrics->health_score < 40) ? lv_color_hex(0xF43F5E) : lv_color_hex(0x38BDF8), LV_PART_ITEMS);
    for (size_t i = 0; i < 64; i++) {
        size_t src_idx = (i * osc_count) / 64;
        int32_t val = (int32_t)(osc_waveform[src_idx] * 120.0f);
        if (val > 120) val = 120;
        if (val < -120) val = -120;
        lv_chart_set_value_by_id(s_osc_chart, s_osc_series, i, val);
    }

    // Update FFT Bars
    for (int i = 0; i < BARS_COUNT; i++) {
        int32_t b_val = (int32_t)(metrics->visual_spectrum[i] * 100.0f);
        if (b_val > 100) b_val = 100;
        lv_chart_set_value_by_id(s_fft_chart, s_fft_series, i, b_val);
    }

    // Update Metrics
    snprintf(buf, sizeof(buf), "%.3fg", metrics->rms_acceleration_g);
    lv_label_set_text(s_rms_val_label, buf);

    snprintf(buf, sizeof(buf), "%.2f", metrics->kurtosis);
    lv_label_set_text(s_kurt_val_label, buf);
    lv_obj_set_style_text_color(s_kurt_val_label, (metrics->kurtosis > 4.0f) ? lv_color_hex(0xF43F5E) : lv_color_hex(0x12141A), LV_PART_MAIN);

    if (metrics->iso_vibration_vel < 1.12f) {
        lv_label_set_text(s_iso_val_label, "CLS A");
        lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0x10B981), LV_PART_MAIN);
    } else if (metrics->iso_vibration_vel < 2.8f) {
        lv_label_set_text(s_iso_val_label, "CLS B");
        lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0xF59E0B), LV_PART_MAIN);
    } else {
        lv_label_set_text(s_iso_val_label, "CLS D");
        lv_obj_set_style_text_color(s_iso_val_label, lv_color_hex(0xF43F5E), LV_PART_MAIN);
    }
}
