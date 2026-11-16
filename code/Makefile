CC = gcc
CFLAGS = -Wall -Wextra -std=c11

PROGS = 1_xor_basic 2_break_single_byte 3_break_repeating_key 4_one_time_pad

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) *.exe
