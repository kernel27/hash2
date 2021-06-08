#ifndef _HASH2_H
#define _HASH2_H

#include <stdbool.h> //bool, false, true

#ifndef uint8_t
#include <stdint.h>
#endif

//table size
uint32_t T = 100;
//delete type
int8_t dtype;

//(option, request) array
uint8_t O[] = {
	'h',
	'A', 'd', 'w',
	'B', 'b', 'a', 'c', 'r', 's', 'o',
	'S', 't', 'D'
};
//	need sizeof here for const array
bool    R[sizeof(O)] = { false };

//(delimiter, symbol) array
uint8_t D[] = {'\\', ' ', '\t', '\b', '\n', '\r', '\f', '\'', '\"'};
uint8_t S[] = {'\\', ' ',  't',  'b',  'n',  'r',  'f', '\'',  '"'};
//additional (delimiter, request) array
uint8_t A[] = {'<', '>', '{', '}', '(', ')', '[', ']'};
//	need sizeof here for const array
bool   AR[sizeof(A)] = { false };

//number of (options, delimiters, additionals)
uint8_t osize = sizeof(O), dsize = sizeof(D), asize = sizeof(A);

//no padding start
#pragma pack(push, 1)
typedef struct word {
	/*
	@type : 0 - delimiter | 1 - identifier
	@size : @name length with '\0'
	@count: @name count
	@next : next @word (if hash collision)
	@prev : previous @word (for linking)
	@name : C99 flexible array member (FAM)
	*/
	bool           type;
	uint32_t       size;
  uint32_t      count;
  struct word*   next;
	struct word*   prev;
  uint8_t      name[];
} word;
#pragma pack(pop)
//no padding end

//hash table
word** table;

//alternative no padding (this struct only)
typedef struct __attribute__((__packed__)) {
	uint32_t   pos;
	uint32_t  size;
	uint8_t  buf[];
} buffer;

//other delimiter buffer
buffer* od;

//struct static members size is const
uint8_t wsize = sizeof(word), bsize = sizeof(buffer);

#endif /* _HASH2_H */
