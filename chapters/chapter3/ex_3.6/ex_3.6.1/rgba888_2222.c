#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void print_usage(void)
{
	printf("findstr -o filename_out -i filename_in\n");
	exit(-1);
}

int main(int argc, char *argv[])
{
	int i = 0, ret = 0;
	char *filename_in = NULL;
	char *filename_out = NULL;
	struct stat sb;
	unsigned char *file_buffer = NULL;
	FILE *fp_in = NULL;
	FILE *fp_out = NULL;
	unsigned char pix = 0;

	if (argc != 5)
		print_usage();

	for (i = 0; i < 5; i ++) {
		if (*argv[i]++ == '-') {
			if (*argv[i] == 'o') {
				filename_out = argv[i + 1];
			} else if (*argv[i] == 'i'){
				filename_in = argv[i + 1];
			}
		}
	}

	if (!(filename_in && filename_out))
		print_usage();

	ret = stat(filename_in, &sb);
	if (ret) {
		fprintf(stderr, "file stat file: %s\n", filename_in);
		return ret;
	}

	file_buffer = malloc(sb.st_size);
	if (!file_buffer) {
		fprintf(stderr, "fail malloc file buffer\n");
		return -1;
	}

	fp_in = fopen(filename_in, "r");
	if (!fp_in) {
		fprintf(stderr, "fail open file: %s\n", filename_in);
		return -1;
	}

	fp_out = fopen(filename_out, "w+");
	if (!fp_out) {
		fprintf(stderr, "fail open file: %s\n", filename_out);
		return -1;
	}

	ret = fread(file_buffer, sb.st_size, 1, fp_in);
	if (ret < 0 ) {
		fprintf(stderr, "read less bytes\n");
		return -1;
	}


	for ( i = 0; i < sb.st_size; i = i + 4) {
		pix = ((file_buffer[i] &0xc0) >> 6) | ((file_buffer[i + 1] & 0xc0) >> 4) |
			((file_buffer[i + 2] & 0xc0) >> 2) | ((file_buffer[i + 3] & 0xc0));
		fwrite(&pix, 1, 1, fp_out);
	}



	if (filename_in)
		fclose(fp_in);
	if (filename_out)
		fclose(fp_out);
	if (file_buffer)
		free(file_buffer);

	return 0;
}

