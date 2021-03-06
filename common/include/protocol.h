

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define USBTHING_PROTOCOL_VERSION       1

#ifndef VENDOR_ID
#define VENDOR_ID   0x0001
#endif

#ifndef PRODUCT_ID
#define PRODUCT_ID  0x0001
#endif

enum usbthing_module_e {
    USBTHING_MODULE_BASE = 1,
    USBTHING_MODULE_GPIO = 2,
    USBTHING_MODULE_SPI = 3,
    USBTHING_MODULE_I2C = 4,
    USBTHING_MODULE_PWM = 5,
    USBTHING_MODULE_ADC = 6,
    USBTHING_MODULE_DAC = 7
};

enum usb_thing_cmd_e {
    USBTHING_CMD_I2C_CFG = 0xB1,
    USBTHING_CMD_I2C_TRANSFER = 0xB2,
    USBTHING_CMD_PWM_CFG = 0xC1,
    USBTHING_CMD_PWM_EN = 0xC2,
    USBTHING_CMD_PWM_SET = 0xC3,
    USBTHING_CMD_ADC_CFG = 0xD1,
    USBTHING_CMD_ADC_GET = 0xD2,
    USBTHING_CMD_DAC_CFG = 0xE1,
    USBTHING_CMD_DAC_SET = 0xE3,
    USBTHING_CMD_UART_CFG = 0xF1
};

enum usb_thing_error_e {
    USBTHING_ERROR_OK = 0,
    USBTHING_ERROR_USB_DISCONNECT = -1,
    USBTHING_ERROR_USB_TIMEOUT = -2,
    USBTHING_ERROR_PERIPHERAL_FAILED = -3,
    USBTHING_ERROR_PERIPHERAL_TIMEOUT = -4
};


/*****       Protocol Configuration         *****/
#define USBTHING_SERIAL_MAX_SIZE        32
#define USBTHING_FIRMWARE_MAX_SIZE      32
#define USBTHING_SPI_MAX_SIZE           64


/*****       Protocol Configuration         *****/
enum base_cmd_e {
    BASE_CMD_NOOP = 0,
    BASE_CMD_SERIAL_GET = 1,
    BASE_CMD_FIRMWARE_GET = 2,
    BASE_CMD_LED_SET = 3,
    BASE_CMD_RESET = 4
};

struct serial_get_s {
    uint8_t serial[USBTHING_SERIAL_MAX_SIZE];
} __attribute((packed));

struct firmware_get_s {
    uint8_t version[USBTHING_FIRMWARE_MAX_SIZE];
} __attribute((packed));

struct led_set_s {
    uint8_t pin;
    uint8_t enable;
} __attribute((packed));

struct base_cmd_s {
    union {
        struct serial_get_s serial_get;
        struct firmware_get_s firmware_get;
        struct led_set_s led_set;
    };
} __attribute((packed));

#define USBTHING_CMD_NOP_SIZE                   0
#define USBTHING_CMD_SERIAL_GET_SIZE            USBTHING_SERIAL_MAX_SIZE
#define USBTHING_CMD_FIRMWARE_GET_SIZE          USBTHING_FIRMWARE_MAX_SIZE
#define USBTHING_CMD_LED_SET_SIZE               (sizeof(struct led_set_s))
#define USBTHING_CMD_RESET_SIZE                 0

/*****       GPIO Configuration messages        *****/

enum usbthing_gpio_cmd_e {
    USBTHING_GPIO_CMD_CONFIG = 0,
    USBTHING_GPIO_CMD_GET = 1,
    USBTHING_GPIO_CMD_SET = 2
};

enum usbthing_gpio_mode_e {
    USBTHING_GPIO_MODE_INPUT = 0,
    USBTHING_GPIO_MODE_OUTPUT = 1
};

enum usbthing_gpio_level_e {
    USBTHING_GPIO_LEVEL_LOW = 0,
    USBTHING_GPIO_LEVEL_HIGH = 1,
};

enum usbthing_gpio_pull_e {
    USBTHING_GPIO_PULL_NONE = 0,
    USBTHING_GPIO_PULL_LOW = 1,
    USBTHING_GPIO_PULL_HIGH = 2
};

enum usbthing_gpio_int_e {
    USBTHING_GPIO_INT_DISABLE = 0,
    USBTHING_GPIO_INT_RISING = 1,
    USBTHING_GPIO_INT_FALLING = 2
};

struct gpio_config_s {
    uint8_t pin;
    uint8_t mode;
    uint8_t pull;
    uint8_t interrupt;
} __attribute((packed));

struct gpio_set_s {
    uint8_t pin;
    uint8_t level;
} __attribute((packed));

struct gpio_get_s {
    uint8_t level;
} __attribute((packed));

struct gpio_cmd_s {
    union {
        struct gpio_config_s config;
        struct gpio_set_s set;
        struct gpio_get_s get;
    };
} __attribute((packed));

#define USBTHING_CMD_GPIO_CFG_SIZE              (sizeof(struct gpio_config_s))
#define USBTHING_CMD_GPIO_GET_SIZE              (sizeof(struct gpio_get_s))
#define USBTHING_CMD_GPIO_SET_SIZE              (sizeof(struct gpio_set_s))

/*****      ADC Configuration messages          *****/

enum usbthing_adc_cmd_e {
    USBTHING_ADC_CMD_CONFIG = 0,
    USBTHING_ADC_CMD_ENABLE = 1,
    USBTHING_ADC_CMD_GET = 2
};

enum usbthing_adc_ref_e {
    USBTHING_ADC_REF_1V25 = 0,
    USBTHING_ADC_REF_2V5 = 1,
    USBTHING_ADC_REF_VDD = 2,
    USBTHING_ADC_REF_5VDIFF = 3
};

enum usbthing_adc_channel_e {
    USBTHING_ADC_CH0 = 0,
    USBTHING_ADC_CH1 = 1,
    USBTHING_ADC_CH2 = 2,
    USBTHING_ADC_CH3 = 3
};

struct adc_config_s {
    uint8_t ref;
} __attribute((packed));

struct adc_enable_s {
    uint8_t enable;
} __attribute((packed));

struct adc_get_s {
    uint32_t value;
} __attribute((packed));

struct adc_cmd_s {
    union {
        struct adc_config_s config;
        struct adc_enable_s enable;
        struct adc_get_s get;
    };
} __attribute((packed));

#define USBTHING_CMD_ADC_CONFIG_SIZE     (sizeof(struct adc_config_s))
#define USBTHING_CMD_ADC_ENABLE_SIZE     (sizeof(struct adc_enable_s))
#define USBTHING_CMD_ADC_GET_SIZE        (sizeof(struct adc_get_s))

/*****      DAC Configuration messages          *****/
struct dac_config_s {
    uint8_t mock;
} __attribute((packed));

struct dac_set_s {
    uint8_t enable;
    uint16_t value;
} __attribute((packed));

struct dac_cmd_s {
    union {
        struct dac_config_s config;
        struct dac_set_s set;
    };
} __attribute((packed));

#define USBTHING_CMD_DAC_CFG_SIZE (sizeof(struct dac_config_s))
#define USBTHING_CMD_DAC_SET_SIZE (sizeof(struct dac_set_s))

/*****      PWM Configuration messages          *****/
struct usbthing_pwm_config_s {
    uint32_t freq_le;
} __attribute((packed));

/*****      SPI Configuration messages          *****/
enum usbthing_spi_cmd_e {
    USBTHING_SPI_CMD_CONFIG = 0,
    USBTHING_SPI_CMD_CLOSE = 1
};

enum usbthing_spi_speed_e {
    USBTHING_SPI_SPEED_100KHZ = 100000,              //!< Standard mode (100 kbps)
    USBTHING_SPI_SPEED_400KHZ = 400000,              //!< Full mode (400 kbps)
    USBTHING_SPI_SPEED_1MHZ = 1000000,               //!< Fast mode (1 Mpbs)
    USBTHING_SPI_SPEED_5MHZ = 5000000                //!< High speed mode (3.2 Mbps)
};

enum usbthing_spi_clock_mode_e {
    USBTHING_SPI_CLOCK_MODE0 = 0,               //!< Idle low, sample leading edge
    USBTHING_SPI_CLOCK_MODE1 = 1,               //!< Idle low, sample trailing edge
    USBTHING_SPI_CLOCK_MODE2 = 2,               //!< Idle high, sample leading edge
    USBTHING_SPI_CLOCK_MODE3 = 3,               //!< Idle low, sample trailing edge
};

struct spi_config_s {
    uint32_t freq_le;
    uint8_t clk_mode;
} __attribute((packed));

struct spi_cmd_s {
    union {
        struct spi_config_s config;
    };
};

#define USBTHING_CMD_SPI_CONFIG_SIZE            (sizeof(struct spi_config_s))
#define USBTHING_CMD_SPI_CLOSE_SIZE             0

/*****      I2C Configuration messages          *****/
#define USBTHING_I2C_CFG_SPEED_SHIFT            (0)
#define USBTHING_I2C_CFG_SPEED_MASK             (0x0F << USBTHING_I2C_CFG_SPEED_SHIFT)
#define USBTHING_I2C_CFG_SIZE                   0

enum usbthing_i2c_speed_e {
    USBTHING_I2C_SPEED_STANDARD = 0,            //!< Standard mode (100 kbps)
    USBTHING_I2C_SPEED_FULL = 1,                //!< Full mode (400 kbps)
    USBTHING_I2C_SPEED_FAST = 2,                //!< Fast mode (1 Mpbs)
    USBTHING_I2C_SPEED_HIGH = 3                 //!< High speed mode (3.2 Mbps)
};

enum usbthing_i2c_transfer_mode_e {
    USBTHING_I2C_MODE_READ = 0,                 //!< Read only mode
    USBTHING_I2C_MODE_WRITE = 1,                //!< Write only mode
    USBTHING_I2C_MODE_WRITE_READ = 2            //!< Write and read mode
};

struct usbthing_i2c_cfg_s {
    uint32_t freq_le;                             //!< I2C device speed
} __attribute((packed));

struct usbthing_i2c_transfer_s {
    uint8_t mode;                               //!< I2C Transfer mode
    uint8_t address;                            //!< I2C Device address
    uint8_t num_write;                          //!< Number of bytes to write
    uint8_t num_read;                           //!< Number of bytes to read
    int8_t result;                              //!< Transaction result
} __attribute((packed));

/*****      PWM Configuration messages          *****/

enum usbthing_pwm_mode_e {
    PWM_MODE_OUTPUT = 0,
    PWM_MODE_CAPTURE = 1
};

struct pwm_cfg_cap_s {
    uint32_t timeout_ms;
    uint32_t num_waves;
};

struct pwm_cfg_out_s {
    uint32_t freq;
};

struct pwm_cfg_s {
    uint8_t mode;
    union {
        struct pwm_cfg_out_s output;
        struct pwm_cfg_cap_s capture;
    };
};

struct pwm_set_s {
    uint16_t duty_cycle;
};

struct pwm_get_s {
    uint32_t frequency;
    uint32_t duty_cycle;
};

struct pwm_cmd_s {
    union {
        struct pwm_cfg_s pwm_cfg;
    };
};

/*****      Combined control message            *****/

struct usbthing_ctrl_s {
    union {
        uint8_t data[32];
        struct base_cmd_s base_cmd;
        struct gpio_cmd_s gpio_cmd;
        struct spi_cmd_s spi_cmd;
        struct adc_cmd_s adc_cmd;
        struct dac_cmd_s dac_cmd;
    };
} __attribute((packed));

struct usbthing_data_msg_out_s {
    uint8_t block_id;
    uint8_t length;
    uint8_t data[48];
};

struct usbthing_data_msg_in_s {
    uint8_t block_id;
    uint8_t length;
    uint8_t data[48];
};

/*****      SPI data message                 *****/

enum spi_data_msg_id_e {
    SPI_DATA_MSG_ID_XFER_INIT = 1,
    SPI_DATA_MSG_ID_OUT = 2,
    SPI_DATA_MSG_ID_IN = 3,
    SPI_DATA_MSG_ID_XFER_COMPLETE = 4
};

struct usbthing_spi_data_msg_xfer_init_s {
    uint16_t bytes_out;
    uint16_t bytes_in;
};

struct usbthing_spi_data_msg_s {
    union {
        struct usbthing_spi_data_msg_xfer_init_s init;
        struct usbthing_data_msg_out_s out;
        struct usbthing_data_msg_in_s in;
    };
};

/*****      I2C data message                 *****/

enum usbthing_data_id_i2c_e {
    I2C_DATA_MSG_ID_TRANSFER_INIT = 1,
    I2C_DATA_MSG_ID_OUT = 2,
    I2C_DATA_MSG_ID_IN = 3,
    I2C_DATA_MSG_ID_TRANSFER_COMPLETE = 4
};

enum usbthing_data_i2c_xfer_mode_e {
    I2C_XFER_MODE_READ = 1,
    I2C_XFER_MODE_WRITE = 2,
    I2C_XFER_MODE_WRITEREAD = 3
};

struct usbthing_i2c_data_transfer_init_s {
    uint8_t mode;
    uint16_t bytes_out;
    uint16_t bytes_in;
};

struct usbthing_i2c_data_msg_s {
    union {
        struct usbthing_i2c_data_transfer_init_s init;
        struct usbthing_data_msg_out_s out;
        struct usbthing_data_msg_in_s in;
    };
};

/*****      Combined data message            *****/

struct usbthing_data_msg_s {
    uint8_t module;
    uint8_t id;
    union {
        uint8_t data[32];
        struct usbthing_spi_data_msg_s spi;
        struct usbthing_i2c_data_msg_s i2c;
    };
} __attribute((packed));

#endif
