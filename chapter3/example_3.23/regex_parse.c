#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <string.h>
#include "list.h"
#include "set_group.h"
#include "tree.h"
#include "graph.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))
#define BUFFER_SIZE	4096

struct stack_node {
	int type;
	union {
		unsigned int c;
		struct tree_node *node;
	} data;
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

struct stack_node *get_stack_node(int type, void *data)
{
	struct stack_node *tmp_stack = NULL;

	tmp_stack = malloc(sizeof(struct stack_node));
	if (!tmp_stack) {
		fprintf(stderr, "fail to malloc stack node\n");
		exit(-1);
	}

	if (type == CHARACTER) {
		tmp_stack->type = CHARACTER;
		tmp_stack->data.c = (unsigned int)data;
	} else {
		tmp_stack->type = TREE_NODE;
		tmp_stack->data.node = (struct tree_node *)data;
	}

	return tmp_stack;
}

void release_stack_node(struct stack_node *node)
{
	free(node);
}

void push_stack_node(int type, void *data, struct set_group *group)
{
	struct stack_node *tmp_node = NULL;

	tmp_node = get_stack_node(type, data);
	push_to_group(group, (void *)tmp_node);

	return;
}

struct stack_node *pop_stack_node(struct set_group *group)
{
	struct set_node *tmp_node = NULL;

	tmp_node = pop_from_group(group);
	if (!tmp_node)
		return NULL;

	return (struct stack_node *)tmp_node->body;
}

int check_stack_top(int type, unsigned int c, struct set_group *group)
{
	struct set_node *tmp_node = NULL;
	struct stack_node *tmp_stack = NULL;

	tmp_node = check_group_top(group);
	if (!tmp_node) {
		fprintf(stderr, "stack is empty\n");
		exit(-1);
	}

	tmp_stack = (struct stack_node *)tmp_node->body;
	if (type == CHARACTER) {
		return tmp_stack->type == CHARACTER && tmp_stack->data.c == c;
	} else
		return tmp_stack->type == TREE_NODE;
}

void set_chld_node(struct tree_node *node, struct tree_node *p_node, int pos)
{
	p_node->nodes[pos] = node;
	p_node->chld_num ++;
}

struct tree_node *parse_others(struct set_group *group)
{
	struct stack_node *tmp_node = NULL;
	struct stack_node *tmp_node_next = NULL;
	struct tree_node *leaf_node = NULL;
	struct tree_node *root_node = NULL;
	int ret = 0;

	tmp_node = pop_stack_node(group);
	if (tmp_node->type == CHARACTER) {
		leaf_node = get_tree_node(NULL, tmp_node->data.c);
		root_node = get_tree_node(NULL, CONJUNCTION);
		set_chld_node(leaf_node, root_node, RIGHT);
	} else {
		root_node = get_tree_node(NULL, CONJUNCTION);
		set_chld_node(tmp_node->data.node,root_node, RIGHT);
	}

	release_stack_node(tmp_node);

	return root_node;
}

struct tree_node *get_choose_node(struct tree_node *left, struct tree_node *right)
{
	struct tree_node *ret_node = NULL;
	struct tree_node *midd_node = NULL;

	ret_node = get_tree_node(NULL, CHOOSE);
	midd_node = get_tree_node(NULL, '|');

	set_chld_node(left, ret_node, LEFT);
	set_chld_node(right, ret_node, RIGHT);
	set_chld_node(midd_node, ret_node, MIDDLE);

	return ret_node;
}

struct tree_node *get_braket_node(struct tree_node *midd)
{
	struct tree_node *right_node = NULL;
	struct tree_node *left_node = NULL;
	struct tree_node *ret_node = NULL;

	right_node = get_tree_node(NULL, ')');
	left_node = get_tree_node(NULL, '(');
	ret_node = get_tree_node(NULL, BRAKET);

	set_chld_node(right_node, ret_node, RIGHT);
	set_chld_node(left_node, ret_node, LEFT);
	set_chld_node(midd, ret_node, MIDDLE);

	return ret_node;
}

struct tree_node *get_closure_node(struct tree_node *left)
{
	struct tree_node *ret_node = NULL, *right_node = NULL;

	ret_node = get_tree_node(NULL, CLOSURE);
	right_node = get_tree_node(NULL, '*');

	set_chld_node(left, ret_node, LEFT);
	set_chld_node(right_node, ret_node, RIGHT);

	return ret_node;
}

struct tree_node *parse_regex_str(char **str)
{
	char *forward = *str;
	struct set_group *stack_c = NULL;
	struct tree_node *midd_node = NULL;
	struct tree_node *ret_node = NULL;
	struct tree_node *right_node = NULL;
	struct tree_node *left_node = NULL;
	struct stack_node *s_node = NULL;
	struct tree_node *root_node = NULL;
	struct tree_node *cur_node = NULL;
	struct tree_node *pre_node = NULL;
	int ret = 0;
	int flag = 1;

	stack_c = get_set_group(NULL);
	while (*forward) {
		if (*forward == '(') {
			forward ++;
			midd_node = parse_regex_str(&forward);
			ret_node = get_braket_node(midd_node);
			push_stack_node(TREE_NODE, (void *)ret_node,
					stack_c);
		} else if (*forward == ')') {
			ret = get_nodes_num(stack_c);
			while (ret > 0) {
				if (flag) {
				 root_node = cur_node =
					 parse_others(stack_c);
				 flag = 0;
				} else {
					cur_node = parse_others(stack_c);
				}

				if (pre_node) {
					set_chld_node(cur_node, pre_node,
					      LEFT);
				}

				pre_node = cur_node;

				ret = get_nodes_num(stack_c);
			}

			*str = forward;
			release_set_group(stack_c);
			return root_node;
		} else if (*forward == '*') {
			s_node = pop_stack_node(stack_c);
			if (s_node->type == CHARACTER) {
				left_node = get_tree_node(NULL, s_node->data.c);
			} else {
				left_node = s_node->data.node;
			}
			ret_node = get_closure_node(left_node);
			push_stack_node(TREE_NODE, (void *)ret_node,
					stack_c);
		} else if (*forward == '|') {
			char *bracket = NULL;
			if (*(forward + 1) == '(') {
				struct tree_node *tmp_node = NULL;
				bracket = forward + 2;
				tmp_node = parse_regex_str(&bracket);
				forward = bracket;
				right_node = get_braket_node(tmp_node);
			} else {

				right_node = get_tree_node(NULL, *(forward + 1));
				forward ++;
			}

			s_node = pop_stack_node(stack_c);
			if (s_node->type == CHARACTER) {
				left_node = get_tree_node(NULL, s_node->data.c);
			} else {
				left_node = s_node->data.node;
			}
			ret_node = get_choose_node(left_node, right_node);
			push_stack_node(TREE_NODE, (void *)ret_node,
					stack_c);
		} else {
			push_stack_node(CHARACTER, (void *)*forward,
					stack_c);
		}

		forward ++;
	}

	s_node = pop_from_group(stack_c);
	release_stack_node(s_node);
	release_set_group(stack_c);
	return ret_node;
}

int main(int argc, char *argv[])
{
	int ret = 0, i = 0;
	char *regex_str = NULL;
	char *str = NULL;
	char *regex_buffer = NULL;
	char *buffer = NULL;
	struct tree_node *root_node;
	struct graph *gph = NULL;
	struct edge *node_tmp = NULL;

	if (argc != 3)
		fprintf(stderr, "regex_parse basic_regex str\n");

	regex_str = argv[1];
	str = argv[2];

	regex_buffer = malloc(strlen(regex_str) + 4);
	if (!regex_buffer) {
		fprintf(stderr, "fail to malloc regex buffer\n");
		exit(-1);
	}

	sprintf(regex_buffer, "(%s)", regex_str);
	buffer = regex_buffer;

	root_node = parse_regex_str(&buffer);
	traverse_one_tree(root_node);
	gph = parse_ast(root_node);
	printf("node num is %u\n", gph->nodes_num);
	//list_for_each_entry(node_tmp, &gph->entry->out_edges, out_node) {
	//	printf("edge is %c\n", node_tmp->key);
	//}
	ret = check_str_in_nfa(gph, str, strlen(str));
	printf("get str? %s\n", ret? "yes":"no");
	free_graph(gph);
	release_tree(root_node);

	return 0;
}
