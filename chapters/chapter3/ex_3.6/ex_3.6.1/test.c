#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct state_des {
	int comp;
	int next_state_t;
	int next_state_f;
};

int sig_flag = 0;

void sighandle(int sig)
{
	sig_flag = 1;
}

void print_usage(void)
{
	printf("findstr -f filename -i str\n");
	exit(-1);
}

void set_fail_function(int *fail_function, char *content)
{
	int t = 0, i = 0;
	int size = strlen(content);

	memset(fail_function, 0, size);

	for (i = 1; i < size; i ++) {
		while (t > 0 && content[i + 1 - 1] != content[t + 1 - 1])
			t = fail_function[t - 1];
		if (content[i + 1 - 1] == content[t + 1 - 1]) {
			t = t + 1;
			fail_function[i + 1 - 1] = t;
		} else
			fail_function[i + 1 - 1] = 0;
	}
}
/*
char *find_str(char *content, unsigned int size, char *key, int *fail_function)
{
	int i = 0, s = 0;
	int key_chars = strlen(key);

	for (i = 1; i <= size; i ++) {
		while ( s > 0 && content[i - 1] != key[s])
			s = fail_function[s - 1];
		if (content[i - 1] == key[s])
			s ++;
		if (s == key_chars)
			return &content[i - s];
	}

	return NULL;
}
*/
char *find_str(char *content, unsigned int size, struct state_des * dfa)
{
	struct state_des * cur_state = NULL;
	int i = 0;
	cur_state = dfa;
	while(i < size && cur_state->comp != 0 && cur_state->next_state_t != 0) {
		if (content[i] == cur_state->comp)
			cur_state = &dfa[cur_state->next_state_t];
		else
			cur_state = &dfa[cur_state->next_state_f];

		i ++;
	}
	if (i != size) {
		content[i] = '\0';
		return content;
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i = 0, ret = 0;
	char *filename = NULL;
	char *str = NULL;
	struct stat sb;
	char *file_buffer = NULL;
	char *result = NULL;
	int *fail_function = NULL;
	struct state_des *dfa_p = NULL;
	int str_len = 0;
	int fd = 0;

	if (argc != 5)
		print_usage();

	for (i = 0; i < 5; i ++) {
		if (*argv[i]++ == '-') {
			if (*argv[i] == 'f') {
				filename = argv[i + 1];
			} else if (*argv[i] == 'i'){
				str = argv[i + 1];
			}
		}
	}

	if (!(filename && str))
		print_usage();

	ret = stat(filename, &sb);
	if (ret) {
		fprintf(stderr, "file stat file: %s\n", filename);
		return ret;
	}

	file_buffer = malloc(sb.st_size);
	if (!file_buffer) {
		fprintf(stderr, "fail malloc file buffer\n");
		return -1;
	}

	str_len = strlen(str);
	fail_function = malloc(str_len * sizeof(int));
	if (!fail_function) {
		fprintf(stderr, "fail malloc fail function");
		goto end;
	}

	fd = open(filename, S_IRUSR);
	if (fd < 0)
		goto end;

	dfa_p = malloc(sizeof(struct state_des) * (str_len + 1));
	if (!dfa_p) {
		fprintf(stderr, "fail to malloc dfa array\n");
		goto end;
	}

	ret = read(fd, file_buffer, sb.st_size);
	if (ret < sb.st_size)
		goto end;

	file_buffer[sb.st_size - 1] = '\0';

	set_fail_function(fail_function, str);

	for ( i = 0; i < str_len; i ++) {
		printf("n = %d, f(%d)= %d\n", i + 1, i + 1, fail_function[i]);
	}

	for (i = 0; i < str_len; i ++) {
		dfa_p[i].comp = str[i];
		dfa_p[i].next_state_t = i + 1;
		if (i == 0)
			dfa_p[i].next_state_f = 0;
		else
			dfa_p[i].next_state_f = fail_function[i - 1];

	}
	dfa_p[str_len].comp = 0;
	dfa_p[str_len].next_state_t = dfa_p[str_len].next_state_f = 0;

	result = find_str(file_buffer, strlen(file_buffer), dfa_p);
	if (result) {
		printf(" find str %s\n", result);
	}

end:

	if (fd)
		close(fd);

	if (fail_function)
		free(fail_function);

	if (file_buffer)
		free(file_buffer);

	if (dfa_p)
		free(dfa_p);

	return 0;
}

