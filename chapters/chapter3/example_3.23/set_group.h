#ifndef _SET_GROUP_H_
#define _SET_GROUP_H_

#include "list.h"

struct set_node {
	struct hlist_head node;
	void *body;
};

typedef void (*set_node_callback)(struct set_node *node, void *pdata);

struct set_group {
	struct list_head set_nodes;
	int flag;
	int nodes_num;
	set_node_callback func;
	DECLARE_HASHTABLE(set_nodes, 5);
};

/**
 * \brief   get a set group
 *
 * \param   func	callback func in set group
 *
 * \return  pointer of set group success, NULL fail
 */
extern struct set_group *get_set_group(set_node_callback func);

/**
 *  \brief   release a set goup
 *
 *  \param  group	the goup will be release
 *
 *  \return void
 */
extern void release_set_group(struct set_group *group);
/**
 *  \brief   compare two set group
 *
 *  \param   src_group	source set group to compare
 *  \param   dst_group	dst set goup to compare
 *
 *  \return  1 src is equal dst, 0 not equal
 */
extern int compare_set_group(struct set_group *src_group, struct set_group *dst_group);

extern void add_to_group(struct set_group *group, void *data);

extern void traverse_set_group(struct set_group *group, void *priv);

extern struct set_node *pop_from_group(struct set_group *group);

extern void push_to_group(struct set_group *group, void *data);

extern struct set_node *pop_from_group(struct set_group *group);

extern struct set_node *check_group_top(struct set_group *group);

extern int check_group_empty(struct set_group *group);

extern int get_nodes_num(struct set_group *group);
#endif //_SET_GROUP_H_
