#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "set_group.h"

struct set_group *get_no_end_sym_group(struct grammer *gr, char *str)
{
	struct set_node *set_node_tmp = NULL;
	struct trans_node *tnode = NULL;
	struct trans_node *new_tnode = NULL;
	struct set_group *ret_group = NULL;
	int ret = 0;

	list_for_each_entry(set_node_tmp, &gr->gen_func_group->set_nodes,
			    node) {
		tnode = (struct trans_node *)set_node_tmp->body;
		if (!strcmp(tnode->key, str)) {
			new_tnode = malloc(sizeof(struct trans_node));
			if (!new_tnode) {
				fprintf(stderr,
					"fail to malloc trans_node");
				exit(-1);
			}

			new_tnode->key = str;
			new_tnode->val = strdup(tnode->val);

			if (!set_group) {
				ret_group = get_set_group(NULL);
			}

			add_to_group(ret_group, (void *)new_tnode);
		}
	}

	return ret_group;
}


struct set_group *first_group(struct grammer *gr, char *str)
{
	int ret = 0;
	char *str_tmp = NULL;
	struct set_group *ret_group = NULL;

	ret = check_end_sym(gr, str);
	if (ret) {
		ret_group = get_set_group(NULL);
		if (!ret_group)
			exit(-1);
		add_to_group(ret_group, strdup(str));
		return ret_group;
	}

	ret = check_no_end_sym(gr, str);
	if (ret) {
		str_tmp = strndup(str, ret);

	}
}
