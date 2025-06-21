#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define N 4

int M1[N][N] = {
    {1, 2, 3, 4},
    {4, 3, 2, 1},
    {1, 3, 2, 4},
    {4, 1, 3, 2}
};
int M2[N][N] = {
    {1, 0, 0, 1},
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 1, 1}
};
int M3[N][N] = {0};
int sum = 0;

volatile int next_row = 0;
volatile int phase = 0;

void multiply_task(void *pvParameters) {
    while (phase == 0) {
        int row = -1;

        if (next_row < N) {
            row = next_row;
            next_row++;
        }

        if (row != -1) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < N; k++) {
                    M3[row][j] += M1[row][k] * M2[k][j];
                }
            }
            printf("Task %s calculated row %d\n", pcTaskGetName(NULL), row);
        } else {
            phase = 1;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void sum_task(void *pvParameters) {
    while (phase == 1) {
        static int i = 0, j = 0;
        static int local_sum = 0;

        if (i < N) {
            local_sum += M3[i][j];
            j++;
            if (j == N) {
                j = 0;
                i++;
            }
        } else {
            sum += local_sum;
            printf("Total sum: %d\n", sum);
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void app_main() {
    xTaskCreate(&multiply_task, "TaskA", 2048, NULL, 5, NULL);
    xTaskCreate(&multiply_task, "TaskB", 2048, NULL, 5, NULL);

    vTaskDelay(pdMS_TO_TICKS(500));
    xTaskCreate(&sum_task, "SumA", 2048, NULL, 5, NULL);
    xTaskCreate(&sum_task, "SumB", 2048, NULL, 5, NULL);
}
