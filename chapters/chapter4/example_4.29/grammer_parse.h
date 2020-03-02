#ifndef _GRAMMER_PARSE_H_
#define _GRAMMER_PARSE_H_

#include <multimap.h>

struct grammer {
	char **end_symbol;
	unsigned int end_symbol_size;
	char **no_end_symbol;
	unsigned int no_end_size;
	char **gen_func;
	unsigned int gen_func_size;
	char *s_symbol;

	struct multimap *gen_func_group;
	char *g_p;
	char *g_p_next;
    char *g_p_end;
	struct general_tree tree;
};


extern struct grammer *get_grammer_entity(char **end_sym, unsigned int end_sym_size, char **no_end_sym,
					  unsigned int no_end_size, char **trans, unsigned int trans_size,
					  char *s_sym);

extern int check_end_sym(struct grammer *gr, char *str);

extern int check_no_end_sym(struct grammer *gr, char *str);

extern void release_grammer(struct grammer *gr);

#endif
