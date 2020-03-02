#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <list.h>
#include <general_tree.h>
#include "grammer_parse.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))


char *grammer_trans[] = {
	"S:cAd",
	"A:ab|a"
};

char *start_sym = "S";

char *g_end_sym[] = {
	"a",
	"b",
	"c",
	"d"
};

char *g_no_end_sym[] = {
	"A",
	"S"
};


static void parse_add_trans_node(char *str, struct multimap *map)
{
    char *key_buf = NULL;
    char *val_buf = NULL;
	char *key_str = NULL;
	int key_str_size = 0;
	char *val_str = NULL;
	char *tmp_str = NULL;
	int r_size = 0;
	int val_str_size = 0;

	val_str = strstr(str, ":");
	if (!val_str) {
		fprintf(stderr, "fail to find :, wrong format\n");
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


		key_buf = malloc(key_str_size + 1);
		if (!key_buf) {
			perror("fail malloc gramer key\n");
			exit(-1);
		}

		val_buf = malloc(val_str_size + 1);
		if (!val_buf) {
			perror("fail malloc grammer val\n");
			exit(-1);
		}

		memcpy(key_buf, key_str, key_str_size);
		key_buf[key_str_size] = '\0';
		memcpy(val_buf, val_str, val_str_size);
		val_buf[val_str_size] = '\0';

		multimap_add(map, key_buf, val_buf, val_str_size + 1);
        free(key_buf);
		r_size = r_size - val_str_size;
		if (tmp_str) {
			val_str = tmp_str + 1;
			r_size --;
		}
	}

	return;
}

void do_multimap_node(struct multimap_list_n *list_n)
{
    free(list_n->val);
}

void release_grammer(struct grammer *gr)
{
    multimap_release(gr->gen_func_group, do_multimap_node);
    release_tree(gr->tree.root);
	free(gr);
}

int check_end_sym(struct grammer *gr, char *str)
{
	int i = 0;

	for (i = 0; i < gr->end_symbol_size; i ++) {
		if (!strcmp(gr->end_symbol[i], str))
			return 1;
	}

	return 0;
}

int check_no_end_sym(struct grammer *gr, char *str)
{
	int i = 0;

	for (i = 0; i < gr->no_end_size; i ++) {
		if (!strcmp(gr->no_end_symbol[i], str))
			return 1;
	}

	return 0;
}

struct multimap_n *get_noend_gen(struct grammer *grm, char *key)
{
	int ret = 0;
    struct multimap_n *pmultimap_n = NULL;

	ret = check_end_sym(grm, key);
	if (ret)
		return NULL;
	ret = check_no_end_sym(grm, key);
	if (!ret)
		return NULL;

    pmultimap_n = multimap_get(grm->gen_func_group, key);
    if (!pmultimap_n) {
        fprintf(stderr, "can't get key set\n");
        return NULL;
    }

    return pmultimap_n;

}

void spread_noend(struct tree_node *cur, char *val)
{
    struct tree_node *pnode = NULL;
    size_t val_len = strlen(val);
    int i = 0;

    for (i = 0; i < val_len; i ++) {
        pnode = tree_node_alloc_nc(NULL, (unsigned int)*(val + i), DEFAULT_TREE_NODE_SIZE);
        if (!pnode) {
            fprintf(stderr, " fail to alloc tree node\n");
            return;
        }

        tree_add_node(cur->tree, cur, pnode);
    }
}

int is_start_sym(char *key)
{
    return *key == *start_sym;
}

int str_to_node(struct grammer *gr, struct tree_node *cur_ptr_node)
{
    struct multimap_n *val_set = NULL;
    struct multimap_list_n *tmp_list_n = NULL;
    char *p_enter = NULL;
    int flag = 0;
    struct tree_node *tmp_node = NULL;
    struct tree_node *ret_node = NULL;
    int i = 0;
    int ret = 0;
    int back_up = 0;

    flag = is_start_sym((char *)&cur_ptr_node->key);
    p_enter = gr->g_p_next;
    /* spread start symbol */
    val_set = get_noend_gen(gr, (char *)&cur_ptr_node->key);
    if (!val_set) {
        fprintf(stderr, "fail to get noend gen\n");
        return -1;
    }

            list_for_each_entry(tmp_list_n, &val_set->entry, head) {
                    //choose this gen function
                    spread_noend(cur_ptr_node,
                            tmp_list_n->val);
                    i = 0;
                    tmp_node = cur_ptr_node;
                    /*from the first symbol*/
                    cur_ptr_node = cur_ptr_node->nodes[i];

                    while (cur_ptr_node && gr->g_p_next != gr->g_p_end) {
                        ret = check_end_sym(gr, (char *)&cur_ptr_node->key);
                        if (ret) {
                            /*if it is a terminal symbol*/
                            if (*(gr->g_p_next) != cur_ptr_node->key) {
                                //back up
                                break;
                            }
                            gr->g_p_next ++;
                        } else {
                            /*if it is a non-terminal symbol, spread it here*/
                            /*if it is left recursion, it will infinite*/
                            ret = str_to_node(gr, cur_ptr_node);
                            if (ret < 0)
                                break;
                        }

                        tmp_node = cur_ptr_node;
                        cur_ptr_node = cur_ptr_node->parent->nodes[++ i];
                    }

                    if (ret < 0)
                        continue;

                    if (cur_ptr_node) {
                        cur_ptr_node = cur_ptr_node->parent;
                        back_up = tree_chld_tree_size(cur_ptr_node) - 2;
                        tree_remove_chld_tree(cur_ptr_node);
                        gr->g_p_next -= back_up;
                    } else {
                        if (gr->g_p_next != gr->g_p_end && flag) {
                            cur_ptr_node = tmp_node->parent;
                            back_up = tree_chld_tree_size(cur_ptr_node) - 2;
                            tree_remove_chld_tree(cur_ptr_node);
                            gr->g_p_next -= back_up;
                        } else if (gr->g_p_next != gr->g_p_end && !flag ) {
                           return 1;
                        } else if (gr->g_p_next == gr->g_p_end) {
                            ret_node = tree_last_node(&gr->tree);
                            if (ret_node == tmp_node)
                                return 1;
                        }
                    }
            }

            return -1;

}

int parse_str(struct grammer *gr, char *str)
{

	struct tree_node *cur_ptr_node = NULL;
    int ret = 0;
    int i = 0;

	gr->g_p = str;
    gr->g_p_next = str;
    gr->g_p_end = str + strlen(str);

    cur_ptr_node =  tree_node_alloc_nc(NULL, *start_sym, DEFAULT_TREE_NODE_SIZE);
    if (!cur_ptr_node) {
        fprintf(stderr, "fail to alloc tree node\n");
        return -1;
    }

    tree_add_node(&gr->tree, NULL, cur_ptr_node);
    ret = str_to_node(gr, cur_ptr_node);

    return ret;
}


struct grammer* get_grammer_entity(char **end_sym, unsigned int end_sym_size,
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

	grmr_tmp->gen_func_group = multimap_create();
	grmr_tmp->g_p = NULL;
	grmr_tmp->g_p_next = NULL;
    grmr_tmp->g_p_end = NULL;
	grmr_tmp->tree.root = NULL;
    grmr_tmp->tree.count = 0;

	for (i = 0; i < trans_size; i ++) {
		parse_add_trans_node(trans[i], grmr_tmp->gen_func_group);
	}

	return grmr_tmp;
}

int main(int argc, char *argv[])
{
    char *str = NULL;
    struct grammer *global_gr = NULL;
    int ret = 0;

    if (argc != 2) {
        fprintf(stderr, "%s str\n", argv[0]);
        return -1;
    }

    str = argv[1];

    printf("start parse str\n");


    global_gr = get_grammer_entity(g_end_sym, ARRAY_SIZE(g_end_sym),
				    g_no_end_sym, ARRAY_SIZE(g_no_end_sym),
				    grammer_trans, ARRAY_SIZE(grammer_trans), start_sym);

    ret = parse_str(global_gr, str);

    if (ret > 0)
        tree_traverse_key(&global_gr->tree);
    else
        printf("can not parse with this grammer parser\n");

    release_grammer(global_gr);
}
