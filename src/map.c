#include "map.h"
#include "fn.h"
#include "gc.h"
#include "str.h"

#include <stdio.h>

void map_print(const Weft_Map *map)
{
	while (map) {
		if (map->left) {
			map_print(map->left);
		}

		printf("%s\n", str_get_c(fn_stringify_def(map->key)));

		map = map->right;
	}
}

void map_mark(Weft_Map *map)
{
	while (!gc_mark(map)) {
		fn_mark(map->key);
		map_mark(map->left);

		map = map->right;
	}
}

static int
compare_key_to_word(const Weft_Fn *key, const char *word, size_t word_len)
{
	const char *key_name = fn_get_name(key);
	for (size_t i = 0; i < word_len; i++) {
		if (key_name[i] > word[i]) {
			return -1;
		} else if (key_name[i] < word[i]) {
			return 1;
		}
	}

	if (key_name[word_len]) {
		return -1;
	}
	return 0;
}

Weft_Fn *map_lookup(Weft_Map *map, const char *word, size_t word_len)
{
	while (map) {
		int cmp = compare_key_to_word(map->key, word, word_len);
		if (cmp < 0) {
			map = map->left;
		} else if (cmp > 0) {
			map = map->right;
		} else {
			return map->key;
		}
	}
	return NULL;
}

static Weft_Map *new_node(Weft_Fn *key, Weft_Map *left, Weft_Map *right)
{
	Weft_Map *node = gc_alloc(sizeof(Weft_Map));
	node->key = key;
	node->left = left;
	node->right = right;

	return node;
}

static Weft_Map *leaf_node(Weft_Fn *key)
{
	return new_node(key, NULL, NULL);
}

static Weft_Map *clone_node(Weft_Map *node)
{
	return new_node(node->key, node->left, node->right);
}

static int compare_keys(const Weft_Fn *left, const Weft_Fn *right)
{
	const char *left_name = fn_get_name(left);
	const char *right_name = fn_get_name(right);

	size_t i;
	for (i = 0; left_name[i]; i++) {
		if (left_name[i] > right_name[i]) {
			return -1;
		} else if (left_name[i] < right_name[i]) {
			return 1;
		}
	}

	if (right_name[i]) {
		return 1;
	}
	return 0;
}

Weft_Map *map_insert(Weft_Map *map, Weft_Fn *key)
{
	if (!map) {
		map = leaf_node(key);
	}

	map = clone_node(map);
	Weft_Map *node = map;

	while (true) {
		int cmp = compare_keys(node->key, key);
		if (cmp < 0) {
			if (!node->left) {
				node->left = leaf_node(key);
				return map;
			}

			node->left = clone_node(node->left);
			node = node->left;
		} else if (cmp > 0) {
			if (!node->right) {
				node->right = leaf_node(key);
				return map;
			}

			node->right = clone_node(node->right);
			node = node->right;
		} else {
			node->key = key;
			return map;
		}
	}
}
