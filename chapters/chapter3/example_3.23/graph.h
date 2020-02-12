#ifndef GRAPH_H_
#define GRAPH_H_

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

typedef void (*graph_node_callback)(struct graph_node *node, void *priv);

struct edge {
	struct list_head in_node;
	struct list_head out_node;
	uint64_t key;
	struct graph_node *precedeor;
	struct graph_node *successor;
};

struct graph {
	struct list_head node_list;
	struct graph_node *entry;
	struct graph_node *accept;
	struct set_group *accept_group;
	unsigned int end_num;

	unsigned int nodes_num;
	struct graph_node **node_array;
	graph_node_callback func;
};

extern struct graph *parse_ast(struct tree_node *root);

extern void free_graph(struct graph *gph);
#endif
