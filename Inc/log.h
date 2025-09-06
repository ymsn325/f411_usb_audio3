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

// 現在のログレベル設定（デフォルト: INFO）
extern log_level_t current_log_level;

// ログレベル設定関数
void log_set_level(log_level_t level);
log_level_t log_get_level(void);

// カラー付きログマクロ（レベルチェック付き）
#define LOG_ERROR(fmt, ...)                                                    \
  do {                                                                         \
    if (current_log_level >= LOG_ERROR) {                                      \
      printf_usart2(ANSI_COLOR_RED ANSI_BOLD "[ERROR] " ANSI_COLOR_RESET fmt,  \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define LOG_WARN(fmt, ...)                                                     \
  do {                                                                         \
    if (current_log_level >= LOG_WARN) {                                       \
      printf_usart2(ANSI_COLOR_YELLOW "[WARN]  " ANSI_COLOR_RESET fmt,         \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define LOG_INFO(fmt, ...)                                                     \
  do {                                                                         \
    if (current_log_level >= LOG_INFO) {                                       \
      printf_usart2(ANSI_COLOR_GREEN "[INFO]  " ANSI_COLOR_RESET fmt,          \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define LOG_DEBUG(fmt, ...)                                                    \
  do {                                                                         \
    if (current_log_level >= LOG_DEBUG) {                                      \
      printf_usart2(ANSI_COLOR_CYAN "[DEBUG] " ANSI_COLOR_RESET fmt,           \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define LOG_TRACE(fmt, ...)                                                    \
  do {                                                                         \
    if (current_log_level >= LOG_TRACE) {                                      \
      printf_usart2(ANSI_COLOR_MAGENTA "[TRACE] " ANSI_COLOR_RESET fmt,        \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

// USB専用ログ（レベルチェック付き）
#define USB_LOG(fmt, ...)                                                      \
  do {                                                                         \
    if (current_log_level >= LOG_INFO) {                                       \
      printf_usart2(ANSI_COLOR_BLUE ANSI_BOLD "[USB]   " ANSI_COLOR_RESET fmt, \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define USB_ERROR(fmt, ...)                                                    \
  do {                                                                         \
    if (current_log_level >= LOG_ERROR) {                                      \
      printf_usart2(ANSI_COLOR_RED ANSI_BG_YELLOW                              \
                    "[USB-ERR] " ANSI_COLOR_RESET fmt,                         \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define USB_SETUP(fmt, ...)                                                    \
  do {                                                                         \
    if (current_log_level >= LOG_DEBUG) {                                      \
      printf_usart2(ANSI_COLOR_CYAN ANSI_UNDERLINE                             \
                    "[SETUP] " ANSI_COLOR_RESET fmt,                           \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define USB_DATA(fmt, ...)                                                     \
  do {                                                                         \
    if (current_log_level >= LOG_TRACE) {                                      \
      printf_usart2(ANSI_COLOR_GREEN "[DATA]  " ANSI_COLOR_RESET fmt,          \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)
