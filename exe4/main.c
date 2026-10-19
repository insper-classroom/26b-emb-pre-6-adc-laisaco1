#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;
const float conversion_factor = 3.3f / (1 << 12);

// Pro timer:
struct repeating_timer blink_timer;
bool timer_running = false;
int current_period = 0;

bool blink_callback(struct repeating_timer *t) {
    gpio_xor_mask(1u << PIN_LED_B); // alterna o LED
    return true;               
}

int main() {
    stdio_init_all();

    // Configura o LED
    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, false);

    // Configura o potenciômetro no GP27, que corresponde ao ADC 1
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);

    while (1) {
        uint16_t adc_value = adc_read();
        float voltage = adc_value * conversion_factor;

        int new_period;

        // Zona 1:
        if (voltage < 1.0f) {
            new_period = 0;       // LED apagado
        } 
        
        // Zona 2:
        else if (voltage < 2.0f) {
            new_period = 300;     // zona de 1 a 2 V
        } 
        
        // Zona 3:
        else {
            new_period = 500;     // zona de 2 a 3,3 V
        }

        // Só reinicia o timer quando o potenciômetro muda de zona:
        if (new_period != current_period) {
            if (timer_running) {
                cancel_repeating_timer(&blink_timer);

                timer_running = false;
            }

            gpio_put(PIN_LED_B, false);
            current_period = new_period;

            if (current_period > 0) {

                add_repeating_timer_ms(
                    -current_period,
                    blink_callback,
                    NULL,
                    &blink_timer
                );

                timer_running = true;
            }
        }

        tight_loop_contents();
    }
}