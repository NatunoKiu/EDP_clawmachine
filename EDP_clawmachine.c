#include <stdio.h>

#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    const uint8_t M1_IN1 = 0;
    const uint8_t M1_IN2 = 1;
    const uint8_t S1 = 2;
    const uint8_t S2 = 3;

    // モーター初期化
    gpio_init(M1_IN1);
    gpio_init(M1_IN2);
    gpio_set_dir(M1_IN1, GPIO_OUT);
    gpio_set_dir(M1_IN2, GPIO_OUT);
    gpio_put(M1_IN1, 0);
    gpio_put(M1_IN2, 0);

    // スイッチ初期化
    gpio_init(S1);
    gpio_init(S2);
    gpio_set_dir(S1, GPIO_IN);
    gpio_set_dir(S2, GPIO_IN);
    gpio_pull_up(S1);
    gpio_pull_up(S2);

    while (true) {
        if (!gpio_get(S2)) {
            while (true) {
                if (!gpio_get(S1)) {
                    gpio_put(M1_IN1, 1);
                    gpio_put(M1_IN2, 0);
                } else {
                    gpio_put(M1_IN1, 0);
                    gpio_put(M1_IN2, 1);
                }
            }
        }
    }
    return 0;
}
