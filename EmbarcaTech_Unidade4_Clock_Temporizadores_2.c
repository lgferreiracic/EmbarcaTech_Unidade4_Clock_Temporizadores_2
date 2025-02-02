#include "pico/stdlib.h" // Biblioteca para uso das funções de entrada e saída padrão
#include "hardware/timer.h" // Biblioteca para uso dos temporizadores
#include <stdio.h>

#define LED_PIN_BLUE 11 // Definição do pino do LED azul
#define LED_PIN_RED 12 // Definição do pino do LED vermelho
#define LED_PIN_GREEN 13 // Definição do pino do LED verde
#define BUTTON_A_PIN 5 // Definição do pino do botão A

volatile bool sequence_active = false; // Variável para controlar a sequência de LEDs
static volatile uint32_t last_time = 0; // Variável para realizar o debouncing

// Função para desligar o LED verde e sinalizar que a sequência de LEDs foi finalizada
int64_t turn_off_callback_green(alarm_id_t id, void *user_data) {
    gpio_put(LED_PIN_GREEN, 0);
    sequence_active = false;
    return 0;
}

// Função para desligar o LED vermelho e chamar a função para desligar o LED verde após 3 segundos
int64_t turn_off_callback_red(alarm_id_t id, void *user_data) {
    gpio_put(LED_PIN_RED, 0);
    add_alarm_in_ms(3000, turn_off_callback_green, NULL, false);
    return 0;
}

// Função para desligar o LED azul e chamar a função para desligar o LED vermelho após 3 segundos
int64_t turn_off_callback_blue(alarm_id_t id, void *user_data) {
    gpio_put(LED_PIN_BLUE, 0); 
    add_alarm_in_ms(3000, turn_off_callback_red, NULL, false);
    return 0;
}

// Função para controlar o botão A 
bool button_callback(struct repeating_timer *t) {   
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual
    if (current_time - last_time > 200000 && gpio_get(BUTTON_A_PIN) == 0) { // 200 ms de debouncing
        
        last_time = current_time; // Atualiza o tempo          

        if (!sequence_active) { // Liga a sequência de LEDs e chama a função para desligá-los após 3 segundos
            sequence_active = true;
            gpio_put(LED_PIN_GREEN, 1); 
            gpio_put(LED_PIN_BLUE, 1);
            gpio_put(LED_PIN_RED, 1);

            add_alarm_in_ms(3000, turn_off_callback_blue, NULL, false);
        }
    }

    return true;
}

// Função para inicializar os LEDs
void init_leds() { 
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_put(LED_PIN_GREEN, 0);

    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_put(LED_PIN_BLUE, 0);

    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);
}

// Função para inicializar o botão A
void init_button() {
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
}

// Função principal
int main() {
    // Inicializa a comunicação serial
    stdio_init_all();
    // Inicializa os LEDs
    init_leds();
    // Inicializa o botão A
    init_button();
    // Inicializa o temporizador para controlar o botão A
    struct repeating_timer timer;
    // Inicializa o temporizador para controlar o botão A a cada 100 ms
    add_repeating_timer_ms(100, button_callback, NULL, &timer);

    while (true) {
        // Mantém o loop principal ativo
        tight_loop_contents();
    }
}