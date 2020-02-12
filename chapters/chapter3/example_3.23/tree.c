#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"
#include "set_group.h"
#include "tree.h"

struct tree_node *get_tree_node_nc(char *name, unsigned int key, unsigned int num)
{
	struct tree_node *tmp_node = NULL;
	int i = 0;

	tmp_node = malloc(sizeof(struct tree_node) + num * sizeof(struct tree_node *));
	if (!tmp_node) {
		fprintf(stderr, "fail to malloc tree node\n");
		exit(-1);
	}
	tmp_node->name = name;
	tmp_node->key = key;
	tmp_node->chld_num = 0;
	tmp_node->nodes_num = num;
	for (i = 0; i < num; i ++) {
		tmp_node->nodes[i] = NULL;
	}

	return tmp_node;
}

struct tree_node *get_tree_node(char *name, unsigned int key)
{
	return get_tree_node_nc(name, key, 3);
}


void release_tree(struct tree_node *root)
{
	int i = 0;
	for (i = 0; i < root->nodes_num; i ++) {
		if (root->nodes[i])
			release_tree(root->nodes[i]);
	}

	free(root);
}


static int traverse_tree(struct tree_node * root, struct set_group *group)
{
	struct tree_node *root_node = root;
	struct tree_node *cur_node = NULL;
	struct tree_node *next_node = NULL;
	int level = 0, ret = 0;
	int flag = 0;
	struct set_node *tmp = NULL;
	int i = 0, j = 0;

	cur_node = root_node;
	if (cur_node) {

		list_for_each_entry(tmp, &group->set_nodes, node) {
			if (tmp->body) {
				printf("    |");
				flag = 1;
			}
			else {
				printf("     ");
				flag = 0;
			}
		}

		if (flag)
			printf("\n");
		else
			printf("\b|\n");

		list_for_each_entry(tmp, &group->set_nodes, node) {
			if (tmp->body) {
				printf("    |");
				flag = 1;
			}
			else {
				printf("     ");
				flag = 0;
			}
		}

		if (!cur_node->chld_num) {
			if (flag)
				printf("----%c\n", cur_node->key);
			else
				printf("\b|----%c\n", cur_node->key);
		} else {
			if (flag)
				printf("----%d\n", cur_node->key);
			else
				printf("\b|----%d\n", cur_node->key);
		}

		for (i = 0; i < cur_node->nodes_num; i ++) {
			unsigned int slash_flag = 0;
			for (j = i + 1; j < cur_node->nodes_num; j ++) {
				slash_flag = slash_flag || cur_node->nodes[j];
			}

			if (cur_node->nodes[i]) {
				push_to_group(group, (void *)slash_flag);
				traverse_tree(cur_node->nodes[i], group);
				pop_from_group(group);
			}
		}
	}

	return 0;
}

int traverse_one_tree(struct tree_node *root_node)
{
	struct set_group *group = NULL;

	group = get_set_group(NULL);
	traverse_tree(root_node, group);
	release_set_group(group);

	return 0;
}


