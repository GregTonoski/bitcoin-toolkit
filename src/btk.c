/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <regex.h>
#include <time.h>
#include "mods/input.h"
#include "mods/json.h"
#include "mods/qrcode.h"
#include "mods/config.h"
#include "mods/opts.h"
#include "mods/error.h"
#include "ctrl_mods/btk_help.h"
#include "ctrl_mods/btk_privkey.h"
#include "ctrl_mods/btk_pubkey.h"
#include "ctrl_mods/btk_address.h"
#include "ctrl_mods/btk_node.h"
#include "ctrl_mods/btk_balance.h"
#include "ctrl_mods/btk_config.h"
#include "ctrl_mods/btk_version.h"

#define BTK_CHECK_NEG(x, y)         if (x < 0) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_NULL(x, y)        if (x == NULL) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_FALSE(x, y)       if (!x) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }
#define BTK_CHECK_TRUE(x, y)        if (x) { error_log(y); error_log("Error [%s]:", command_str); error_print(); return EXIT_FAILURE; }

static regex_t grep;

int btk_set_config_opts(opts_p);
int btk_init(opts_p);
int btk_cleanup(opts_p);
int btk_print_output(output_item, opts_p);

int main(int argc, char *argv[])
{
	int i, r = 0;
	char *command = NULL;
	char command_str[BUFSIZ];
	opts_p opts = NULL;
	input_item input = NULL;
	output_item output = NULL;
	int output_offset = 0;

	int (*command_init)(opts_p) = NULL;
	int (*command_requires_input)(opts_p) = NULL;
	int (*command_main)(output_item *, opts_p, unsigned char *, size_t) = NULL;
	int (*command_cleanup)(opts_p) = NULL;

	// Assembling the original command string for logging purposes
	memset(command_str, 0, BUFSIZ);
	for (i = 0; i < argc; ++i)
	{
		if (i > 0)
		{
			strncat(command_str, " ", BUFSIZ - 1 - strlen(command_str));
		}
		strncat(command_str, argv[i], BUFSIZ - 1 - strlen(command_str));
	}

	BTK_CHECK_TRUE((argc <= 1), "Missing command parameter.");

	command = argv[1];

	if (strcmp(command, "privkey") == 0)
	{
		command_main = &btk_privkey_main;
		command_requires_input = &btk_privkey_requires_input;
		command_init = &btk_privkey_init;
		command_cleanup = &btk_privkey_cleanup;
	}
	else if (strcmp(command, "pubkey") == 0)
	{
		command_main = &btk_pubkey_main;
		command_requires_input = &btk_pubkey_requires_input;
		command_init = &btk_pubkey_init;
		command_cleanup = &btk_pubkey_cleanup;
	}
	else if (strcmp(command, "address") == 0)
	{
		command_main = &btk_address_main;
		command_requires_input = &btk_address_requires_input;
		command_init = &btk_address_init;
		command_cleanup = &btk_address_cleanup;
	}
	else if (strcmp(command, "node") == 0)
	{
		command_main = &btk_node_main;
		command_requires_input = &btk_node_requires_input;
		command_init = &btk_node_init;
		command_cleanup = &btk_node_cleanup;
	}
	else if (strcmp(command, "balance") == 0)
	{
		command_main = &btk_balance_main;
		command_requires_input = &btk_balance_requires_input;
		command_init = &btk_balance_init;
		command_cleanup = &btk_balance_cleanup;
	}
	else if (strcmp(command, "config") == 0)
	{
		command_main = &btk_config_main;
		command_requires_input = &btk_config_requires_input;
		command_init = &btk_config_init;
		command_cleanup = &btk_config_cleanup;
	}
	else if (strcmp(command, "version") == 0)
	{
		command_main = &btk_version_main;
		command_requires_input = &btk_version_requires_input;
		command_init = &btk_version_init;
		command_cleanup = &btk_version_cleanup;
	}
	else if (strcmp(command, "help") == 0)
	{
		command_main = &btk_help_main;
		command_requires_input = &btk_help_requires_input;
		command_init = &btk_help_init;
		command_cleanup = &btk_help_cleanup;
	}
	else
	{
		error_log("See 'btk help' to read about available commands.");
		error_log("'%s' is not a valid command.", command);
		error_log("Error [%s]:", command_str);
		error_print();
		return EXIT_FAILURE;
	}

	opts = malloc(sizeof(*opts));
	BTK_CHECK_NULL(opts, "Memory allocation error.");

	r = opts_init(opts, command);
	BTK_CHECK_NEG(r, NULL);

	r = opts_get(opts, argc, argv);
	BTK_CHECK_NEG(r, NULL);

	r = btk_set_config_opts(opts);
	BTK_CHECK_NEG(r, "Could not set opts from config.");

	r = btk_init(opts);
	BTK_CHECK_NEG(r, "Could not initialize btk.");

	r = command_init(opts);
	BTK_CHECK_NEG(r, "Initialization error.");

	if (command_requires_input(opts))
	{
		if (opts->input_format_binary)
		{
			r = input_get(&input);
			BTK_CHECK_NEG(r, NULL);

			r = command_main(&output, opts, input->data, input->len);
			BTK_CHECK_NEG(r, NULL);

			if (opts->output_stream)
			{
				r = btk_print_output(output, opts);
				BTK_CHECK_NEG(r, "Error printing output.");

				output_free(output);
				output = NULL;
			}
		}
		else if (opts->input_format_list)
		{
			while ((r = input_get_line(&input)) > 0)
			{
				// Ignore empty strings
				if (input->len == 0)
				{
					free(input);
					input = NULL;

					continue;
				}

				output_offset = output_length(output);

				r = command_main(&output, opts, input->data, input->len);
				BTK_CHECK_NEG(r, NULL);

				r = output_append_input(output, input, output_offset);
				ERROR_CHECK_NEG(r, "Coul dnot append input to output list.");

				if (opts->output_stream)
				{
					r = btk_print_output(output, opts);
					BTK_CHECK_NEG(r, "Error printing output.");

					output_free(output);
					output = NULL;
				}
			}
			BTK_CHECK_NEG(r, "Error getting list input.");
		}
		else if (opts->input_format_json)
		{
			while ((r = input_get_json(&input)) > 0)
			{
				while (input != NULL)
				{
					output_offset = output_length(output);

					r = command_main(&output, opts, input->data, input->len);
					BTK_CHECK_NEG(r, NULL);

					r = output_append_input(output, input, output_offset);
					ERROR_CHECK_NEG(r, "Coul dnot append input to output list.");

					input = input->next;
				}

				if (opts->output_stream)
				{
					r = btk_print_output(output, opts);
					BTK_CHECK_NEG(r, "Error printing output.");

					output_free(output);
					output = NULL;
				}
			}
			BTK_CHECK_NEG(r, "Error getting json input.");
		}
		else
		{
			// In theory, we shouldn't ever get here. If we do, check that
			// we are setting the defaults properly in btk_init()
			BTK_CHECK_NEG(-1, "No input format specified.");
		}
	}
	else
	{
		if (opts->output_stream)
		{
			while (1)
			{
				r = command_main(&output, opts, NULL, 0);
				BTK_CHECK_NEG(r, NULL);

				r = btk_print_output(output, opts);
				BTK_CHECK_NEG(r, "Error printing output.");

				output_free(output);
				output = NULL;
			}
		}
		else
		{
			r = command_main(&output, opts, NULL, 0);
			BTK_CHECK_NEG(r, NULL);
		}
	}
	
	if (output)
	{
		r = btk_print_output(output, opts);
		BTK_CHECK_NEG(r, "Error printing output.");

		output_free(output);
	}

	r = command_cleanup(opts);
	BTK_CHECK_NEG(r, "Cleanup error.");

	r = btk_cleanup(opts);
	BTK_CHECK_NEG(r, "Could not cleanup btk.");

	return EXIT_SUCCESS;
}

int btk_set_config_opts(opts_p opts)
{
	int r;
	char config_path[BUFSIZ];
	memset(config_path, 0, BUFSIZ);

	r = config_get_path(config_path);
    ERROR_CHECK_NEG(r, "Could not get config path.");

    r = config_load(config_path);
    ERROR_CHECK_NEG(r, "Could not load config.");

    // Set opts on a per command basis.
    if (strcmp(opts->command, "balance") == 0)
    {
        if (!opts->host_name && config_exists("hostname"))
        {
            opts->host_name = malloc(100);
            ERROR_CHECK_NULL(opts->host_name, "Memory allocation error.");

            r = config_get(opts->host_name, "hostname");
            ERROR_CHECK_NEG(r, "Could not get value from config file.");
        }

        if (!opts->rpc_auth && config_exists("rpc-auth"))
        {
            opts->rpc_auth = malloc(100);
            ERROR_CHECK_NULL(opts->rpc_auth, "Memory allocation error.");

            r = config_get(opts->rpc_auth, "rpc-auth");
            ERROR_CHECK_NEG(r, "Could not get value from config file.");
        }
    }

    config_unload();

    return 1;
}

int btk_init(opts_p opts)
{
	int r, i;

	// Sanity check the opts

	i = 0;
	if (opts->input_format_binary) { i++; }
	if (opts->input_format_list) { i++; }
	if (opts->input_format_json) { i++; }
	ERROR_CHECK_TRUE((i > 1), "Can not use multiple input formats.");
	if (i == 0)
	{
		opts->input_format_json = 1;
	}

	i = 0;
	if (opts->output_format_binary) { i++; }
	if (opts->output_format_list) { i++; }
	if (opts->output_format_qrcode) { i++; }
	if (opts->output_format_json) { i++; }
	ERROR_CHECK_TRUE((i > 1), "Can not use multiple output formats.");
	if (i == 0)
	{
		opts->output_format_json = 1;
	}

	ERROR_CHECK_TRUE(opts->output_format_binary && opts->output_grep, "Can not grep on binary formatted output.");

	// Compile regex for grep here.

	if (opts->output_grep)
	{
		r = regcomp(&grep, opts->output_grep, REG_ICASE|REG_NOSUB);
		ERROR_CHECK_TRUE(r, "Could not compile regex for grep option.");
	}

	return 1;
}

int btk_cleanup(opts_p opts)
{
	if (opts->output_grep)
	{
		regfree(&grep);
	}

	return 1;
}

int btk_print_output(output_item output, opts_p opts)
{
	int r;
	size_t i;
	char qrcode_str[BUFSIZ];
	cJSON *json_output = NULL;
	char *json_output_str = NULL;

	assert(output);

	if (opts->output_format_list)
	{
		while(output)
		{
			if (opts->output_grep)
			{
				r = regexec(&grep, (char *)(output->content), 0, NULL, 0);
				if (r == REG_NOMATCH)
				{
					output = output->next;
					continue;
				}
			}

			printf("%s\n", (char *)(output->content));

			output = output->next;
		}
	}
	else if (opts->output_format_qrcode)
	{
		while(output)
		{
			if (opts->output_grep)
			{
				r = regexec(&grep, (char *)(output->content), 0, NULL, 0);
				if (r == REG_NOMATCH)
				{
					output = output->next;
					continue;
				}
			}

			memset(qrcode_str, 0, BUFSIZ);

			r = qrcode_from_str(qrcode_str, (char *)(output->content));
			ERROR_CHECK_NEG(r, "Can not generate qr code.");

			printf("\n%s\n", qrcode_str);

			output = output->next;
		}
	}
	else if (opts->output_format_binary)
	{
		while(output)
		{
			for (i = 0; i < output->length; i++)
			{
				fputc(((unsigned char *)(output->content))[i], stdout);
			}

			output = output->next;
		}
	}
	else if (opts->output_format_json)
	{
		char output_item_str[BUFSIZ];
		char input_item_str[BUFSIZ];

		r = json_init_output(&json_output, opts->trace);
		ERROR_CHECK_NEG(r, "Error initializing JSON output.");

		while(output)
		{
			memset(output_item_str, 0, BUFSIZ);
			memset(input_item_str, 0, BUFSIZ);

			memcpy(output_item_str, output->content, output->length);

			if (opts->output_grep)
			{
				r = regexec(&grep, output_item_str, 0, NULL, 0);
				if (r == REG_NOMATCH)
				{
					output = output->next;
					continue;
				}
			}

			if (opts->trace && output->input)
			{
				while (output->input != NULL)
				{
					memset(input_item_str, 0, BUFSIZ);
					memcpy(input_item_str, output->input->data, output->input->len);

					r = json_add_output(json_output, output_item_str, input_item_str);
					ERROR_CHECK_NEG(r, "Output handling error.");

					memset(output_item_str, 0, BUFSIZ);
					memcpy(output_item_str, output->input->data, output->input->len);

					output->input = output->input->input;
				}
			}
			else
			{
				r = json_add_output(json_output, output_item_str, NULL);
				ERROR_CHECK_NEG(r, "Output handling error.");
			}

			output = output->next;
		}

		if (json_output_size(json_output) > 0)
		{
			r = json_to_string(&json_output_str, json_output);
			ERROR_CHECK_NEG(r, "Error converting output to JSON.");

			printf("%s\n", json_output_str);

			free(json_output_str);
		}

		json_free(json_output);
	}
	else
	{
		error_log("No output format specified.");
		return -1;
	}

	return 1;
}