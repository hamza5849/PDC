CC     = mpicc
CFLAGS = -Wall -O2
TARGET = mapreduce
SRCS   = mapreduce.c

all: $(TARGET)

 $(TARGET): $(SRCS)
    $(CC) $(CFLAGS) -o $@ $^

run2:
    mpirun --allow-run-as-root -np 2 ./$(TARGET)

run4:
    mpirun --allow-run-as-root -np 4 ./$(TARGET)

run8:
    mpirun --allow-run-as-root -np 8 ./$(TARGET)

clean:
    rm -f $(TARGET) output.txt