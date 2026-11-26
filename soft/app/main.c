#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "system.h"
#include "altera_avalon_pio_regs.h"

#define LED_COUNT 10
#define UPDATE_PERIOD_US 100000

#ifdef LED_CHASER

int main(void)
{
    for (;;) {
        for (unsigned int count = 1; count <= LED_COUNT; ++count) {
            IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, (1u << count) - 1u);
            usleep(UPDATE_PERIOD_US);
        }

        for (unsigned int count = LED_COUNT - 1; count > 1; --count) {
            IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, (1u << count) - 1u);
            usleep(UPDATE_PERIOD_US);
        }
    }
}

#else

#include "altera_avalon_i2c.h"

#define SLAVE_ADDR     0x1D
#define POWER_CTL_REG  0x2D
#define DATA_START_REG 0x32
#define MEASURE_MODE   0x08
#define Y_MAX_ABS      260

static int16_t decode_axis(const uint8_t *bytes)
{
    int32_t value = ((int32_t)bytes[1] << 8) | bytes[0];

    if (value > INT16_MAX) {
        value -= 65536;
    }

    return (int16_t)value;
}

static uint32_t spirit_level_leds(int16_t y)
{
    if (y >= 0) {
        return 0;
    }

    // Widen before negating so INT16_MIN is also handled.
    int32_t led_index = (-(int32_t)y * LED_COUNT) / Y_MAX_ABS;
    if (led_index >= LED_COUNT) {
        led_index = LED_COUNT - 1;
    }

    return 1u << led_index;
}

int main(void)
{
    uint8_t init_tx[] = {POWER_CTL_REG, MEASURE_MODE};
    uint8_t data_register = DATA_START_REG;
    uint8_t samples[6];

    IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, 0);
    ALT_AVALON_I2C_DEV_t *i2c_dev = alt_avalon_i2c_open(I2C_0_NAME);
    if (i2c_dev == NULL) {
        puts("ERREUR: Interface I2C introuvable.");
        return EXIT_FAILURE;
    }

    alt_avalon_i2c_master_target_set(i2c_dev, SLAVE_ADDR);
    if (alt_avalon_i2c_master_tx(i2c_dev, init_tx, sizeof(init_tx),
                                ALT_AVALON_I2C_NO_INTERRUPTS) != ALT_AVALON_I2C_SUCCESS) {
        puts("ERREUR: Échec de l'activation du mode mesure.");
        return EXIT_FAILURE;
    }
    puts("Mode mesure activé.");

    for (;;) {
        if (alt_avalon_i2c_master_tx_rx(i2c_dev, &data_register, sizeof(data_register),
                                       samples, sizeof(samples),
                                       ALT_AVALON_I2C_NO_INTERRUPTS) == ALT_AVALON_I2C_SUCCESS) {
            int16_t x = decode_axis(&samples[0]);
            int16_t y = decode_axis(&samples[2]);
            int16_t z = decode_axis(&samples[4]);

            IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, spirit_level_leds(y));
            printf("Accelerometer Data - X: %6d | Y: %6d | Z: %6d\n", x, y, z);
        } else {
            IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, 0);
            puts("ERREUR: Échec de la lecture I2C.");
        }

        usleep(UPDATE_PERIOD_US);
    }
}

#endif
