#pragma once
#include <Arduino.h>

/**
 * MECHA-WHISPERER: The Stethoscope for Machines
 * Hardware Pin Configurations & Board Definitions
 * Supports Waveshare ESP32-S3-Touch-AMOLED-1.8 (V1 and V2 Revisions)
 */

// ==========================================
// DISPLAY CONFIGURATION (368 x 448 AMOLED)
// ==========================================
#define LCD_WIDTH       368
#define LCD_HEIGHT      448

// QSPI Display Bus Pins
#define LCD_SDIO0       4
#define LCD_SDIO1       5
#define LCD_SDIO2       6
#define LCD_SDIO3       7
#define LCD_SCLK        11
#define LCD_CS          12

// ==========================================
// I2C BUS (PMU, IMU, RTC, Touch, Expander)
// ==========================================
#define IIC_SDA         15
#define IIC_SCL         14
#define IIC_FREQ_HZ     400000

// I2C Device Addresses
#define IO_EXPANDER_ADDR        0x20    // Adafruit XCA9554 / TCA9554
#define PMU_AXP2101_ADDR        0x34    // AXP2101 PMU
#define IMU_QMI8658_ADDR_L      0x6B    // QMI8658 Primary
#define IMU_QMI8658_ADDR_H      0x6A    // QMI8658 Alternate
#define RTC_PCF85063_ADDR       0x51    // PCF85063A RTC
#define CODEC_ES8311_ADDR       0x18    // ES8311 Audio Codec
#define TOUCH_FT3168_ADDR       0x38    // V1 Touch Controller
#define TOUCH_CST820_ADDR       0x15    // V2 Touch Controller

// ==========================================
// TOUCH INTERRUPT PIN
// ==========================================
#define TP_INT          21

// ==========================================
// AUDIO CODEC (ES8311) & SPEAKER AMPLIFIER
// ==========================================
#define I2S_MCK_IO      16      // Master Clock
#define I2S_BCK_IO      9       // Bit Clock (SCLK)
#define I2S_WS_IO       45      // Word Select / LCLK
#define I2S_DI_IO       10      // Data In (Mic Audio)
#define I2S_DO_IO       8       // Data Out (Speaker)
#define POWER_AMP_PA_IO 46      // Speaker Amplifier Enable (Active High)

// Audio Sampling Parameters
#define AUDIO_SAMPLE_RATE       16000   // 16 kHz
#define AUDIO_BUFFER_SAMPLES    512     // 512 samples per buffer (~32ms)

// ==========================================
// IMU SAMPLING PARAMETERS
// ==========================================
#define IMU_SAMPLE_RATE_HZ      1000    // 1000 Hz ODR for micro-vibration analysis
#define IMU_FIFO_SIZE           256     // FFT window size (256-point FFT)
#define IMU_UPDATE_INTERVAL_MS  20      // 50 Hz UI refresh

// ==========================================
// IO EXPANDER PIN MAPPING (XCA9554)
// ==========================================
#define EXPANDER_PIN_LCD_RST    1       // LCD Reset
#define EXPANDER_PIN_TP_RST     2       // Touchscreen Reset
#define EXPANDER_PIN_PMU_IRQ    5       // PMU IRQ
#define EXPANDER_PIN_SD_CS      7       // MicroSD Chip Select

// ==========================================
// COLOR PALETTE FOR AMOLED (RGB565 & HEX)
// Cyberpunk / Medical Diagnostic HUD
// ==========================================
#define COLOR_BLACK         0x0000      // True AMOLED Black
#define COLOR_CYAN          0x07FF      // #00FFFF Electric Cyan (Nominal/Healthy)
#define COLOR_NEON_GREEN    0x07E0      // #00FF00 Emerald Green
#define COLOR_ALERT_RED     0xF800      // #FF0000 Aggressive Alert Red
#define COLOR_ALERT_ORANGE  0xFD20      // #FFA500 Diagnostic Warning Orange
#define COLOR_AMBER         0xFDE0      // #FFCC00 Cautious Amber
#define COLOR_MAGENTA       0xF81F      // #FF00FF Acoustic Noise Trace
#define COLOR_DEEP_BLUE     0x001F      // #0000FF Dark Blue
#define COLOR_DARK_GRAY     0x2104      // #212121 Subtle grid lines
#define COLOR_MID_GRAY      0x4208      // #424242 Card background
#define COLOR_LIGHT_GRAY    0x8410      // #848484 Inactive text
#define COLOR_WHITE         0xFFFF      // #FFFFFF Crisp text
