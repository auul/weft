#include "map.h"
#include "fn.h"
#include "gc.h"

void map_print(const Weft_Map *map)
{
	printf("{");
	map_print_bare(map);
	printf("}");
}

void map_print_bare(const Weft_Map *map)
{
	while (map) {
		if (map->left) {
			map_print_bare(map->left);
			printf(" ");
		}

		fn_print(map->key);

		map = map->right;
		if (map) {
			printf(" ");
		}
	}
}

void map_mark(const Weft_Map *map)
{
	while (gc_mark(map)) {
		fn_mark(map->key);
		map_mark(map->left);

		map = map->right;
	}
}

static int compare_key_to_word(const Weft_Fn *key, const Weft_Word word)
{
	size_t i;
	for (i = 0; key->name[i]; i++) {
		if (i > word.len || key->name[i] > word.src[i]) {
			return -1;
		} else if (key->name[i] < word.src[i]) {
			return 1;
		}
	}

	if (word.len > i) {
		return 1;
	}
	return 0;
}

Weft_Fn *map_lookup(Weft_Map *map, const Weft_Word word)
{
	while (map) {
		int cmp = compare_key_to_word(map->key, word);
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

static int compare_key(const Weft_Fn *left, const Weft_Fn *right)
{
	size_t i;
	for (i = 0; left->name[i]; i++) {
		if (left->name[i] > right->name[i]) {
			return -1;
		} else if (left->name[i] < right->name[i]) {
			return 1;
		}
	}

	if (right->name[i]) {
		return 1;
	}
	return 0;
}

Weft_Map *map_insert(Weft_Map *map, Weft_Fn *key)
{
	if (!map) {
		return leaf_node(key);
	}

	map = clone_node(map);
	Weft_Map *node = map;

	while (true) {
		int cmp = compare_key(node->key, key);
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

static Weft_Map *touch_node(const Weft_Word word)
{
	return leaf_node(new_fn_list(word, NULL));
}

Weft_Fn *map_touch(Weft_Map **map_p, const Weft_Word word)
{
	Weft_Map *map = *map_p;
	if (!map) {
		map = touch_node(word);
		*map_p = map;

		return map->key;
	}

	map = clone_node(map);
	*map_p = map;

	Weft_Map *node = map;

	while (true) {
		int cmp = compare_key_to_word(node->key, word);
		if (cmp < 0) {
			if (!node->left) {
				node->left = touch_node(word);
				return node->left->key;
			}

			node->left = clone_node(node->left);
			node = node->left;
		} else if (cmp > 0) {
			if (!node->right) {
				node->right = touch_node(word);
				return node->right->key;
			}

			node->right = clone_node(node->right);
			node = node->right;
		} else {
			node->key = new_fn_list(word, NULL);
			return node->key;
		}
	}
}
