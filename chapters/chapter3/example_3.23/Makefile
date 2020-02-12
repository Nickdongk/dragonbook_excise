TARGET:= regex_parse
SOURCES:= regex_parse.c set_group.c tree.c graph.c

.phony: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	@gcc -o $@ $^

clean:
	@rm -f $(TARGET)


