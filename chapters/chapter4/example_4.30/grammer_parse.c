#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "list.h"
#include "grammer_parse.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))

struct tree_node {
	char *name;
	unsigned int key;
	unsigned int chld_num;
	unsigned int node_num;
	struct tree_node *nodes[];
};

struct tree_node * get_tree_node(char *name, unsigned int key)
{
	struct tree_node *tmp = NULL;
	int i = 0;

	tmp = malloc(sizeof(struct tree_node) + 3 *sizeof(struct tree_node *));
	if (!tmp) {
		perror("fail malloc tree node\n");
		return NULL;
	}

	tmp->node_num = 3;
	tmp->chld_num = 0;
	for (i = 0; i < 3; i ++) {
		tmp->nodes[i] = NULL;
	}

	tmp->name = malloc(strlen(name) + 1);
	if (!tmp->name) {
		perror("fail malloc name space\n");
		return NULL;
	}

	tmp->key = key;

	return tmp;
}

struct tree_node *renum_chld_nodes(struct tree_node *node, unsigned int num)
{
	struct tree_node *tmp = NULL;
	int i = 0;

	if (num < 3)
		return node;

	tmp = realloc(node, sizeof(struct tree_node) +  num * sizeof(struct tree_node *));
	if (!tmp) {
		perror("fail to realloc tree_node\n");
		return NULL;
	}

	for (i = 3; i < num; i ++) {
		tmp->nodes[i] = NULL;
	}

	tmp->node_num = num;

	return tmp;
}

static void parse_add_trans_node(char *str, struct set_group *group)
{
	struct trans_node *tmp = NULL;
	char *key_str = NULL;
	int key_str_size = 0;
	char *val_str = NULL;
	char *tmp_str = NULL;
	int r_size = 0;
	int val_str_size = 0;

	val_str = strstr(str, ":");
	if (!val_str) {
		fprintf(stderr, "fail to find |, wrong format\n");
		exit(-1);
	}

	if (!*(val_str + 1)) {
		fprintf(stderr, "wrong format\n");
		exit(-1);
	}

	key_str = str;
	key_str_size = val_str - key_str;

	val_str = val_str + 1;
	r_size = strlen(val_str);

	while (r_size > 0) {
		tmp_str = strstr(val_str, "|");
		if (tmp_str)
			val_str_size = tmp_str - val_str;
		else
			val_str_size = strlen(val_str);

		tmp = malloc(sizeof(struct trans_node));
		if (!tmp) {
			perror("fail malloc grammer node\n");
			exit(-1);
		}

		tmp->flag = 0;
		tmp->key = malloc(key_str_size + 1);
		if (!tmp->key) {
			perror("fail malloc gramer key\n");
			exit(-1);
		}

		tmp->val = malloc(val_str_size + 1);
		if (!tmp->val) {
			perror("fail malloc grammer val\n");
			exit(-1);
		}

		memcpy(tmp->key, key_str, key_str_size);
		tmp->key[key_str_size] = '\0';
		memcpy(tmp->val, val_str, val_str_size);
		tmp->val[val_str_size] = '\0';

		add_to_group(group, (void *)tmp);
		r_size = r_size - val_str_size;
		if (tmp_str) {
			val_str = tmp_str + 1;
			r_size --;
		}
	}

	return;
}

void free_trans_node(struct trans_node *node)
{
	free(node->key);
	free(node->val);
	free(node);
}

void trans_node_callback(struct set_node *node, void *pdata)
{
	free_trans_node((struct trans_node *)node->body);
	return;
}

void release_grammer(struct grammer *gr)
{
	gr->gen_func_group->func = trans_node_callback;
	traverse_set_group(gr->gen_func_group, NULL);
	release_set_group(gr->gen_func_group);
	free(gr);
}

int check_end_sym(struct grammer *gr, char *str)
{
	int i = 0;

	for (i = 0; i < gr->end_symbol_size; i ++) {
		if (!memcmp(gr->end_symbol[i], str, strlen(gr->end_symbol[i])))
			return strlen(gr->end_symbol[i]);
	}

	return 0;
}

int check_no_end_sym(struct grammer *gr, char *str)
{
	int i = 0;

	for (i = 0; i < gr->no_end_size; i ++) {
		if (!memcmp(gr->no_end_symbol[i], str, strlen(gr->no_end_symbol[i])))
			return strlen(gr->no_end_symbol[i]);
	}

	return 0;
}

int spread_trans(struct grammer *grm, char *key)
{
	int ret = 0;

	ret = check_end_sym(grm, key);
	if (ret) {
		fprintf(stderr, "it is a end sym\n");
		exit(-1);
	}

	ret = check_no_end_sym(grm, key);
	if (!ret) {
		fprintf(stderr, "it is not a no end sym\n");
		exit(-1);
	}

	if ()
}

struct trans_node *parse_str(struct grammer *gr, char *str)
{

	struct tree_node *cur_ptr_node = NULL;
	int size = strlen(str);
	char *end_p = str + size;

	gr->global_p = str;

	while (gr->global_p != end_p) {

	}
}

char *grammer_trans[] = {
	"S:cAd",
	"A:ab|a"
};

char *start_sym = "S";

char *end_sym[] = {
	"a",
	"b",
	"c",
	"d"
};

char *no_end_sym[] = {
	"A",
	"S"
};

struct grammer * get_grammer_entity(char **end_sym, unsigned int end_sym_size,
				    char **no_end_sym, unsigned int no_end_size,
				    char **trans, unsigned int trans_size, char *s_sym)
{
	struct grammer *grmr_tmp = NULL;
	int i = 0;

	grmr_tmp = malloc(sizeof(struct grammer));
	if (!grmr_tmp) {
		fprintf(stderr, "fail to malloc grammer\n");
		exit(-1);
	}

	grmr_tmp->end_symbol = end_sym;
	grmr_tmp->end_symbol_size = end_sym_size;
	grmr_tmp->no_end_symbol = no_end_sym;
	grmr_tmp->no_end_size = no_end_size;
	grmr_tmp->gen_func = trans;
	grmr_tmp->gen_func_size = trans_size;
	grmr_tmp->s_symbol = s_sym;

	grmr_tmp->gen_func_group = get_set_group(NULL);
	grmr_tmp->global_p = NULL;
	grmr_tmp->g_p_next = NULL;
	grmr_tmp->gramer_tree_root = NULL;

	for (i = 0; i < trans_size; i ++) {
		parse_add_trans_node(trans[i], grmr_tmp->gen_func_group);
	}

	return grmr_tmp;
}


