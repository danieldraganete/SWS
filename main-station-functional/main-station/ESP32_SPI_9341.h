#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// Pin definitions for the LCD display
#define LCD_MOSI 13
#define LCD_MISO 12
#define LCD_SCK 14
#define LCD_CS 15
#define LCD_RST -1
#define LCD_DC 2
#define LCD_BL 21

// Pin definitions for the touch screen
#define TOUCH_MOSI 32
#define TOUCH_MISO 39
#define TOUCH_SCK 25
#define TOUCH_CS 33
#define TOUCH_IRQ 36

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel_instance;  // Display panel instance (ILI9341)
    lgfx::Bus_SPI _bus_instance;          // SPI bus instance
    lgfx::Touch_XPT2046 _touch_instance;  // Touch screen controller instance (XPT2046)
    lgfx::Light_PWM _light_instance;      // PWM backlight instance

public:
    LGFX(void) {
        { 
            // SPI Bus configuration
            auto cfg = _bus_instance.config(); // Get the configuration structure for the bus

            // SPI bus settings
            cfg.spi_host = HSPI_HOST;  // Select the SPI host to use (HSPI_HOST or VSPI_HOST)
            cfg.spi_mode = 0;          // Set the SPI communication mode (0 ~ 3)
            cfg.freq_write = 40000000; // SPI clock frequency for writing (up to 80MHz)
            cfg.freq_read = 16000000;  // SPI clock frequency for reading
            cfg.spi_3wire = true;      // Set to true if using 3-wire SPI (MOSI used for both send and receive)
            cfg.use_lock = true;       // Set to true to use transaction lock
            cfg.dma_channel = 1;       // Set the DMA channel (1 or 2; 0 = disable)
            cfg.pin_sclk = LCD_SCK;    // Set the pin for SPI clock (SCLK)
            cfg.pin_mosi = LCD_MOSI;   // Set the pin for SPI MOSI
            cfg.pin_miso = LCD_MISO;   // Set the pin for SPI MISO (-1 = disable)
            cfg.pin_dc = LCD_DC;       // Set the pin for Data/Command (-1 = disable)

            _bus_instance.config(cfg);              // Apply the configuration to the bus
            _panel_instance.setBus(&_bus_instance); // Set the bus instance to the panel
        }

        { 
            // Display panel control settings
            auto cfg = _panel_instance.config(); // Get the configuration structure for the display panel

            cfg.pin_cs = LCD_CS;   // Pin number where the CS is connected (-1 = disable)
            cfg.pin_rst = LCD_RST; // Pin number where the RST is connected (-1 = disable)
            cfg.pin_busy = -1;     // Pin number where the BUSY is connected (-1 = disable)

            // General settings for the display panel
            cfg.memory_width = 240;   // Maximum width supported by the driver IC
            cfg.memory_height = 320;  // Maximum height supported by the driver IC
            cfg.panel_width = 240;    // Actual displayable width
            cfg.panel_height = 320;   // Actual displayable height
            cfg.offset_x = 0;         // Offset in the X direction
            cfg.offset_y = 0;         // Offset in the Y direction
            cfg.offset_rotation = 0;  // Offset value for rotation 0~7 (4~7 are for upside-down)
            cfg.dummy_read_pixel = 8; // Number of dummy read bits before pixel reading
            cfg.dummy_read_bits = 1;  // Number of dummy read bits before reading non-pixel data
            cfg.readable = true;      // Set to true if the panel can read data
            cfg.invert = false;       // Set to true if the display colors are inverted
            cfg.rgb_order = false;    // Set to true if red and blue colors are swapped
            cfg.dlen_16bit = false;   // Set to true if the panel sends data in 16-bit length
            cfg.bus_shared = true;    // Set to true if the bus is shared with the SD card

            _panel_instance.config(cfg); // Apply the configuration to the panel
        }

        { 
            // Backlight control settings (remove if not needed)
            auto cfg = _light_instance.config(); // Get the configuration structure for backlight

            cfg.pin_bl = LCD_BL; // Pin number where the backlight is connected
            cfg.invert = false;  // Set to true if backlight brightness should be inverted
            cfg.freq = 44100;    // PWM frequency for the backlight
            cfg.pwm_channel = 7; // PWM channel number to use

            _light_instance.config(cfg);                 // Apply the configuration for backlight
            _panel_instance.setLight(&_light_instance);  // Set the backlight instance to the panel
        }

        { 
            // Touch screen control settings (remove if not needed)
            auto cfg = _touch_instance.config(); // Get the configuration structure for touch screen

            cfg.x_min = 0;           // Minimum X value from the touch screen (raw value)
            cfg.x_max = 239;         // Maximum X value from the touch screen (raw value)
            cfg.y_min = 0;           // Minimum Y value from the touch screen (raw value)
            cfg.y_max = 319;         // Maximum Y value from the touch screen (raw value)
            cfg.pin_int = TOUCH_IRQ; // Pin number where the INT is connected
            cfg.bus_shared = true;   // Set to true if the bus is shared with the display
            cfg.offset_rotation = 0; // Adjust for alignment of display and touch orientation (0~7)

            // SPI connection settings for the touch screen
            cfg.spi_host = VSPI_HOST;  // Select the SPI host (HSPI_HOST or VSPI_HOST)
            cfg.freq = 1000000;        // Set the SPI clock frequency for the touch controller
            cfg.pin_sclk = TOUCH_SCK;  // Pin number where SCLK is connected
            cfg.pin_mosi = TOUCH_MOSI; // Pin number where MOSI is connected
            cfg.pin_miso = TOUCH_MISO; // Pin number where MISO is connected
            cfg.pin_cs = TOUCH_CS;     // Pin number where CS is connected

            _touch_instance.config(cfg);                 // Apply the configuration for touch screen
            _panel_instance.setTouch(&_touch_instance);  // Set the touch instance to the panel
        }

        setPanel(&_panel_instance); // Set the panel in use
    }
};
