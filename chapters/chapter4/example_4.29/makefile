TARGET:= down_recall

.phony: all clean

all: $(TARGET)

%:%.c
	@gcc -o $@ $^

clean:
	@rm -f $(TARGET)

