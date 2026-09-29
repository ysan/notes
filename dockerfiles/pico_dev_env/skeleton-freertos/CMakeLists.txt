cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(freertos_blink C CXX ASM)
pico_sdk_init()

# pico_sdk_init() の後、add_executable() より前に置く必要があります
# (このport用のCMakeLists.txtがpico-sdkのフック機構を使って自分自身を
# 差し込むため)。
include(FreeRTOS_Kernel_import.cmake)

add_executable(freertos_blink main.c)

# FreeRTOSConfig.hを置いたディレクトリをinclude対象に加えます。
# pico-sdk側のソース(gpio.cなど)もこのターゲットに直接コンパイルされるため、
# これが無いとSDK側から見て "FreeRTOSConfig.h" が見つからずビルドが失敗します。
target_include_directories(freertos_blink PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(freertos_blink)
target_link_libraries(freertos_blink
  pico_stdlib
  FreeRTOS-Kernel
  FreeRTOS-Kernel-Heap4
)
