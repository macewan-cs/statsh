#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <signal.h>
#include <time.h>

#include "signals.h"

static bool *received_timer_signal;
static bool *received_intr_signal;

static void handle_timer_signal(int sig, siginfo_t *si, void *uc)
{
  *received_timer_signal = true;
}

void register_timer_signal_handler(bool *flag_to_toggle)
{
  received_timer_signal = flag_to_toggle;

  struct sigaction signal_action = { 0 };
  signal_action.sa_flags = SA_SIGINFO;
  signal_action.sa_sigaction = handle_timer_signal;
  sigemptyset(&signal_action.sa_mask);

  if (sigaction(SIGRTMIN, &signal_action, NULL) == -1)
    {
      perror( "sigaction");
      exit(EXIT_FAILURE);
    }
}

void create_and_start_timer(long milliseconds)
{
  timer_t timer_id;
  struct sigevent how_to_notify = {
    .sigev_notify = SIGEV_SIGNAL,
    .sigev_signo = SIGRTMIN,
    .sigev_value.sival_ptr = &timer_id,
  };

  if (timer_create(CLOCK_MONOTONIC, &how_to_notify, &timer_id) == -1)
    {
      perror("timer_create");
      exit(EXIT_FAILURE);
    }

  struct itimerspec itimer_spec = { 0 };
  itimer_spec.it_value.tv_sec = 0;
  itimer_spec.it_value.tv_nsec = milliseconds * 1000000L;
  itimer_spec.it_interval.tv_sec = itimer_spec.it_value.tv_sec;
  itimer_spec.it_interval.tv_nsec = itimer_spec.it_value.tv_nsec;

  if (timer_settime(timer_id, 0, &itimer_spec, NULL) == -1)
    {
      perror("timer_settime");
      exit(EXIT_FAILURE);
    }
}

static void handle_intr_signal(int sig, siginfo_t *si, void *uc)
{
  signal(SIGRTMIN, SIG_IGN);
  *received_intr_signal = true;
}

void register_intr_signal_handler(bool *flag_to_toggle)
{
  received_intr_signal = flag_to_toggle;

  struct sigaction signal_action = { 0 };
  signal_action.sa_flags = SA_SIGINFO;
  signal_action.sa_sigaction = handle_intr_signal;
  sigemptyset(&signal_action.sa_mask);

  if (sigaction(SIGINT, &signal_action, NULL) == -1)
    {
      perror("sigaction");
      exit(EXIT_FAILURE);
    }
}
