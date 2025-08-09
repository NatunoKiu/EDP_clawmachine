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
    const uint8_t M1_IN1 = 0;
    const uint8_t M1_IN2 = 1;
    const uint8_t M2_IN1 = 2;
    const uint8_t M2_IN2 = 3;
    const uint8_t M3_IN1 = 4;
    const uint8_t M3_IN2 = 5;
    const uint8_t S1 = 11;
    const uint8_t S2 = 15;

    const uint8_t SURVO = 6;

    // モーター初期化
    gpio_init(M1_IN1);
    gpio_init(M1_IN2);
    gpio_set_dir(M1_IN1, GPIO_OUT);
    gpio_set_dir(M1_IN2, GPIO_OUT);
    gpio_put(M1_IN1, 0);
    gpio_put(M1_IN2, 0);

    gpio_init(M2_IN1);
    gpio_init(M2_IN2);
    gpio_set_dir(M2_IN1, GPIO_OUT);
    gpio_set_dir(M2_IN2, GPIO_OUT);
    gpio_put(M2_IN1, 0);
    gpio_put(M2_IN2, 0);

    gpio_init(M3_IN1);
    gpio_init(M3_IN2);
    gpio_set_dir(M3_IN1, GPIO_OUT);
    gpio_set_dir(M3_IN2, GPIO_OUT);
    gpio_put(M3_IN1, 0);
    gpio_put(M3_IN2, 0);

    // サーボモータ初期化
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

    // フラグ初期化
    bool s1_first_press = true;
    bool s2_first_press = true;

    while (true) {
        // S1を押している間だけ、右へ動く
        if (!gpio_get(S1) && s1_first_press) {
            s1_first_press = false;
            while (true) {
                gpio_put(M1_IN1, 1);
                gpio_put(M1_IN2, 0);
                if (!gpio_get(S1) == false) {
                    break;
                }
            }
        }
        // S2を押している間だけ、奥へ動く
        if (!gpio_get(S2) && s2_first_press) {
            s2_first_press = false;
            while (true) {
                gpio_put(M2_IN1, 1);
                gpio_put(M2_IN2, 0);
                if (!gpio_get(S2) == false) {
                    break;
                }
            }
            // アームを開く
            pwm_set_gpio_level(SURVO, pulse1);
            sleep_ms(1500);
            // アームを下げる
            gpio_put(M3_IN1, 1);
            gpio_put(M3_IN2, 0);
            sleep_ms(3000);
            // アームを閉じる
            pwm_set_gpio_level(SURVO, pulse2);
            sleep_ms(1500);
            // アームを上げる
            gpio_put(M3_IN1, 0);
            gpio_put(M3_IN2, 1);
            sleep_ms(3000);
            // 手前へ動く
            gpio_put(M2_IN1, 0);
            gpio_put(M2_IN2, 1);
            sleep_ms(2000);
            // 左へ動く
            gpio_put(M1_IN1, 0);
            gpio_put(M1_IN2, 1);
            sleep_ms(2000);
            // アームを開く
            pwm_set_gpio_level(SURVO, pulse1);
            sleep_ms(1500);
        }
    }
    return 0;
}