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
    const uint8_t SURVO = 6;
    const uint8_t S1 = 16;
    const uint8_t S2 = 17;
    const uint8_t SENSER1 = 18;
    const uint8_t SENSER2 = 19;
    const uint8_t SENSER3 = 20;

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
    gpio_init(SENSER1);
    gpio_init(SENSER2);
    gpio_init(SENSER3);
    gpio_set_dir(S1, GPIO_IN);
    gpio_set_dir(S2, GPIO_IN);
    gpio_set_dir(SENSER1, GPIO_IN);
    gpio_set_dir(SENSER2, GPIO_IN);
    gpio_set_dir(SENSER3, GPIO_IN);
    gpio_pull_up(S1);
    gpio_pull_up(S2);
    gpio_pull_up(SENSER1);
    gpio_pull_up(SENSER2);
    gpio_pull_up(SENSER3);

    bool s1_pressed;
    bool s2_pressed;
    bool senser1_pressed;
    bool senser2_pressed;
    bool senser3_pressed;
    bool s1_pre_pressed = false;
    bool s2_pre_pressed = false;
    bool senser1_pre_pressed = false;
    bool senser2_pre_pressed = false;
    bool senser3_pre_pressed = false;
    bool senser1_home = false;
    bool senser2_home = false;
    bool senser3_home = false;

    while (true) {
        s1_pressed = !gpio_get(S1);
        s2_pressed = !gpio_get(S2);

        if (s1_pressed) {
            // 右に動く（M1正転）
            gpio_put(M1_IN1, 1);
            gpio_put(M1_IN2, 0);
            sleep_ms(100);
        } else if (s2_pressed) {
            // 奥に動く（M2正転）
            gpio_put(M2_IN1, 1);
            gpio_put(M2_IN2, 0);
            sleep_ms(100);
        } else if (!s2_pressed && s2_pre_pressed) {
            // M2停止
            gpio_put(M2_IN1, 1);
            gpio_put(M2_IN2, 1);
            sleep_ms(1000);
            // 下に動く（M3正転）
            gpio_put(M3_IN1, 1);
            gpio_put(M3_IN2, 0);
            sleep_ms(3690);
            gpio_put(M3_IN1, 1);
            gpio_put(M3_IN2, 1);
            sleep_ms(1000);
            // アームを閉じる（SURVO正転）
            pwm_set_gpio_level(SURVO, pulse1);
            sleep_ms(500);

            // 行きの経路をたどって戻る
            while (true) {
                senser1_pressed = !gpio_get(SENSER1);
                senser2_pressed = !gpio_get(SENSER2);
                senser3_pressed = !gpio_get(SENSER3);

                if (!senser3_home) {
                    // 上に動く（M3逆転）
                    gpio_put(M3_IN1, 0);
                    gpio_put(M3_IN2, 1);
                    sleep_ms(100);
                } else if (!senser2_home) {
                    // 前に動く（M2逆転）
                    gpio_put(M2_IN1, 0);
                    gpio_put(M2_IN2, 1);
                    sleep_ms(100);
                } else if (!senser1_home) {
                    // 左に動く（M1逆転）
                    gpio_put(M1_IN1, 0);
                    gpio_put(M1_IN2, 1);
                    sleep_ms(100);
                } else if (senser1_home && senser2_home && senser3_home) {
                    // アームを開く
                    pwm_set_gpio_level(SURVO, pulse2);
                    sleep_ms(500);
                }
                if (senser3_pressed && !senser3_pre_pressed) {
                    senser3_home = true;
                    sleep_ms(1000);
                    gpio_put(M3_IN1, 1);
                    gpio_put(M3_IN2, 0);
                    sleep_ms(700);
                    // M3停止
                    gpio_put(M3_IN1, 1);
                    gpio_put(M3_IN2, 1);
                    sleep_ms(1000);
                }
                if (senser2_pressed && !senser2_pre_pressed) {
                    senser2_home = true;
                    sleep_ms(1000);
                    gpio_put(M2_IN1, 1);
                    gpio_put(M2_IN2, 0);
                    sleep_ms(400);
                    // M2停止
                    gpio_put(M2_IN1, 1);
                    gpio_put(M2_IN2, 1);
                    sleep_ms(1000);
                }
                if (senser1_pressed && !senser1_pre_pressed) {
                    senser1_home = true;
                    sleep_ms(1000);
                    gpio_put(M1_IN1, 1);
                    gpio_put(M1_IN2, 0);
                    sleep_ms(700);
                    // M1停止
                    gpio_put(M1_IN1, 1);
                    gpio_put(M1_IN2, 1);
                    sleep_ms(1000);
                }
                senser1_pre_pressed = senser1_pressed;
                senser2_pre_pressed = senser2_pressed;
                senser3_pre_pressed = senser3_pressed;
                sleep_ms(100);
            }

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