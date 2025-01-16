#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pdm_test.h"

static struct pdm_test_unit *pdm_test_units[] = {
	&pdm_test_unit_show_help,
//	&pdm_test_unit_switch,
	NULL
};


static struct pdm_test_unit *pdm_test_match_unit(struct pdm_test_unit **units,
const char *name)
{
	if (!units || !name) {
		return NULL;
	}

	while (*units != NULL) {
		if (strcmp((*units)->name, name) == 0) {
			return *units;
		}
		units++;
	}

	return NULL;
}

static void pdm_test_show_help_comment(void)
{
	printf("show this message.\n");
}

static int pdm_test_show_help(int argc, char *argv[])
{
	struct pdm_test_unit **units = pdm_test_units;

	(void)argc;
	(void)argv;

	printf("\n### Available commands:\n");

	while (*units != NULL) {
		printf("\n------------------\n");
		printf(" - %s:\n", (*units)->name);
		(*units)->comment_func();
		units++;
	}

	printf("\n### \n\n");

	return 0;
}

struct pdm_test_unit pdm_test_unit_show_help = {
	.name = "show_help",
	.comment_func = pdm_test_show_help_comment,
	.main_func = pdm_test_show_help
};

static int pdm_test_dispatch_command(int argc, char *argv[])
{
	struct pdm_test_unit *unit;

	if (!argv[0]) {
		fprintf(stderr, "Error: No command provided.\n");
		return -1;
	}

	unit = pdm_test_match_unit(pdm_test_units, argv[0]);
	if (!unit) {
		printf("Error: Unknown command '%s'.\n", argv[0]);
		return pdm_test_show_help(argc, argv);
	}

	printf("[CMD]: %s\n", unit->name);
	return unit->main_func(argc, argv);
}

int main(int argc, char *argv[])
{
	if (argc < 1) {
		return -1;
	}

	printf("\n====== PDM Test ======\n");

	if (!strncmp(argv[0], "./", 2)) {
		argv[0] += 2;
	}

	if (strcmp(argv[0], PDM_TEST_PROGRAM_NAME)) {
		return pdm_test_dispatch_command(argc, argv);
	} else if (!strcmp(argv[0], PDM_TEST_PROGRAM_NAME) && (1 == argc)) {
		return pdm_test_show_help(argc, argv);
	} else if (!strcmp(argv[0], PDM_TEST_PROGRAM_NAME) && (argc > 1)) {
		return pdm_test_dispatch_command(argc - 1, argv + 1);
	} else {
		return pdm_test_dispatch_command(argc, argv);
	}
}
