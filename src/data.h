#ifndef WEFT_DATA_H
#define WEFT_DATA_H

// Forward Declarations

typedef enum weft_data_type Weft_DataType;
typedef struct weft_data Weft_Data;
typedef struct weft_list Weft_List;
typedef struct weft_fn Weft_Fn;

// Data Types

enum weft_data_type {
	WEFT_DATA_FN,
	WEFT_DATA_LIST,
};

struct weft_data {
	Weft_DataType type;
	union {
		void *ptr;
		Weft_Fn *fn;
		Weft_List *list;
	};
};

// Functions

Weft_Data data_tag_fn(Weft_Fn *fn);
Weft_Data data_tag_list(Weft_List *list);
Weft_DataType data_type(const Weft_Data data);
void *data_ptr(const Weft_Data data);
void data_print(const Weft_Data data);
void data_mark(const Weft_Data data);

#endif
