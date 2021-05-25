// https://stackoverflow.com/questions/5713451/is-it-safe-to-parse-a-proc-file
// https://stackoverflow.com/questions/62126052/setvbuf-not-changing-buffer-size

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "interrupts.h"
#include "cpu_stats.h"
#include "mem_stats.h"

static bool received_timer_signal = false;
static bool received_intr_signal = false;

int main()
{
  register_intr_signal_handler(&received_intr_signal);
  register_timer_signal_handler(&received_timer_signal);

  initialize_cpu_state();
  initialize_mem_state();

  printf("%s,%s\n",
	 format_cpu_stats(NULL, ","),
	 format_mem_stats(NULL, ","));

  create_and_start_timer(500);

  refresh_cpu_state();
  refresh_mem_state();

  while (1) {
    pause(); // Wait for a signal

    if (received_timer_signal) {
      refresh_cpu_state();
      refresh_mem_state();

      cpu_stats cpu_stats = get_cpu_stats();
      mem_stats mem_stats = get_mem_stats();

      printf("%s,%s\n",
	     format_cpu_stats(&cpu_stats, ","),
	     format_mem_stats(&mem_stats, ","));

      received_timer_signal = false;
    }
    if (received_intr_signal) {
      printf("terminating...\n");
      break;
    }
  }

  cleanup_cpu_state();
  cleanup_mem_state();

  exit(0);
}