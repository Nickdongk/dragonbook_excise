#ifndef TREE_H_
#define TREE_H_

enum {
	LEFT = 0,
	MIDDLE,
	RIGHT,
};

enum {
	CONJUNCTION = 10,
	CHOOSE,
	CLOSURE,
	BRAKET,
};

enum {
	CHARACTER,
	TREE_NODE,
};


struct tree_node {
	char *name;
	unsigned int key;
	unsigned int chld_num;
	unsigned int nodes_num;
	struct tree_node *nodes[];
};

extern struct tree_node *get_tree_node_nc(char *name, unsigned int key, unsigned int num);

extern struct tree_node *get_tree_node(char *name, unsigned int key);

extern void release_tree(struct tree_node *root);

extern int traverse_one_tree(struct tree_node * root);

#endif
