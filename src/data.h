#include "debug.h"

#ifndef WEFT_DATA_H
#define WEFT_DATA_H

#include <stdbool.h>

#ifdef WEFT_DEBUG_ALL
#define WEFT_DEBUG_DATA
#endif

// Forward Declarations

typedef enum weft_data_type Weft_DataType;
typedef struct weft_data Weft_Data;

// Data Types

enum weft_data_type {
	WEFT_DATA_BOOL,
	WEFT_DATA_NUM,
	WEFT_DATA_LIST,
	WEFT_DATA_FN,
	WEFT_DATA_SHUFFLE,
};

struct weft_data {
	Weft_DataType type;
	union {
		void *ptr;
		bool bnum;
		double num;
	};
};

// Functions

Weft_Data data_tag_bool(bool bnum);
Weft_Data data_tag_num(double num);
Weft_Data data_tag_ptr(Weft_DataType type, void *ptr);
Weft_DataType data_type(const Weft_Data data);
bool data_bool(const Weft_Data data);
double data_num(const Weft_Data data);
void *data_ptr(Weft_Data data);
const void *data_const_ptr(const Weft_Data data);
void data_print(const Weft_Data data);
void data_mark(Weft_Data data);
bool data_equal(const Weft_Data left, const Weft_Data right);

#endif
