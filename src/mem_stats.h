#ifndef MEM_STATS_H
#define MEM_STATS_H

typedef struct {
  long total;
  long used;
  long free;
  long shared;
  long buffers;
  long cache;
  long s_reclaimable;
  long available;
} mem_stats;

void initialize_mem_state();
void cleanup_mem_state();
void refresh_mem_state();

mem_stats get_mem_stats();

// format_mem_stats returns a pointer to a static NUL-terminated
// string for the formatted mem stats. If the mem_stats pointer is
// NULL, it formats a header line in the string.
char *format_mem_stats(mem_stats *stats, const char *sep);

#endif /* MEM_STATS_H */
