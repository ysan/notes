#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "simple_list.h"


/*
 * Constant define
 */


/*
 * Type define
 */

/*
 * Variables
 */

/*
 * Prototypes
 */
simple_list_if_t * create_simple_list (int limit); // extren
static size_t _size (simple_list_if_t *self);
static simple_list_t * _create (simple_list_if_t *self, void *data, size_t size);
static bool _append (simple_list_if_t *self, simple_list_t *queue);
static void _remove (
	simple_list_if_t *self,
	simple_list_t *addr,
	int idx,
	uint8_t flag,
	void(*rm_cb)(simple_list_t *rm_q)
);
static void _remove_all (simple_list_if_t *self, void(*rm_cb)(simple_list_t *rm_q));
static simple_list_t * _ref (simple_list_if_t *self, int idx);
//static bool _attach_data (simple_list_if_t *self, void *data, size_t size);
static simple_list_t* _foreach (
	simple_list_if_t *self,
	simple_list_foreach_rtncode_t (*cb)(simple_list_t *q)
);
//static simple_list_t* _get_unused (simple_list_if_t *self);
static void _destroy(simple_list_if_t *self, void(*rm_cb)(simple_list_t *rm_q));


simple_list_if_t * create_simple_list (int limit)
{
	size_t n = sizeof(simple_list_if_t) + sizeof(simple_list_private_t);
	void *p = calloc (1, n);
	if (!p) {
		return NULL;
	}

	simple_list_if_t *self = (simple_list_if_t*)p;
	self->size = _size;
	self->create = _create;
	self->append = _append;
	self->remove = _remove;
	self->ref = _ref;
//	self->attach_data = _attach_data;
	self->foreach = _foreach;
//	self->get_unused = _get_unused;
	self->destroy = _destroy;

	simple_list_private_t* _private = (simple_list_private_t*)(self +1);
	self->private = _private;
	self->private->limit = limit;

	return self;
}

static size_t _size (simple_list_if_t *self)
{
	if (!self) {
		return 0;
	}
	if (!self->private->head) {
		// list emply
		return 0;
	}

	simple_list_t *current = self->private->head;
	size_t size = 0;

	while (current) {
		++ size;
		current = current->next;
	}

	return size;
}

static simple_list_t * _create (simple_list_if_t *self, void *data, size_t size)
{
	simple_list_t *p = malloc(sizeof(simple_list_t));
	if (!p) {
		return NULL;
	}
	p->data = data;
	p->size = size;
	p->next = NULL;
//	p->is_used = false;
	return p;
}

static bool _append (simple_list_if_t *self, simple_list_t *queue)
{
	if (!self) {
		return false;
	}

	if ((int)_size(self) == self->private->limit) {
		return false;
	}

	if (!self->private->head) {
		// list empty
		self->private->head = queue;
		self->private->tail = queue;

	} else {
		self->private->tail->next = queue;
		self->private->tail = queue;
	}

	return true;
}

static void _remove (
	simple_list_if_t *self,
	simple_list_t *addr,
	int idx,
	uint8_t flag,
	void(*rm_cb)(simple_list_t *rm_q)
)
{
	if (!self) {
		return;
	}
	if (!self->private->head) {
		// list emply
		return;
	}

	simple_list_t *current = self->private->head;
	simple_list_t *before = NULL;
	simple_list_t *remove_queue = NULL;
	int _idx = 0;
	int remove_by = flag & SIMPLE_LIST_REMOVE_BY_MASK;

	while (current) {
		if (
			((remove_by == SIMPLE_LIST_REMOVE_BY_ADDR) && (current == addr)) ||
			((remove_by == SIMPLE_LIST_REMOVE_BY_IDX) && (idx == _idx))
		) {
			remove_queue = current;

			if (current == self->private->head) {
				if (current->next) {
					self->private->head = current->next;
					current->next = NULL;
				} else {
					// only one element
					self->private->head = NULL;
					self->private->tail = NULL;
					current->next = NULL;
				}

			} else if (current == self->private->tail) {
				if (before) {
					before->next = NULL;
					self->private->tail = before;
				}

			} else {
				before->next = current->next;
				current->next = NULL;
			}

			break;

		} else {
			// cache
			before = current;
		}

		++ _idx;

		// next
		current = current->next;
	}

	if (rm_cb) {
		rm_cb (remove_queue);
	}

	if (flag & SIMPLE_LIST_REMOVE_RECYCLE) {
		_append (self, remove_queue);
	} else {
		free(remove_queue);
	}
}

static void _remove_all (simple_list_if_t *self, void(*rm_cb)(simple_list_t *rm_q))
{
	if (!self) {
		return;
	}
	if (!self->private->head) {
		// list emply
		return;
	}

	simple_list_t *current = self->private->head;

	while (current) {
		if (rm_cb) {
			rm_cb (current);
		}
		free(current);
		current = current->next;
	}
}

static simple_list_t * _ref (simple_list_if_t *self, int idx)
{
	if (!self) {
		return NULL;
	}
	if (!self->private->head) {
		// list emply
		return NULL;
	}

	simple_list_t *current = self->private->head;
	int _idx = 0;

	while (current) {
		if (idx == _idx) {
			return current;
		}
		++ _idx;
		current = current->next;
	}

	return NULL;

} 

/*
static bool _attach_data (simple_list_if_t *self, void *data, size_t size)
{
	if (!self) {
		return false;
	}
	if (!self->private->head) {
		// list emply
		return false;
	}

	simple_list_t *current = self->private->head;

	while (current) {
		if (!current->data && current->size == 0) {
			current->data = data;
			current->size = size;
//			current->is_used = true;
			return true;
		}

		current = current->next;
	}

	// full used
	return false;
}
*/

static simple_list_t* _foreach (
	simple_list_if_t *self,
	simple_list_foreach_rtncode_t (*cb)(simple_list_t *q)
)
{
	if (!self) {
		return NULL;
	}
	if (!self->private->head) {
		// list emply
		return NULL;
	}

	simple_list_t *current = self->private->head;

	while (current) {
		if (cb) {
			simple_list_foreach_rtncode_t r = cb (current);
			if (r == SIMPLE_LIST_FOREACH_RETURN_ADDR) {
				return current;
			}
		}
		current = current->next;
	}

	return NULL;
}

/*
static simple_list_t* _get_unused (simple_list_if_t *self)
{
	if (!self) {
		return NULL;
	}
	if (!self->private->head) {
		// list emply
		return NULL;
	}

	simple_list_t *current = self->private->head;

	while (current) {
		if (!current->is_used) {
			return current;
		}
		current = current->next;
	}

	// full used
	return NULL;
}
*/

void _destroy(simple_list_if_t *self, void(*rm_cb)(simple_list_t *rm_q))
{
	if (!self) {
		return;
	}

	_remove_all(self, rm_cb);

	free(self);
}


#if 1
//--------------    for debug/test    ----------------
typedef struct {
	int v;
	char *p;
	struct _msg {
		char data[256];
	} msg;
} test_t;
simple_list_foreach_rtncode_t cb (simple_list_t *q) {
	test_t *p = (test_t*)q->data;
	printf("cb %p %ld - %d %s\n", q->data, q->size, p->v, p->p);
	return SIMPLE_LIST_FOREACH_NORMAL;
}
simple_list_foreach_rtncode_t cb_get (simple_list_t *q) {
	test_t *p = (test_t*)q->data;
	printf("cb_get %p %ld - %d %s\n", q->data, q->size, p->v, p->p);
	if (p->v == 3) {
		return SIMPLE_LIST_FOREACH_RETURN_ADDR;
	} else {
		return SIMPLE_LIST_FOREACH_NORMAL;
	}
}
void cb_rm (simple_list_t *q) {
	test_t *p = (test_t*)q->data;
	printf("cb_rm %p %ld - %d %s\n", q->data, q->size, p->v, p->p);
}
#include <time.h>
#define SEC2NSEC (1 * 1000 * 1000 * 1000)
void diff_timespec(struct timespec *result, struct timespec *end, struct timespec *begin) {
	result->tv_sec  = end->tv_sec  - begin->tv_sec;
	result->tv_nsec = end->tv_nsec - begin->tv_nsec;
	if (result->tv_nsec < 0) {
		result->tv_sec--;
		result->tv_nsec += SEC2NSEC;
	}
}
int main (void) 
{
	test_t tests[] = {
		{0, "test0", {"AAA"}},
		{1, "test1", {"BBB"}},
		{2, "test2", {"CCC"}},
		{3, "test3", {"DDD"}},
		{4, "test4", {"EEE"}},
		{5, "test5", {"FFF"}},
		{6, "test6", {"GGG"}},
		{7, "test7", {"HHH"}},
		{8, "test8", {"III"}},
		{9, "test9", {"JJJ"}},
	};

	simple_list_if_t *obj = create_simple_list(sizeof(tests)/sizeof(test_t));
	printf("size %ld\n", obj->size(obj));

	for (int i = 0; i < sizeof(tests)/sizeof(test_t); ++ i) {
		simple_list_t *q = obj->create(obj, &tests[i], sizeof(test_t));
		bool r = obj->append(obj, q);
		printf("append %d\n", r);
	}
	{
		// err
		simple_list_t *q = obj->create(obj, &tests[0], sizeof(test_t));
		bool r = obj->append(obj, q);
		printf("append %d\n", r);
		free(q);
	}


	printf("size %ld\n", obj->size(obj));



	obj->foreach(obj, cb);
	puts("");

	simple_list_t *q = obj->foreach(obj, cb_get);
	printf("get %p\n", q->data);
	puts("");

	uint8_t flg = SIMPLE_LIST_REMOVE_BY_ADDR | SIMPLE_LIST_REMOVE_RECYCLE;
	obj->remove(obj, q, 0, flg, NULL);
	puts("");

	obj->foreach(obj, cb);
	puts("");

	flg = SIMPLE_LIST_REMOVE_BY_IDX | SIMPLE_LIST_REMOVE_RECYCLE;
	obj->remove(obj, NULL, 4, flg, NULL);
	puts("");

	obj->foreach(obj, cb);
	puts("");

	flg = SIMPLE_LIST_REMOVE_BY_IDX;
	obj->remove(obj, NULL, 7, flg, cb_rm);
	obj->remove(obj, NULL, 6, flg, cb_rm);
	obj->remove(obj, NULL, 5, flg, cb_rm);
	puts("");

	obj->foreach(obj, cb);
	puts("");

	q = obj->foreach(obj, cb_get);
	test_t *t = (test_t*)q->data;
	t->v += 10;

	obj->foreach(obj, cb);
	puts("");

	obj->destroy(obj, NULL);


	{
		simple_list_if_t *obj = create_simple_list(sizeof(tests)/sizeof(test_t));
	
		for (int i = 0; i < sizeof(tests)/sizeof(test_t); ++ i) {
			simple_list_t *q = obj->create(obj, &tests[i], sizeof(test_t));
			bool r = obj->append(obj, q);
			printf("append %d\n", r);
		}
		printf("size %ld\n", obj->size(obj));

	    struct timespec begin;
	    struct timespec end;
	    struct timespec result;
		clock_gettime(CLOCK_MONOTONIC, &begin);

		for (int j = 0; j < 10000; ++ j) {
			for (int i = 0; i < sizeof(tests)/sizeof(test_t); ++ i) {
				simple_list_t *q = obj->ref(obj, 0);
				flg = SIMPLE_LIST_REMOVE_BY_ADDR | SIMPLE_LIST_REMOVE_RECYCLE;
				obj->remove(obj, q, 0, flg, NULL);
//				obj->foreach(obj, cb);
//				puts("");
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &end);
		diff_timespec(&result, &end, &begin);
		printf("linklist elapsed = %lf [s]\n", result.tv_sec + (double) result.tv_nsec / SEC2NSEC);

		obj->destroy(obj, NULL);
	}

	{
	    struct timespec begin;
	    struct timespec end;
	    struct timespec result;
		clock_gettime(CLOCK_MONOTONIC, &begin);

		for (int j = 0; j < 10000; ++ j) {
			test_t r = tests [0];
			for (int i = 0; i < sizeof(tests)/sizeof(test_t); ++ i) {
				if (i < sizeof(tests)/sizeof(test_t) -1) {
					memcpy (&tests [i], &tests [i+1], sizeof(test_t));
				} else {
					tests [i] = r;
				}
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &end);
		diff_timespec(&result, &end, &begin);
		printf("array    elapsed = %lf [s]\n", result.tv_sec + (double) result.tv_nsec / SEC2NSEC);
	}


	return 0;
}
#endif
