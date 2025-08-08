#include <stdio.h>

#include "hardware/pwm.h"
#include "pico/stdlib.h"

static uint16_t set_pwm_pulse(uint16_t period_cycle, float cycletime,
                              float pulse_width) {
    float pwm_count_deg = (float)period_cycle / cycletime * pulse_width;
    return (uint16_t)pwm_count_deg;
};

int main() {
    stdio_init_all();
    const uint8_t M1_IN1 = 25;
    const uint8_t M1_IN2 = 1;
    const uint8_t S1 = 2;
    const uint8_t S2 = 3;

    const uint8_t SURVO = 14;

    // モーター初期化
    gpio_init(M1_IN1);
    gpio_init(M1_IN2);
    gpio_set_dir(M1_IN1, GPIO_OUT);
    gpio_set_dir(M1_IN2, GPIO_OUT);
    gpio_put(M1_IN1, 0);
    gpio_put(M1_IN2, 0);

    // サーボモータ
    static uint16_t PERIOD_CYCLE = 25000;
    static float CYCLETIME = 20.0F;
    static float PULSE_WIDTH_DEG_M60 = 0.82F;
    static float PULSE_WIDTH_DEG_P60 = 2.10F;
    uint16_t pulse1 =
        set_pwm_pulse(PERIOD_CYCLE, CYCLETIME, PULSE_WIDTH_DEG_M60);

    uint16_t pulse2 =
        set_pwm_pulse(PERIOD_CYCLE, CYCLETIME, PULSE_WIDTH_DEG_P60);

    gpio_set_function(SURVO, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SURVO);
    // 分解能を指定
    pwm_set_wrap(slice_num, 25000 - 1);
    // 分周比を指定
    pwm_set_clkdiv(slice_num, 100.0f);
    pwm_set_enabled(slice_num, true);

    // スイッチ初期化
    gpio_init(S1);
    gpio_init(S2);
    gpio_set_dir(S1, GPIO_IN);
    gpio_set_dir(S2, GPIO_IN);
    gpio_pull_up(S1);
    gpio_pull_up(S2);

    while (true) {
        if (!gpio_get(S2)) {
            pwm_set_gpio_level(SURVO, pulse1);
            sleep_ms(500);
        } else {
            pwm_set_gpio_level(SURVO, pulse2);
            sleep_ms(500);
        }
    }
    return 0;
}
