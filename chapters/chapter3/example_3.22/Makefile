TARGET:= simulate_nfa

.phony: all clean

all: $(TARGET)

%:%.c
	@gcc -o $@ $^

clean:
	@rm -f $(TARGET)


