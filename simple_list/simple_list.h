#ifndef _SIMPLE_LIST_H_
#define _SIMPLE_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


/*
 * Constant define
 */
#define SIMPLE_LIST_REMOVE_BY_ADDR  (0x00)
#define SIMPLE_LIST_REMOVE_BY_IDX   (0x01)
#define SIMPLE_LIST_REMOVE_BY_xxx   (0x02)

#define SIMPLE_LIST_REMOVE_BY_MASK  (0x03) // 2bits mask
#define SIMPLE_LIST_REMOVE_RECYCLE  (0x04)


/*
 * Type define
 */
#if !defined (_NO_TYPEDEF_uint8_t)
typedef unsigned char uint8_t;
#endif

#if !defined (_NO_TYPEDEF_uint16_t)
typedef unsigned short uint16_t;
#endif

#if !defined (_NO_TYPEDEF_uint32_t)
typedef unsigned int uint32_t;
#endif

#if !defined (_NO_TYPEDEF_uint64_t)
typedef unsigned long int uint64_t;
#endif

typedef enum {
	SIMPLE_LIST_FOREACH_NORMAL = 0,
	SIMPLE_LIST_FOREACH_RETURN_ADDR,
} simple_list_foreach_rtncode_t;

typedef struct _queue {
	void* data;
	size_t size;
//	bool is_used;

	struct _queue * next;

} simple_list_t;

typedef struct _private {
	simple_list_t * head;
	simple_list_t * tail;
	int limit;

} simple_list_private_t;

typedef struct _if {
	size_t (*size) (struct _if *self);
	simple_list_t* (*create) (struct _if *self, void *data, size_t size);
	bool (*append) (struct _if *self, simple_list_t *queue);
	void (*remove) (
		struct _if *self,
		simple_list_t *queue,
		int idx,
		uint8_t flag,
		void(*rm_cb)(simple_list_t *rm_q)
	);
	simple_list_t* (*ref) (struct _if *self, int idx);
//	bool (*attach_data) (struct _if *self, void *data, size_t size);
	simple_list_t* (*foreach) (
		struct _if *self,
		simple_list_foreach_rtncode_t (*cb)(simple_list_t *q)
	);
//	simple_list_t* (*get_unused) (struct _if *self);
	void (*destroy) (struct _if *self, void(*rm_cb)(simple_list_t *rm_q));


	simple_list_private_t * private;

} simple_list_if_t;


/*
 * External
 */
simple_list_if_t * create_simple_list(int limit);

#endif
