#pragma once

// usart.h または専用のヘッダーファイルに追加
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_UNDERLINE "\x1b[4m"

// 背景色
#define ANSI_BG_RED "\x1b[41m"
#define ANSI_BG_GREEN "\x1b[42m"
#define ANSI_BG_YELLOW "\x1b[43m"
#define ANSI_BG_BLUE "\x1b[44m"

// ログレベル定義
typedef enum {
  LOG_ERROR = 0,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG,
  LOG_TRACE
} log_level_t;

// カラー付きログマクロ
#define LOG_ERROR(fmt, ...)                                                    \
  printf_usart2(ANSI_COLOR_RED ANSI_BOLD "[ERROR] " ANSI_COLOR_RESET fmt,      \
                ##__VA_ARGS__)

#define LOG_WARN(fmt, ...)                                                     \
  printf_usart2(ANSI_COLOR_YELLOW "[WARN]  " ANSI_COLOR_RESET fmt,             \
                ##__VA_ARGS__)

#define LOG_INFO(fmt, ...)                                                     \
  printf_usart2(ANSI_COLOR_GREEN "[INFO]  " ANSI_COLOR_RESET fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...)                                                    \
  printf_usart2(ANSI_COLOR_CYAN "[DEBUG] " ANSI_COLOR_RESET fmt, ##__VA_ARGS__)

#define LOG_TRACE(fmt, ...)                                                    \
  printf_usart2(ANSI_COLOR_MAGENTA "[TRACE] " ANSI_COLOR_RESET fmt,            \
                ##__VA_ARGS__)

// USB専用ログ
#define USB_LOG(fmt, ...)                                                      \
  printf_usart2(ANSI_COLOR_BLUE ANSI_BOLD "[USB]   " ANSI_COLOR_RESET fmt,     \
                ##__VA_ARGS__)

#define USB_ERROR(fmt, ...)                                                    \
  printf_usart2(ANSI_COLOR_RED ANSI_BG_YELLOW                                  \
                "[USB-ERR] " ANSI_COLOR_RESET fmt,                             \
                ##__VA_ARGS__)

#define USB_SETUP(fmt, ...)                                                    \
  printf_usart2(ANSI_COLOR_CYAN ANSI_UNDERLINE                                 \
                "[SETUP] " ANSI_COLOR_RESET fmt,                               \
                ##__VA_ARGS__)

#define USB_DATA(fmt, ...)                                                     \
  printf_usart2(ANSI_COLOR_GREEN "[DATA]  " ANSI_COLOR_RESET fmt, ##__VA_ARGS__)
