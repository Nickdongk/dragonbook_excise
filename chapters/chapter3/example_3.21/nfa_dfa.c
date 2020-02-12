#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "list.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))

#define PRO_IN		0x01
#define PRO_END		(0x01 << 1)

struct set_node {
	struct list_head node;
	void *body;
};

struct graph_node {
	struct list_head node;
	uint64_t key;
	unsigned int color;
	unsigned int flag;
	struct list_head in_edges;
	unsigned int in_edges_size;
	struct list_head out_edges;
	unsigned int out_edges_size;
};

typedef void (*set_node_callback)(struct set_node *node, void *pdata);
typedef void (*graph_node_callback)(struct graph_node *node, void *priv);

struct set_group {
	struct list_head set_nodes;
	int flag;
	int nodes_num;
	set_node_callback func;
};

struct edge {
	struct list_head in_node;
	struct list_head out_node;
	uint64_t key;
	struct graph_node *precedeor;
	struct graph_node *successor;
};

struct graph {
	struct list_head node_list;
	struct list_head  end;

	unsigned int nodes_num;
	struct graph_node **node_array;
	graph_node_callback func;
};

struct graph_node *add_graph_node(struct graph *gph, uint64_t key)
{
	struct graph_node *node = NULL;
	struct graph_node *pos = NULL;

	list_for_each_entry(pos, &gph->node_list, node) {
		if (pos->key == key)
			return pos;
	}

	node = malloc(sizeof(struct graph_node));
	if (!node) {
		fprintf(stderr, "fail to malloc node");
		return NULL;
	}

	node->key = key;
	node->color = 0;
	node->flag = 0;
	INIT_LIST_HEAD(&node->in_edges);
	INIT_LIST_HEAD(&node->out_edges);
	INIT_LIST_HEAD(&node->node);
	list_add(&node->node, &gph->node_list);
	gph->nodes_num ++;
	return node;
}

struct edge *add_edge(uint64_t key, struct graph_node *source, struct graph_node *dst)
{
	struct edge *edge_r = NULL;

	edge_r = malloc(sizeof(struct edge));
	if (!edge_r) {
		fprintf(stderr, "fail to malloc edge\n");
		return NULL;
	}

	memset(edge_r, 0, sizeof(struct edge));
	edge_r->key = key;
	list_add(&edge_r->out_node, &source->out_edges);
	source->out_edges_size ++;
	list_add(&edge_r->in_node, &dst->in_edges);
	dst->in_edges_size ++;
	edge_r->precedeor = source;
	edge_r->successor = dst;

	return edge_r;
}

struct graph *get_empty_graph(graph_node_callback func)
{
	struct graph *direct_graph = NULL;

	direct_graph = malloc(sizeof(struct graph));
	if (!direct_graph) {
		fprintf(stderr, "malloc graph fail.\n");
		exit(-1);
	}

	INIT_LIST_HEAD(&direct_graph->node_list);
	INIT_LIST_HEAD(&direct_graph->end);
	direct_graph->nodes_num = 0;
	direct_graph->func = func;

	return direct_graph;
}


void free_graph(struct graph *gph)
{
	struct graph_node *state_one = NULL;
	struct edge *edge_one = NULL;
	struct graph_node *state_tmp = NULL;
	struct edge *edge_tmp = NULL;

	list_for_each_entry_safe(state_one, state_tmp, &gph->node_list, node) {
		list_for_each_entry_safe(edge_one, edge_tmp, &state_one->out_edges, out_node) {
			list_del(&edge_one->out_node);
			edge_one->precedeor = NULL;
			list_del(&edge_one->in_node);
			edge_one->successor = NULL;
			free(edge_one);
		}

		list_for_each_entry_safe(edge_one, edge_tmp, &state_one->in_edges, in_node) {
			list_del(&edge_one->in_node);
			edge_one->successor = NULL;
			list_del(&edge_one->out_node);
			edge_one->precedeor = NULL;
			free(edge_one);
		}
		list_del(&state_one->node);
		if (gph->func)
			gph->func(state_one, NULL);
		free(state_one);
	}
}

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

void get_closure(struct set_group *group, struct graph_node *state_one, char c)
{
	struct edge *edge_one = NULL;
	struct set_node *set_node_tmp = NULL;

	list_for_each_entry(set_node_tmp, &group->set_nodes, node) {
		if (set_node_tmp->body == (void *)state_one)
			return;
	}

	if (c == '#') {
			set_node_tmp = malloc(sizeof(struct set_node));
			if (!set_node_tmp)
				exit(-1);
			INIT_LIST_HEAD(&set_node_tmp->node);
			set_node_tmp->body = state_one;
			list_add(&set_node_tmp->node, &group->set_nodes);
			group->nodes_num ++;
			set_node_tmp = NULL;
			list_for_each_entry(edge_one, &state_one->out_edges, out_node) {
				if (edge_one->key == c) {
					get_closure(group, (void *)edge_one->successor, '#');
				}
			}

	} else {
		list_for_each_entry(edge_one, &state_one->out_edges, out_node) {
			if (edge_one->key == c) {
				set_node_tmp = malloc(sizeof(struct set_node));
				if (!set_node_tmp)
					exit(-1);
				INIT_LIST_HEAD(&set_node_tmp->node);
				set_node_tmp->body = edge_one->successor;
				list_add(&set_node_tmp->node, &group->set_nodes);
				group->nodes_num ++;
			}
		}
	}
}

struct set_group * move_t_c(struct set_group *src_group,
		   char c)
{
	struct set_node *node_tmp = NULL;
	struct set_group *dst_group = NULL;

	dst_group = get_set_group(NULL);

	list_for_each_entry(node_tmp, &src_group->set_nodes, node) {
		get_closure(dst_group, (struct graph_node *)node_tmp->body, c);
	}

	return dst_group;
}

struct set_group *get_closure_t(struct set_group *src_group)
{
	struct set_node *node_tmp = NULL;
	struct set_group *dst_group = NULL;

	dst_group = get_set_group(NULL);
	list_for_each_entry(node_tmp, &src_group->set_nodes, node) {
		get_closure(dst_group, (struct graph_node *)node_tmp->body, '#');
	}

	release_set_group(src_group);
	return dst_group;
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

struct graph_node  *get_no_color_node(struct graph *gph)
{
	struct graph_node *node_tmp = NULL;

	list_for_each_entry(node_tmp, &gph->node_list, node) {
		if (!node_tmp->color)
			return node_tmp;
	}

	return NULL;
}

int end_node_in_group(struct set_group *gp)
{
	struct graph_node *gnode_tmp = NULL;
	struct set_node *snode_tmp = NULL;

	list_for_each_entry(snode_tmp, &gp->set_nodes, node) {
		if(((struct graph_node *)snode_tmp->body)->flag & PRO_END)
			return 1;
	}

	return 0;
}

int set_in_graph(struct graph *gph, struct set_group *group)
{
	struct graph_node *node_tmp = NULL;
	int ret_tmp = 0;
	int ret = 0;

	list_for_each_entry(node_tmp, &gph->node_list, node) {
		ret_tmp = compare_set_group(group, (struct set_group *)node_tmp->key);
		if (ret_tmp)
			return 1;
	}

	return 0;
}

void set_callback(struct set_node *node, void *pdata)
{
	struct graph_node *state_one = NULL;

	state_one = (struct graph_node *)node->body;
	printf(" %ld\n", state_one->key);
}

void graph_callback(struct graph_node *node, void *pdata)
{
	struct set_group *group_tmp = (struct set_group *)node->key;

	release_set_group(group_tmp);
}

struct graph *generate_graph_from_graph(struct graph *src_graph, char *alph)
{
	struct set_group *group_entr = NULL;
	struct set_group *group_tmp = NULL;
	struct graph *ret_graph = NULL;
	struct graph_node *node_tmp = NULL;
	struct graph_node *node_tmp_dst = NULL;
	int i = 0;
	int alph_size = strlen(alph);
	int ret = 0;

	ret_graph = get_empty_graph(graph_callback);

	group_entr = get_set_group(NULL);
	get_closure(group_entr, src_graph->node_array[0], '#');
	node_tmp = add_graph_node(ret_graph, (uint64_t)group_entr);
	node_tmp->flag |= PRO_IN;
	node_tmp->color = 0;
	do {
		node_tmp->color = 1;
		group_tmp = (struct set_group *)node_tmp->key;
		for (i = 0; i < alph_size; i ++) {
			struct set_group *group_mid = NULL;
			struct set_group *group_lt = NULL;
			group_mid = move_t_c(group_tmp, alph[i]);
			group_lt = get_closure_t(group_mid);
			ret = set_in_graph(ret_graph, group_lt);
			if (!ret) {
				node_tmp_dst = add_graph_node(ret_graph, (uint64_t)group_lt);
				ret = end_node_in_group(group_lt);
				if (ret)
					node_tmp_dst->flag |= PRO_END;

				add_edge(alph[i], node_tmp, node_tmp_dst);
				node_tmp_dst->color = 0;
			}
		}
		node_tmp = get_no_color_node(ret_graph);
	} while (node_tmp);

	return ret_graph;
}

static char *graph_des[] = {
"^0-#->1",
"^0-#->7",
"1-#->2",
"1-#->4",
"2-a->3",
"3-#->6",
"4-b->5",
"5-#->6",
"6-#->1",
"6-#->7",
"7-a->8",
"8-b->9",
"9-b->10$"
};

void generate_graph_from_des(struct graph **gph, char **graph_str, int size)
{
	int i = 0;
	unsigned int src_key = 0, dst_key = 0;
	char edge_key = 0;
	struct graph_node *src = NULL, *dst = NULL;
	struct graph *direct_graph = NULL;
	int s_flag = 0, r_flag = 0;

	direct_graph = get_empty_graph(NULL);

	for (i = 0; i < size; i ++) {
		if (!(graph_str[i][0] == '^')) {
			sscanf(graph_str[i], "%u-%c->%u", &src_key, &edge_key, &dst_key);
		} else {
			sscanf(&graph_str[i][1], "%u-%c->%u", &src_key, &edge_key, &dst_key);
			s_flag = 1;
		}

		if (strstr(graph_str[i], "$"))
			r_flag = 1;

		src = add_graph_node(direct_graph, src_key);
		dst = add_graph_node(direct_graph, dst_key);
		if (!(src && dst))
			exit(-1);
		if (s_flag) {
			src->flag |= PRO_IN;
			s_flag = 0;
		}

		if (r_flag) {
			dst->flag |= (PRO_END);
			r_flag = 0;
		}

		 add_edge(edge_key, src, dst);
	}

	 direct_graph->node_array = malloc(direct_graph->nodes_num * sizeof(struct graph_node *));
	if (!direct_graph->node_array) {
		perror("malloc fail");
		exit(-1);
	}

	list_for_each_entry(src, &direct_graph->node_list, node) {
		direct_graph->node_array[src->key] = src;
	}

	*gph = direct_graph;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	struct graph_node *state_one = NULL;
	struct graph *global_graph = NULL;
	struct graph *dst_graph = NULL;
	struct set_group *group_tmp = NULL;
	struct set_node *node_tmp = NULL;
	char *edge_list = "ab";

	generate_graph_from_des(&global_graph, graph_des, ARRAY_SIZE(graph_des));

	dst_graph = generate_graph_from_graph(global_graph, edge_list);

	list_for_each_entry(state_one, &dst_graph->node_list, node) {
		group_tmp = (struct set_group *)state_one->key;
		if (state_one->flag & PRO_IN)
			printf("^ state <");
		else if (state_one->flag & PRO_END)
			printf("$ state <");
		else
			printf(" state < ");
		list_for_each_entry(node_tmp, &group_tmp->set_nodes, node) {
			printf("%ld ", (uint64_t) ((struct graph_node *)node_tmp->body)->key);
		}
		printf(">\n");
	}

	free_graph(dst_graph);
	free_graph(global_graph);
	return 0;
}
