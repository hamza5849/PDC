CC     = mpicc
CFLAGS = -Wall -O2
TARGET = mapreduce
SRCS   = mapreduce.c mapper.c reducer.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run2:
	mpirun -np 2 ./$(TARGET)

run4:
	mpirun -np 4 ./$(TARGET)

run8:
	mpirun -np 8 ./$(TARGET)

benchmark:
	@echo "--- 2 processes ---" && mpirun -np 2 ./$(TARGET)
	@echo "--- 4 processes ---" && mpirun -np 4 ./$(TARGET)
	@echo "--- 8 processes ---" && mpirun -np 8 ./$(TARGET)

clean:
	rm -f $(TARGET) output.txt