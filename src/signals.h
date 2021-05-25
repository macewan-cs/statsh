#ifndef SIGNALS_H
#define SIGNALS_H

void register_intr_signal_handler(bool *flag_to_toggle);
void register_timer_signal_handler(bool *flag_to_toggle);
void create_and_start_timer(long milliseconds);

#endif /* SIGNALS_H */
