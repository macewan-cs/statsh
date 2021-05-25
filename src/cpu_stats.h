#ifndef CPU_STATS_H
#define CPU_STATS_H

typedef struct {
  float user;
  float nice;
  float system;
  float idle;
} cpu_stats;

void initialize_cpu_state();
void cleanup_cpu_state();
void refresh_cpu_state();

cpu_stats get_cpu_stats();

// format_cpu_stats returns a pointer to a static NUL-terminated
// string for the formatted CPU stats. If the cpu_stats pointer is
// NULL, it formats a header line in the string.
char *format_cpu_stats(cpu_stats *stats, const char *sep);

#endif /* CPU_STATS_H */
