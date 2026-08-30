idf_component_register(SRCS "main.c" "dsp_engine.c" "ui_engine.c"
                       INCLUDE_DIRS "."
                       REQUIRES waveshare__esp32_s3_touch_amoled_1_8 waveshare__qmi8658 lvgl__lvgl esp_driver_i2s esp_codec_dev esp_timer esp_driver_gpio esp_driver_i2c)
