#include "hash2.h" //<stdbool.h> bool, false, true | N, T, DEBUG, D
#include "jhash.h" //<stdint.h> uint8_t, uint32_t

#include <stdlib.h> //realloc(), exit(), atoi()
#include <unistd.h> //read(), STDIN_FILENO
#include <stdio.h> //size_t, FILE, NULL, printf(), getchar()
#include <string.h> //strchr(), strlen(), memcpy(), memcmp(), memset()


void dump(uint8_t* b, uint8_t l){

	//pointer: print address, init for iteration
	printf("%p : ", (void*) b); uint8_t* p = b;

	//print bytes
	for (size_t i = 0; i < l; i++) {
		printf("%2.2x ", *p);
		++p;
	}
  printf("\t");

	//reinit pointer for iteration
	p = b;

	//print chars
	for (size_t i = 0; i < l; i++) {
		printf("%c", *p ? *p : ' ');
		++p;
	}
  printf("\n");
}

void init(){
	//init table entries
  for (size_t i = 0; i < T; i++) {
    table[i] = NULL;
  }
}

void print(){
	//init ellipsis print
	bool e = true;

	//for each table entry
  for (size_t i = 0; i < T; i++) {

		//get table entry
		word* w = table[i];

		//table entry exists
		if (w){
			//reinit ellipsis print
			e = true;
			//init tabulated print
			bool t = false;

			//debug mode: print table entry address
			if (R[osize - 1]) printf("table[%lu] = %p:", i, &table[i]);

			//word exists
    	while (w){
				//debug mode
				if (R[osize - 1]){
					//tabulate @table entry address
					if (t) printf("\t\t\t  ");
					//print word (type, size)
					printf("%5s %u ", w->type ? "true" : "false", w->size);

				//tabulate other members @table entry
				} else if (t){
					if(R[osize - 2]) { printf("\n\t");} else { printf("\n");}
				}

				//print (word, count)
      	printf("%s: %u", w->name, w->count);

				//debug mode: print (prev, this, next) word address
				if (R[osize - 1]){
					printf(" [prev = %p w = %p next = %p]\n", w->prev, w, w->next);
				}
				//(end, start) of (table address, word member) tabulation
				t = true;

				//try next word
				w = w->next;
			}//while

			printf("\n");
		//print ellipsis for each table gap
		} else if (e){ e = false;}// printf("\t...\n");}
  }
}

bool eq(uint8_t* dest, uint8_t* sour, uint32_t size){
  return !memcmp((const void*) dest, (const void*) sour, size);
}

void escape(uint8_t** name, uint32_t size){

	//(name, size) exists
  if (name && *name && **name && size){

		//alloc byte buffer
		buffer* b = (buffer*) realloc(NULL, bsize + size);

		if (b){
			//get name length
			uint32_t l = strlen((const char*) *name);
			//init buffer (position, size)
			b->pos = 0; b->size = size;
			//init escape byte
			uint8_t* e;

			//for each byte in name
			for (size_t i = 0; i < l; i++) {
				//check byte for escape sequence
				e = (uint8_t*) memchr(D, (*name)[i], dsize);

				//escape sequence found
				if (e){
					//insert (backslash + escape symbol) to buffer
					b->buf[b->pos++] = *D; b->buf[b->pos++] = S[e - D];
				//standard symbol in byte
				} else { b->buf[b->pos++] = (*name)[i];}
			}
			//redefine name
			if (b->pos == size){
				//set word terminator (necessary for memcpy)
				b->buf[b->pos++] = '\0';

				//realloc name of buffer size
				//	*name scrambled after dealloc if (*name) = b->buf
				*name = (uint8_t*) realloc(NULL, b->pos);

				//copy buffer to name
				if (!memcpy(*name, b->buf, b->pos)){
					printf("escape: memcpy failed.\n"); exit(EXIT_FAILURE);
				}
				//dealloc byte buffer
				if (realloc(b, 0)){
					printf("escape: dealloc failed.\n"); exit(EXIT_FAILURE);
				}
			}
			//size parameter does not match name size
			else {
				printf("get: size %u out of \'%s\' bounds.\n", size, b->buf);
				exit(EXIT_FAILURE);
			}
		//b
		} else { printf("escape: alloc failed.\n"); exit(EXIT_FAILURE);}
	}
}

word* get(uint8_t* name, uint32_t size){

	//(name, size) exists
  if (name && *name && size){

		//replace found escape sequences with symbols
		escape(&name, size);

		//get name hash
    uint32_t i = jhash((const void*)name, size, 0) % T;
		//get table entry
		word* w = table[i];

		//table entry exists
		if(w){
			//name maybe in table entry
			while (w){
				//exact word found
				if (eq(w->name, name, size)){
					//dealloc replaced name
					if (realloc(name, 0)){
						printf("get: dealloc failed.\n"); exit(EXIT_FAILURE);
					}
					//return word
					return w;
				}
				//try next word
				w = w->next;
			}//while
		}
		//dealloc replaced name
		if(realloc(name, 0)){
			printf("get: dealloc failed.\n"); exit(EXIT_FAILURE);
		}
		return NULL;
  }
  return NULL;
}

word* unbind(word* w, uint32_t i){

	//previous words found
	if (w->prev){
		//inner word : chain adjacent words
		if (w->next){ w->prev->next = w->next; w->next->prev = w->prev;}
		//last word  : remove previous word reference to last word
		else {w->prev->next = NULL;}
	//no previous words
	} else {
		//first word : remove first word reference from next word
		if (w->next){ table[i] = w->next; w->next->prev = NULL;}
		//only word  : remove table entry
		else { w = table[i]; table[i] = NULL;}
	}
	//return unbound word
	return w;
}

word* del(uint8_t *name, uint32_t size){
	//(name, size) exists
  if (name && *name && size){

		//replace found escape sequences with symbols
		escape(&name, size);

		//get name hash
		uint32_t i = jhash((const void*)name, size, 0) % T;
		//get table entry
		word* w = table[i];

		//table entry exists
		if(w){

			//name maybe in table entry
			while (w){

				//exact word found
				if (eq(w->name, name, size)){

					//dealloc replaced name
					if (realloc(name, 0)){
						printf("get: dealloc failed.\n"); exit(EXIT_FAILURE);
					}

					//return word from table entry
					return unbind(w, i);
				}
				//try next word
				w = w->next;
			}//while
		}
		//dealloc replaced name
		if(realloc(name, 0)){
			printf("get: dealloc failed.\n"); exit(EXIT_FAILURE);
		}
		//no table entry
		return NULL;
	}
	return NULL;
}

void clean(){

	//clean table
	for (size_t i = 0; i < T; i++) {

		//get table entry
		word* w = table[i];

		//table entry exists
		if (w){
			//for each table entry word
			while (w){
				//init unbound word
				word* u = NULL;

				//unbind (delimiter, word, all) from table entry
				if (w->type == dtype || dtype == -1){ u = unbind(w, i);}

				//try next word (necessary before dealloc)
				w = w->next;

				//dealloc unbound word
				if(u && realloc(u, 0)){
					printf("clean: dealloc failed.\n"); exit(EXIT_FAILURE);
				}
			}//while
		}
	}//for
}

buffer* extend(buffer* d, uint8_t* s, uint32_t i){

	//buffer (dest, source, increment) exists
	if (d && s && i){

		//dest end position
		if (d->pos == d->size){
			//extend dest buffer size
			d->size += i; d = (buffer*) realloc(d, bsize + d->size);

		//position past dest bound
		} else if (d->pos > d->size){
			printf("byte position %u past buffer bound %u.\n", d->pos, d->size);
			dump(d->buf, d->pos); exit(EXIT_FAILURE);
		}
		//extend dest buffer
		for (size_t j = 0; j < strlen((const char*)s); j++) {
			d->buf[d->pos++] = *s;
		}
	}

	//error if no return?
	return d;
}

void push(word** d, buffer* b){

	//define word variable member size
	uint8_t vs = (b->pos + 1) * sizeof(*b->buf);
	//alloc word (static + variable) members
	word* w = (word*) realloc(NULL, wsize + vs);

	//alloc success
	if(w){
		//copy buffer to word
		if (!memcpy((void*) w->name, (const void*) b->buf, vs)){
			exit(EXIT_FAILURE);
		}
		//set word terminator (necessary for memcpy)
		w->name[b->pos] = '\0';

		//check first buffer byte for (additional, other) delimiters
		//	only first byte, because only two types
		uint8_t* dt = (uint8_t*) memchr((const char*)D, *b->buf, dsize);
		uint8_t* at = (uint8_t*) memchr((const char*)A, *b->buf, asize);
		uint8_t* ot = (uint8_t*) memchr((const char*)od->buf, *b->buf, od->size-1);

		//init (additional, other) delimiter request
		bool arq = false, orq = false;

		//(additional, other) delimiter found
		if (at){ arq = AR[at - A];} else { arq = false;}
		if (ot){ orq =  R[10]    ;} else { orq = false;}

		//set word (type, size, count, next pointer)
		w->type = (dt || arq || orq) ? false : true;
		w->size = b->pos + 1;
		w->count++;
		w->next = NULL;

		//set to destination
		*d = w;

	} else {exit(EXIT_FAILURE);}
}

bool insert(buffer* b){
	//buffer not empty
  if (b && *b->buf && b->pos && b->size){

		//get buffer hash
		uint32_t i = jhash((const void*)b->buf, b->pos, 0) % T;
		//init (next, previous) word
		word* n = table[i]; word* p = n;

		//word maybe in table (collision)
		if (n){
			//search table entry
			while(n){
				//exact word found
				if (eq(n->name, b->buf, b->size)){n->count++; break;}

				//set (previous, next) word
				p = n; n = n->next;
			}//while

			//word not in table entry (collision)
			if(!n){ push(&n, b); n->prev = p   ; p->next  = n;}
		//new table entry
	} else {    push(&n, b); n->prev = NULL; table[i] = n;}

    return true;
  }
	return false;
}

void help(){
	printf("\n\
hash2 - count word repetitions from standard input and print \
results in the form [word: count] to standard output.\n\
\n\
Usage: [COMMAND] | [./]hash2 [OPTIONS]\n \
\n\
\t./ is necessary at hash2 $PWD (.) if hash2 binary not in $PATH.\n\
\tOPTIONS flag delimiter (-) is unnecessary. \
OPTIONS can be packed with no separation and positions exchanged.\n\
\n\
Info:\n\
\thash2 uses Jenkins algorithm to calculate hash values for table indices.\n\
\tIf table index collision occurs, \
hash2 chains words by using doubly linked lists.\n\
\tRefer to -S option below to avoid word collisions. \
Refer to -t option below to tabulate word collisions.\n\
\tWord buffer is dynamically allocated, so any size input \
stream is possible up to system limits.\n\
\tStarting and ending delimiters are ignored.\n\
\n\
Options:\n\
\t-A           count all identifiers (delimiters and words)\n\
\t-d           count only delimiters\n\
\t-w (default) count only words\n\
\t-B           treat all brackets as delimiters, same as -acrs\n\
\t-b (default) treat no brackets as delimiters\n\
\t-a           treat angle brackets <> as delimiters\n\
\t-c           treat curly brackets {} as delimiters\n\
\t-r           treat round brackets () as delimiters\n\
\t-s           treat square brackets [] as delimiters\n\
\t-o SYMBOLS   treat other SYMBOLS as delimiters\n\
\t-S SIZE      hash table SIZE (default is 100 entries)\n\
\t-t           tabulate collisions for each linked list head\n\
\t-h           display this help and exit\n\
\t-D           print in debug mode (w/ linked list addresses), same as -tD\n\
\n\
Examples:\n\
\t\"man gcc | ./hash2 S1000000t\"\n\
\t\tprint words from table of size 10^6 and tabulate collisions.\n\n\
\t\"echo '  ,\\nre<>{}()[]foo.bar\\n\\r. ' | ./hash2 Acso,.\"\n\
\t\tprint identifiers in debug mode and tabulate collisions.\n\
\t\tAdditional {}[],. symbols are treated as delimiters.\n\n\
\t\"man gcc | /hash2 wtS10000Bo',.;:-+*=|/_@&#' | grep -i gnu\"\n\
\t\tprint words from table of size 10^4 and tabulate collisions.\n\
\t\tAdditional <>{}()[],.;:-+*=|/_@&# symbols are treated as delimiters.\n\
\t\tPipe output to grep to search for lowercase/uppercase \'gnu\'.\n\
\n\
");
}

void request(uint8_t** pars, uint8_t size){

	//init (size, other delimiter buffer) buffer
	//	uint32_t 0xffffffff = 4294967295 is 10 symbols + '\0'
	uint32_t ss = bsize + 11;
	//	ss + options (w/ -) + all 7bit ASCIIs
	uint32_t ds = ss + 2*osize + (2 << 7);

	//init (size, other delimiter) buffer
	buffer* s = (buffer*) realloc(NULL, ss); s->size = ss;
	od = (buffer*) realloc(NULL, ds); od->size = ds;

	//init table size
	uint32_t tsize = T;
	//init (option, additional) byte
	uint8_t* o; uint8_t* a;

	//alloc success
	if (s && od){

		//for each flag (- not necessary)
		for (size_t i = 1; i < size; i++) {
			//for each flag byte
			for (size_t j = 0; j < strlen((const char*)pars[i]); j++) {
				//check byte for (options, additionals)
				o = (uint8_t*) memchr(O, pars[i][j], osize);
				a = (uint8_t*) memchr(A, pars[i][j], asize);

				//(option, additional) byte start
				if (o || a){
					//option byte
					if (o){
						//help option
						if (*o == *O){ help(); exit(EXIT_SUCCESS);}
						//other delimiter symbols option
						else if (*o == O[osize - 4]){ od->pos++;}
						//table size option
						else if (*o == O[osize - 3]){ s->pos++;}

						//set option in request array
						R[o - O] = true;
					//additional byte
					} else { AR[a - A] = true;}
				} else {
					//append table size symbol
					if (s->pos){
						//digit byte (for all option packing)
						if (0x30 <= pars[i][j] && pars[i][j] <= 0x39){
							s->buf[s->pos++ - 1] = pars[i][j];
						//other byte
						} else { s->pos = s->size;}
					}
					//append other delimiter
				  if (od->pos){ od->buf[od->pos++ - 1] = pars[i][j];}
				}
			}//for

			//end of table size buffer
			if (*s->buf && s->pos == s->size){
				//set size buffer terminator
				s->buf[s->pos] = '\0'; s->pos = 0;
				//set table size from size buffer
				tsize = atoi((const char*) s->buf);
			}
			//end of other delimiter buffer
			if (*od->buf && od->pos){
				//set size buffer terminator
				od->buf[od->pos] = '\0';

				//realloc other delimiter buffer
				od = (buffer*) realloc(od, bsize + od->pos);
				od->size = od->pos; od->pos = 0;
			}
		}//for
	//alloc failed
	} else { printf("request: alloc failed."); exit(EXIT_FAILURE);}

	//dealloc size buffer
	if(realloc(s, 0)){
		printf("request: dealloc failed."); exit(EXIT_FAILURE);
	}

	//count words + delimiters
	if (R[1]){ R[2] = R[3] = true; dtype = -1;}
	else {
		//count delimiters
		if (R[2]){
			//delete words
			dtype = 1;

			//count words + delimiters
			if (R[3]){ R[1] = true; dtype = -1;}

		//count words, delete delimiters (default)
		} else { R[3] = true; dtype = 0;}
	}

	//all brackets as delimiters
	if (R[4] || (R[6] && R[7] && R[8] && R[9])){
		R[5] = false; R[4] = R[6] = R[7] = R[8] = R[9] = true;
		memset(AR, true, asize);
	//no brackets as delimiters (default)
	} else if (R[5]){
		R[4] = R[6] = R[7] = R[8] = R[9] = false;
		memset(AR, false, asize);
	//some brackets as delimiters
	} else {
		if(R[6]) AR[0] = AR[1] = true; //angle
		if(R[7]) AR[2] = AR[3] = true; //curly
		if(R[8]) AR[4] = AR[5] = true; //round
		if(R[9]) AR[6] = AR[7] = true; //square
	}

	//reset table size
	if (R[osize - 3]) T = tsize;
	//tabulated print in debug mode (default)
	if (R[osize - 1]) R[osize - 2] = true;
}

int main(int count, char** pars){

	//get user request
	request((uint8_t**) pars, count);

	//alloc table
	table = (word**) realloc(NULL, T * wsize);
	//init table
  init();

	//init (word, stream, delimiter) variable member size
	uint8_t in = 5, s = sizeof(uint8_t);
	//alloc (word, stream, delimiter) buffer
	buffer* wb = (buffer*) realloc(NULL, bsize + in * s);
	buffer* sb = (buffer*) realloc(NULL, bsize + in * s);
	buffer* db = (buffer*) realloc(NULL, bsize + s);

	//init (delimiter, aditional, other) request
	uint8_t* d; uint8_t* a; uint8_t* o;
	bool drq, arq, orq;

	//alloc success
	if (wb && sb && db){
		//init (word, stream, delimiter) position
		wb->pos = sb->pos = db->pos = 0;
		//init (word, stream, delimiter) buffer size
		wb->size = sb->size = in * s; db->size = s;
		//define (stream, escape, null) byte
		uint8_t c, e = '\0', n = '\0';

		//read stream byte
		while(read(STDIN_FILENO, &c, s)){

			//check byte for (additional, other) delimiters
			d = (uint8_t*) memchr(D, c, dsize);
			a = (uint8_t*) memchr(A, c, asize);
			o = (uint8_t*) memchr(od->buf, c, od->size - 1);

			//delimiter found
			if (d || e){
				//(start, continue) escape sequence
				if (c == *D) { e = *D;}
				//end escaoe sequence
				else if (e) { e = '\0';}

				drq = true;
			} else {drq = false;}
			//(additional, other) delimiter found
			if (a){ arq = AR[a - A];} else { arq = false;}
			if (o){ orq = R[10];} else { orq = false;}

			//printf("c=\'%c\' %u %u %u\n", c, drq, arq, orq);

			//delimiter (not \) | additional (bracket) | other
			if (drq || arq || orq){//was: c != *D

				//word buffer filled
				if (*wb->buf && wb->pos && wb->size){

					//transfer delimiter
					if (db->pos){
						//delimit stream buffer words
						//sb = extend(sb, db->buf, db->size);

						//insert delimiter to table
						insert(db);
						//delimiter buffer: clear, reinit position
						memset(db->buf, 0, db->pos); db->pos = 0;
						//delimiter buffer: reinit size, realloc
						db->size = s; db = (buffer*) realloc(db, bsize + db->size);
					}
					//delimiter start
					db->buf[db->pos++] = c;

					if (wb->buf == (uint8_t*)"elif"){printf("%s\n", "yoyo"); exit(0);}
					//insert word to table
					insert(wb);
					//word buffer: clear, reinit position
					memset(wb->buf, 0, wb->pos); wb->pos = 0;
					//word buffer: reinit size, realloc
					wb->size = in * s; wb = (buffer*) realloc(wb, bsize + wb->size);
				//delimiter end
				} else {
					//ignore pre-first-word delimiter
					if (db->pos) db = extend(db, &c, s);
					//while delimiter stream
					continue;
				}
		//count words
		} else {
				//extend (word, stream) buffer
				wb = extend(wb, &c, in * s);
				//sb = extend(sb, &c, in * s);
			}
		}//while

		//count last word (always empty?)
		if (*wb->buf){
			//insert word to table
			insert(wb);
		}
		//set buffer terminator
		sb = extend(sb, &n, s);

		//dealloc (delimiter, word, stream, other delimiter) buffer
		buffer* d[] = {db, wb, sb, od};
		for (size_t i = 0; i < 4; i++) {
			//dealloc failed
			if(realloc(d[i], 0)){
				printf("buffer %lu dealloc failed.", i);
				exit(EXIT_FAILURE);
			}
		}//for
	} else {printf("main: alloc failed.\n"); exit(EXIT_FAILURE);}

	//print request to stdout
	if (dtype != -1) clean();
	print();

	//dealloc all table
	dtype = -1; clean();

  //getchar();
  return 0;
}
