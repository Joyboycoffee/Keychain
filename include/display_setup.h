#pragma once
#include <LovyanGFX.hpp>
#include "config.h"

class LGFX_ST7789 : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789      _panel_instance;
    lgfx::Bus_SPI           _bus_instance;
    lgfx::Light_PWM         _light_instance;

public:
    LGFX_ST7789() {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 3;           // Mode 3 is REQUIRED for ST7789 displays without CS pin!
            cfg.freq_write = 20000000;  // 20MHz safe write frequency
            cfg.freq_read  = 16000000;
            cfg.spi_3wire = false;      // DC pin is used
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = PIN_TFT_SCL; // GPIO 6
            cfg.pin_mosi = PIN_TFT_SDA; // GPIO 10
            cfg.pin_miso = -1;
            cfg.pin_dc   = PIN_TFT_DC;  // GPIO 3
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = -1;          // No CS pin on 7-pin module
            cfg.pin_rst          = PIN_TFT_RES; // GPIO 5
            cfg.pin_busy         = -1;
            cfg.panel_width      = 240;
            cfg.panel_height     = 240;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = false;
            cfg.invert           = true;        // ST7789 IPS inversion
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = false;
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = PIN_TFT_BL; // GPIO 2
            cfg.invert = false;
            cfg.freq   = 44100;
            cfg.pwm_channel = 0;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};

extern LGFX_ST7789 tft;
extern LGFX_Sprite canvas;
