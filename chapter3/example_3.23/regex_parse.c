#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <string.h>
#include "list.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))
#define BUFFER_SIZE	4096

struct tree_node {
	char *name;
	unsigned int key;
	unsigned int chld_num;
	struct tree_node *nodes[3];
};

struct stack_node {
	int type;
	union {
		unsigned int c;
		struct tree_node *node;
	};
};

struct regex_parse {
	struct tree_node *root;
	struct tree_node *cur_node;
	char *global_p;
	char *forward;

	unsigned int content_size;
	unsigned int buff_size;
	char buff[];
};

struct stack_node *parse_regex_braket(char *str)
{
	struct set_group *stack_c = NULL;
	struct tree_node *tmp_node = NULL;

	stack_c = get_set_group(NULL);

	push_to_group(stack, (void *)'(');
	while (*str) {
		if (*str != ')' && *str != '(') {
			push_to_group(stack, (void *)*str);
		} else if (*str == '(') {
			str ++;
			tmp_node = parse_regex_braket(str);
		} else if (*str == ')') {
			parse_regex_other
		}
		str ++;
	}

}
