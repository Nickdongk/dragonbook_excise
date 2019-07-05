#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "list.h"
#include "set_group.h"
#include "tree.h"
#include "graph.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))

#define PRO_IN		0x01
#define PRO_END		(0x01 << 1)
#define SIGMA		65536

static int global_count = 0;

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
	direct_graph->entry = NULL;
	direct_graph->accept = NULL;
	direct_graph->accept_group = get_set_group(NULL);
	direct_graph->end_num = 0;

	direct_graph->nodes_num = 0;
	direct_graph->node_array = NULL;
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

void get_closure(struct set_group *group, struct graph_node *state_one, unsigned int c)
{
	struct edge *edge_one = NULL;
	struct set_node *set_node_tmp = NULL;

	list_for_each_entry(set_node_tmp, &group->set_nodes, node) {
		if (set_node_tmp->body == (void *)state_one)
			return;
	}

	if (c == SIGMA) {
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
					get_closure(group, (void *)edge_one->successor, SIGMA);
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
		get_closure(dst_group, (struct graph_node *)node_tmp->body, SIGMA);
	}

	release_set_group(src_group);
	return dst_group;
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
	get_closure(group_entr, src_graph->node_array[0], SIGMA);
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

struct graph *choose_graph(struct graph *src, struct graph *dst)
{
	struct graph_node *start, *end;

	dst->entry->flag &= ~PRO_IN;
	dst->accept->flag &= ~PRO_END;
	src->entry->flag &= ~PRO_IN;
	src->accept->flag &= ~PRO_END;
	start = add_graph_node(dst, global_count ++);
	add_edge(SIGMA, start, dst->entry);
	add_edge(SIGMA, start, src->entry);
	start->flag |= PRO_IN;
	end = add_graph_node(dst, global_count ++);
	add_edge(SIGMA, dst->accept, end);
	add_edge(SIGMA, src->accept, end);
	end->flag |= PRO_END;

	list_splice(&src->node_list, &dst->node_list);
	dst->nodes_num += src->nodes_num;
	dst->entry = start;
	dst->accept = end;
	if (src->accept_group)
		release_set_group(src->accept_group);
	free(src);

	return dst;
}

struct graph *conjunction_graph(struct graph *src, struct graph *dst)
{

	list_splice(&src->node_list, &dst->node_list);
	dst->accept->flag &= ~PRO_END;
	add_edge(SIGMA, dst->accept, src->entry);
	src->entry->flag &= ~PRO_IN;
	dst->nodes_num += src->nodes_num;
	dst->accept = src->accept;

	if (src->accept_group)
		release_set_group(src->accept_group);
	free(src);
	return dst;
}

struct graph *closure_graph(struct graph *gph)
{
	struct graph_node *start, *end;
	gph->accept->flag &= ~PRO_END;
	gph->entry->flag &= ~PRO_IN;
	add_edge(SIGMA,gph->accept, gph->entry);
	start = add_graph_node(gph, global_count ++);
	end = add_graph_node(gph, global_count ++);

	add_edge(SIGMA, start, gph->entry);
	add_edge(SIGMA, gph->accept, end);
	add_edge(SIGMA, start, end);
	gph->entry = start;
	gph->accept = end;

	return gph;
}

struct graph *single_graph(unsigned int key)
{
	struct graph_node *start = NULL, *end = NULL;
	struct graph *ret_graph = NULL;

	ret_graph = get_empty_graph(NULL);

	start = add_graph_node(ret_graph, global_count ++);
	start->flag |= PRO_IN;
	ret_graph->entry = start;

	end = add_graph_node(ret_graph, global_count ++);
	end->flag |= PRO_END;
	ret_graph->accept = end;
	add_edge(key, start, end);

	return ret_graph;
}

struct graph *parse_ast(struct tree_node *root)
{
	struct graph *graph_tmp = NULL, *graph_tmp2;
	struct graph *graph_ret = NULL;

	if (!root)
		return NULL;

	if (root->chld_num > 1) {
		switch (root->key) {
		case BRAKET:
			graph_ret = parse_ast(root->nodes[MIDDLE]);

			break;
		case CLOSURE:
			graph_tmp = parse_ast(root->nodes[LEFT]);
			graph_ret = closure_graph(graph_tmp);
			break;
		case CHOOSE:
			graph_tmp = parse_ast(root->nodes[LEFT]);
			graph_tmp2 = parse_ast(root->nodes[RIGHT]);
			graph_ret = choose_graph(graph_tmp2, graph_tmp);

			break;
		case CONJUNCTION:
			graph_tmp = parse_ast(root->nodes[LEFT]);
			graph_tmp2 = parse_ast(root->nodes[RIGHT]);
			graph_ret = conjunction_graph(graph_tmp2, graph_tmp);
			break;
		}
	} else {
		int i = 0;
		for (i = 0; i < RIGHT + 1; i ++) {
			if (root->nodes[i]) {
				graph_ret = parse_ast(root->nodes[i]);
				break;
			}
		}

		if (i == RIGHT + 1) {
			graph_ret = single_graph(root->key);
		}

	}

	return graph_ret;
}

int check_str_in_nfa(struct graph *src_graph, char * str, int str_len)
{
	struct set_group *group_entr = NULL;
	struct set_group *group_cur = NULL;
	struct set_group *group_tmp = NULL;
	struct set_group *group_next = NULL;
	int i = 0;
	int ret = 0;


	group_cur = get_set_group(NULL);
	get_closure(group_cur, src_graph->entry, SIGMA);

	while (i < str_len) {
		group_tmp = move_t_c(group_cur, str[i]);
		if (!group_tmp)
			break;
		group_next = get_closure_t(group_tmp);
		release_set_group(group_cur);
		group_cur = group_next;
		i ++;
	}

	ret = end_node_in_group(group_cur);
	if (ret)
		return 1;
}

