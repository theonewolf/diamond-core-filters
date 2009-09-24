/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <cstdlib>
#include <cstdio>

#include "plugin-runner.h"
#include "factory.h"

static bool
sc(const char *a, const char *b) {
	return strcmp(a, b) == 0;
}

static void
print_key_value(const char *key,
		const char *value) {
	printf("K %d\n%s\n", strlen(key), key);
	printf("V %d\n%s\n", strlen(value), value);
}

static void
print_key_value(const char *key,
		int value_len,
		void *value) {
	printf("K %d\n%s\n", strlen(key), key);
	printf("V %d\n", value_len);
	fwrite(value, value_len, 1, stdout);
	printf("\n");
}

static void
print_plugin(const char *type, img_factory *imgf) {
	print_key_value("type", type);
	print_key_value("display-name", imgf->get_name());
	print_key_value("internal-name", imgf->get_description());
	printf("\n");
}

void
list_plugins(void) {
	void *cookie;

	img_factory *imgf;

	imgf = get_first_factory(&cookie);
	if (imgf != NULL) {
		do {
			print_plugin("filter", imgf);
		} while((imgf = get_next_factory(&cookie)));
	}

	imgf = get_first_codec_factory(&cookie);
	if (imgf != NULL) {
		do {
			print_plugin("codec", imgf);
		} while((imgf = get_next_factory(&cookie)));
	}
}

static img_search *get_plugin(const char *type,
			      const char *internal_name) {
	img_factory *imgf;

	if (sc(type, "filter")) {
		imgf = find_factory(internal_name);
	} else if (sc(type, "codec")) {
		imgf = find_codec_factory(internal_name);
	} else {
		printf("Invalid type\n");
		return NULL;
	}

	if (!imgf) {
	  return NULL;
	}

	return imgf->create("filter");
}

static void
print_search_config(img_search *search) {
	// editable?
	print_key_value("is-editable", search->is_editable() ? "true" : "false");

	// print blob
	print_key_value("blob", search->get_auxiliary_data_length(),
			search->get_auxiliary_data());

	// newline separator
	printf("\n");

	// print config
	if (search->is_editable()) {
		search->write_config(stdout, NULL);
	}
}

int
get_plugin_initial_config(const char *type,
			  const char *internal_name) {
	img_search *search = get_plugin(type, internal_name);
	if (search == NULL) {
		printf("Can't find %s\n", internal_name);
		return 1;
	}

	print_search_config(search);

	return 0;
}

int
edit_plugin_config(const char *type,
		   const char *internal_name) {
	img_search *search = get_plugin(type, internal_name);
	if (search == NULL) {
		printf("Can't find %s\n", internal_name);
		return 1;
	}

	if (!search->is_editable()) {
		printf("Not editable");
		return 1;
	}

	// TODO take in config from user

	search->set_exit_gui_on_close_edit_win(true);
	search->edit_search();
	gtk_main();

	print_search_config(search);

	return 0;
}
