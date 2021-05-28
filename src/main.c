#define _POSIX_C_SOURCE 200112L

// https://stackoverflow.com/questions/5713451/is-it-safe-to-parse-a-proc-file
// https://stackoverflow.com/questions/62126052/setvbuf-not-changing-buffer-size

#include <bits/posix1_lim.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "signals.h"
#include "cpu_stats.h"
#include "disk_stats.h"
#include "mem_stats.h"
#include "net_stats.h"

static char hostname[_POSIX_HOST_NAME_MAX + 1];
static bool received_timer_signal = false;
static bool received_intr_signal = false;

int main ()
{
  register_intr_signal_handler (&received_intr_signal);
  register_timer_signal_handler (&received_timer_signal);

  int rc = gethostname (hostname, _POSIX_HOST_NAME_MAX + 1);
  if (rc != 0)
    {
      perror ("gethostname");
      exit (EXIT_FAILURE);
    }

  initialize_cpu_state ();
  initialize_disk_state ();
  initialize_mem_state ();
  initialize_net_state ();

  printf ("%s,%s,%s,%s,%s\n",
	  "hostname",
          format_cpu_stats (NULL, ","),
          format_disk_stats (NULL, ","),
          format_mem_stats (NULL, ","),
          format_net_stats (NULL, ","));

  create_and_start_timer (10);

  refresh_cpu_state ();
  refresh_disk_state ();
  refresh_mem_state ();
  refresh_net_state ();

  while (1)
    {
      pause (); // Wait for a signal

      if (received_timer_signal)
        {
          refresh_cpu_state ();
	  refresh_disk_state ();
          refresh_mem_state ();
          refresh_net_state ();

          cpu_stats cpu_stats = get_cpu_stats ();
          disk_stats disk_stats = get_disk_stats ();
          mem_stats mem_stats = get_mem_stats ();
          net_stats net_stats = get_net_stats ();

          printf ("%s,%s,%s,%s,%s\n",
		  hostname,
                  format_cpu_stats (&cpu_stats, ","),
                  format_disk_stats (&disk_stats, ","),
                  format_mem_stats (&mem_stats, ","),
                  format_net_stats (&net_stats, ","));

          received_timer_signal = false;
        }
      if (received_intr_signal)
        {
          printf ("terminating...\n");
          break;
        }
    }

  cleanup_cpu_state ();
  cleanup_disk_state ();
  cleanup_mem_state ();
  cleanup_net_state ();

  exit (EXIT_SUCCESS);
}
