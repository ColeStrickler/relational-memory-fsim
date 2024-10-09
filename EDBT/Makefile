CC=riscv64-unknown-linux-gnu-gcc
CC2=gcc
OUTPUT=new_demo
INCLUDE_DIR=./include
UTILS_DIR=./utils
QUERY_DIR=./queries
SOURCES=demo.c\
$(UTILS_DIR)/db_generate.c \
$(UTILS_DIR)/db_relcache.c \
$(UTILS_DIR)/exp_config.c \
$(UTILS_DIR)/parse_config.c \
$(UTILS_DIR)/performance_counters.c \
$(UTILS_DIR)/print_utils.c \
$(QUERY_DIR)/q4.c\
$(QUERY_DIR)/q1.c\
$(QUERY_DIR)/q2.c\
$(QUERY_DIR)/q3.c\
$(QUERY_DIR)/experiments.c\


CFLAGS = -I$(INCLUDE_DIR) -lm -O0

all: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)

	
all2: clean $(SOURCES)
	$(CC2) $(CFLAGS) $(SOURCES) -o $(OUTPUT)


clean:
	rm -f $(OUTPUT) *.o

test:
	$(CC) $(CFLAGS) $(UTILS_DIR)/test_counters.c -o test_counters
	chmod +x ./test_counters
