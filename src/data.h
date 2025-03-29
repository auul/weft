#ifndef WEFT_DATA_H
#define WEFT_DATA_H

#include <stdbool.h>

// Forward Declarations

typedef struct weft_str Weft_Str;
typedef struct weft_list Weft_List;
typedef struct weft_fn Weft_Fn;
typedef struct weft_shuffle Weft_Shuffle;
typedef enum weft_data_type Weft_DataType;
typedef struct weft_data Weft_Data;

// Data Types

enum weft_data_type {
	WEFT_DATA_NIL,
	WEFT_DATA_BOOL,
	WEFT_DATA_NUM,
	WEFT_DATA_CHAR,
	WEFT_DATA_STR,
	WEFT_DATA_LIST,
	WEFT_DATA_FN,
	WEFT_DATA_SHUFFLE,
};

struct weft_data {
	Weft_DataType type;
	union {
		void *ptr;
		bool b;
		double num;
		char c;
		Weft_Str *str;
		Weft_List *list;
		Weft_Fn *fn;
		Weft_Shuffle *shuffle;
	};
};

// Functions

Weft_Data data_nil();
Weft_Data data_tag_bool(bool b);
Weft_Data data_tag_num(double num);
Weft_Data data_tag_char(char c);
Weft_Data data_tag_ptr(Weft_DataType type, void *ptr);
Weft_DataType data_get_type(const Weft_Data data);
bool data_get_bool(const Weft_Data data);
double data_get_num(const Weft_Data data);
void *data_get_ptr(Weft_Data data);
Weft_Str *data_stringify(const Weft_Data data);
void data_print(const Weft_Data data);
void data_mark(Weft_Data data);
bool data_is_equal(const Weft_Data left, const Weft_Data right);

#endif
