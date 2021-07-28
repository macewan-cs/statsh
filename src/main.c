#define _POSIX_C_SOURCE 200809L

// https://stackoverflow.com/questions/5713451/is-it-safe-to-parse-a-proc-file
// https://stackoverflow.com/questions/62126052/setvbuf-not-changing-buffer-size

#include <assert.h>
#include <bits/posix1_lim.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "signals.h"
#include "cpu_stats.h"
#include "disk_stats.h"
#include "mem_stats.h"
#include "net_stats.h"

static char hostname[_POSIX_HOST_NAME_MAX + 1];
static bool received_timer_signal = false;
static bool received_intr_signal = false;

static void usage (config *cfg)
{
  printf ("usage: %s [options]\n\
\n\
Options:\n\
 -d sec   Seconds between updates (default: %f)\n\
 -D dev   Block device name to monitor (default: %s)\r\n\
 -h       Display help\n\
 -t       Include timestamp (ms since epoch) in output\n\
 -q       Quiet\n",
          cfg->program_name,
          cfg->delay_seconds,
          cfg->disk_device);
}

static void process_arguments (int argc, char *argv[], config *cfg)
{
  int opt;

  while ((opt = getopt (argc, argv, "d:D:hqt")) != -1)
    {
      switch (opt)
        {
          case 'd':cfg->delay_seconds = strtof (optarg, NULL);;
          break;

          case 'D':cfg->disk_device = optarg;
          break;

          case 'h':usage (cfg);
          exit (EXIT_SUCCESS);
          break;

          case 'q':cfg->quiet = true;
          break;

          case 't':cfg->timestamp = true;
          break;

          case '?':usage (cfg);
          exit (EXIT_FAILURE);
        }
    }
}

int get_token_count (char const *s)
{
  int count = 0;

  // Operate on a copy of the string because strtok modifies the string
  char *s_copy = strdup (s);
  if (s_copy == NULL)
    {
      perror ("strdup");
      exit (EXIT_FAILURE);
    }

  char *token;
  token = strtok (s_copy, " ");
  while (token != NULL)
    {
      count++;
      token = strtok (NULL, " ");
    }

  free (s_copy);

  return count;
}

typedef struct {
  int argc;
  char **argv;
} arguments;

void free_arguments (arguments *args)
{
  if (args == NULL)
    {
      return;
    }

  for (int i = 0; i < args->argc; i++)
    {
      free (args->argv[i]);
      args->argv[i] = NULL;
    }

  free (args->argv);
  args->argv = NULL;
}

arguments *arguments_from_string (char *s, char *program_name)
{
  // Add 1 for the program name
  int count = get_token_count (s) + 1;

  arguments *args = malloc (sizeof (arguments));
  if (args == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  args->argc = count;
  int n_strings = count + 1; // +1 for NULL sentinel
  args->argv = calloc (n_strings, sizeof (char **));
  if (args->argv == NULL)
    {
      perror ("calloc");
      exit (EXIT_FAILURE);
    }

  int arg_num = 0;
  args->argv[arg_num] = strdup (program_name);
  if (args->argv[arg_num] == NULL)
    {
      perror ("strdup");
      exit (EXIT_FAILURE);
    }
  arg_num++;

  char *token;
  token = strtok (s, " ");
  while (token != NULL)
    {
      assert (arg_num < n_strings);
      args->argv[arg_num] = strdup (token);
      if (args->argv[arg_num++] == NULL)
        {
          perror ("strdup");
          exit (EXIT_FAILURE);
        }

      token = strtok (NULL, " ");
    }

  assert (arg_num < n_strings);
  args->argv[arg_num] = NULL;

  return args;
}

static arguments *load_config_from_environment (config *cfg)
{
  char *env_value = getenv ("TERM");
  if (env_value == NULL)
    {
      return NULL;
    }

  arguments *args = arguments_from_string (env_value, cfg->program_name);

  return args;
}

config *get_valid_configuration (int argc, char *argv[])
{
  static config cfg;
  cfg = create_default_config (argv[0]);

  // Update the configuration with either:
  // (1) arguments in the TERM environment variable or
  // (2) command-line arguments
  if (argc == 1)
    {
      // Attempt to get arguments from the TERM environment variable
      arguments *args = load_config_from_environment (&cfg);
      process_arguments (args->argc, args->argv, &cfg);
      free (args);
    }
  else
    {
      // Attempt to use command-line arguments
      process_arguments (argc, argv, &cfg);
    }

  char const *error;
  if ((error = is_valid_config (cfg)) != NULL)
    {
      fprintf (stderr, "invalid configuration: %s\n", error);
      exit (EXIT_FAILURE);
    }

  return &cfg;
}

int main (int argc, char *argv[])
{
  config *cfg = get_valid_configuration (argc, argv);

  register_intr_signal_handler (&received_intr_signal);
  register_timer_signal_handler (&received_timer_signal);

  int rc = gethostname (hostname, _POSIX_HOST_NAME_MAX + 1);
  if (rc != 0)
    {
      perror ("gethostname");
      exit (EXIT_FAILURE);
    }

  initialize_cpu_state ();
  initialize_disk_state (cfg);
  initialize_mem_state ();
  initialize_net_state (cfg);

  if (!cfg->quiet)
    {
      if (cfg->timestamp)
        {
          printf ("timestamp.ms,");
        }
      printf ("%s,%s,%s,%s,%s\n",
              "hostname",
              format_cpu_stats (NULL, ","),
              format_disk_stats (NULL, ","),
              format_mem_stats (NULL, ","),
              format_net_stats (NULL, ","));
    }

  unsigned long delay_milliseconds = (unsigned long) roundf (cfg->delay_seconds * 1000.0);
  create_and_start_timer (delay_milliseconds);

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

          if (cfg->timestamp)
            {
              struct timeval tv;
              gettimeofday (&tv, NULL);
              printf ("%ld,",
                      tv.tv_sec * 1000 + tv.tv_usec / 1000
              );
            }

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
