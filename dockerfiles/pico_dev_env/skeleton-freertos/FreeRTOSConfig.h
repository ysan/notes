#pragma once

// 以下、FreeRTOS.hが必須としているマクロです。定義しないとコンパイルエラーに
// なります (configUSE_16_BIT_TICKSとconfigTICK_TYPE_WIDTH_IN_BITSはどちらか
// 一方でよいです)。
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configNUMBER_OF_CORES 1   // シングルコアで使う場合は1
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 256
#define configUSE_16_BIT_TICKS 0

#define configCPU_CLOCK_HZ 125000000
#define configTICK_RATE_HZ 1000
#define configTOTAL_HEAP_SIZE (128*1024)
#define configMAX_TASK_NAME_LEN 16
#define configIDLE_SHOULD_YIELD 1
#define configUSE_MUTEXES 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

// タイマーを使うための設定です。
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 3
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE
// RP2040ポートのport.cが内部でxTimerPendFunctionCallFromISRを使うため
// 実質必須です。無いとリンクエラーになります。
#define INCLUDE_xTimerPendFunctionCall 1

#define configKERNEL_INTERRUPT_PRIORITY 255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 254

#define INCLUDE_vTaskDelay 1
