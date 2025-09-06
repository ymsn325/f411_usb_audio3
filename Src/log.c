#include "log.h"
#include "usart.h"

// 現在のログレベル（デフォルト: INFO）
log_level_t current_log_level = LOG_INFO;

void log_set_level(log_level_t level) {
  current_log_level = level;
  printf_usart2("Log level set to: %d\r\n", level);
}

log_level_t log_get_level(void) { return current_log_level; }
