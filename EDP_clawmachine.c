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
    const uint8_t S3 = 16;

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
    gpio_init(S3);
    gpio_set_dir(S1, GPIO_IN);
    gpio_set_dir(S2, GPIO_IN);
    gpio_set_dir(S3, GPIO_IN);
    gpio_pull_up(S1);
    gpio_pull_up(S2);
    gpio_pull_up(S3);

    bool s1_pressed;
    bool s2_pressed;
    bool s3_pressed;
    bool s1_pre_pressed = false;
    bool s2_pre_pressed = false;
    while (true) {
        s1_pressed = !gpio_get(S1);
        s2_pressed = !gpio_get(S2);
        s3_pressed = !gpio_get(S3);

        if (s1_pressed) {
            // M1正転
            gpio_put(M1_IN1, 1);
            gpio_put(M1_IN2, 0);
            // M2停止
            gpio_put(M2_IN1, 1);
            gpio_put(M2_IN2, 1);
        } else if (s2_pressed) {
            // M1停止
            gpio_put(M1_IN1, 1);
            gpio_put(M1_IN2, 1);
            // M2正転
            gpio_put(M2_IN1, 1);
            gpio_put(M2_IN2, 0);
        } else if (s3_pressed) {
            // M1逆転
            gpio_put(M1_IN1, 0);
            gpio_put(M1_IN2, 1);
            // M2逆転
            gpio_put(M2_IN1, 0);
            gpio_put(M2_IN2, 1);
        } else if (!s2_pressed && s2_pre_pressed) {
            // M2停止
            gpio_put(M2_IN1, 1);
            gpio_put(M2_IN2, 1);
            sleep_ms(1000);
            // M3正転
            gpio_put(M3_IN1, 1);
            gpio_put(M3_IN2, 0);
            sleep_ms(3890);
            // M3停止
            gpio_put(M3_IN1, 1);
            gpio_put(M3_IN2, 1);
        } else {
            // M1停止
            gpio_put(M1_IN1, 1);
            gpio_put(M1_IN2, 1);
            // M2停止
            gpio_put(M2_IN1, 1);
            gpio_put(M2_IN2, 1);
        }

        s1_pre_pressed = s1_pressed;
        s2_pre_pressed = s2_pressed;
        sleep_ms(100);
    }
    return 0;
}