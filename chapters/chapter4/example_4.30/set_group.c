#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#include "set_group.h"
#include "list.h"

struct set_group *get_set_group(set_node_callback func)
{

	struct set_group *ret_group = NULL;

	ret_group = malloc(sizeof(struct set_group));
	if (!ret_group) {
		fprintf(stderr, "fail malloc set group\n");
		exit(-1);
	}
	INIT_LIST_HEAD(&ret_group->set_nodes);
	ret_group->flag = 0;
	ret_group->nodes_num = 0;
	ret_group->func = func;

	return ret_group;
}

void release_set_group(struct set_group *group)
{
	struct set_node *set_node_tmp = NULL;
	struct set_node *set_node_tmp1 = NULL;

	list_for_each_entry_safe(set_node_tmp, set_node_tmp1, &group->set_nodes, node) {
		set_node_tmp->body = NULL;
		free(set_node_tmp);
		if (group->nodes_num)
			group->nodes_num --;
	}

	free(group);
}

void traverse_set_group(struct set_group *group, void *priv)
{
	struct set_node *set_node_tmp = NULL;

	list_for_each_entry(set_node_tmp, &group->set_nodes, node) {
		group->func(set_node_tmp, priv);
	}
}

int compare_set_group(struct set_group *src_group, struct set_group *dst_group)
{
	struct set_node *node_src = NULL;
	struct set_node *node_dst = NULL;
	int ret = 0;

	if (src_group->nodes_num != dst_group->nodes_num)
		return 0;

	list_for_each_entry(node_src, &src_group->set_nodes, node) {
		list_for_each_entry(node_dst, &dst_group->set_nodes, node) {
			if (node_src->body == node_dst->body)
				ret ++;
		}
	}

	return (ret == src_group->nodes_num);
}

void add_to_group(struct set_group *group, void *data)
{
	struct set_node *node_tmp = NULL;

	node_tmp = malloc(sizeof(struct set_node));
	if (!node_tmp) {
		fprintf(stderr, "fail to malloc set_node\n");
		exit(-1);
	}

	INIT_LIST_HEAD(&node_tmp->node);
	node_tmp->body = data;

	list_add(&node_tmp->node, &group->set_nodes);
	group->nodes_num ++;
	return;
}

void push_to_group(struct set_group *group, void *data)
{
	struct set_node *node_tmp = NULL;

	node_tmp = malloc(sizeof(struct set_node));
	if (!node_tmp) {
		fprintf(stderr, "fail to malloc set_node\n");
		exit(-1);
	}

	INIT_LIST_HEAD(&node_tmp->node);
	node_tmp->body = data;

	list_add_tail(&node_tmp->node, &group->set_nodes);
	group->nodes_num ++;
	return;

}

struct set_node * pop_from_group(struct set_group *group)
{
	struct list_head *tmp = group->set_nodes.prev;
	struct set_node *tmp_node = NULL;

	if(list_empty(&group->set_nodes)) {
		return NULL;
	}

	list_del(tmp);
	group->nodes_num --;

	return container_of(tmp, struct set_node, node);
}

struct set_node *check_group_top(struct set_group *group)
{
	struct list_head *tmp = group->set_nodes.prev;

	if(list_empty(&group->set_nodes)) {
		return NULL;
	}

	return container_of(tmp, struct set_node, node);

}

int check_group_empty(struct set_group *group)
{
	return !group->nodes_num;
}

int get_nodes_num(struct set_group *group)
{
	return group->nodes_num;
}
