//< text processing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../___.h"

#include "str.h"
#include "txt.h"
#include "chr.h"

#include "../glb.h"

UJ txt_get_word(FILE* f, S dir, S source, I max_d, I max_s, I ptr)
{
	LOG("txt_get_word");
	UJ j, i;

	for (j = 0; valid_key(dir[j]) && j < max_d; j++);

	// X(j == max_d, T(FATAL, "WORD_BUF will be overflowed (max. capacity %d)", max_d), NIL);

	// if (j >= max_d -1)
		// O("IM HERE!\n");

	X(j == max_d, T(WARN, "WORD_BUF is overflowed (max. capcity %d); file ptr at %d", max_d, ftell(f)), NIL);

	for (i = 0; valid_key(source[ptr + i]) && i < max_s && j + i < max_d; i++) 
		dir[j + i] = source[ptr + i];
	dir[j + i] = 0;

	R (i == max_s || j + i == max_d) ? NIL : i;
} 

//<	process buf and break it into words 
//<	use fn for each word found
UJ txt_process_buf(FILE* f, S buf, V* tri, V* hsh, I len, WORD_ADD fn, I param)
{
	LOG("txt_process_buf");
	I i = 0, var;
	if (valid_key(WORD_BUF[0])) {
		if (valid_key(buf[0])) {
			var = txt_get_word(f, WORD_BUF, buf, SZ_WBUF, len, i);		
			P (var == NIL, NIL);									//< WORD_BUF or buf is overflowed
			i += var;
		}
		goto INCLUDE;
	}

	W(i < len - 1) {
		var = swipe_buf(buf, i, len, 0);							//< how many chars swiped or NIL
		P(var == NIL, 0);											//< end of buf
		i += var;													//< move pointer

		var = txt_get_word(f, WORD_BUF, buf, SZ_WBUF, len, i);		//< length of word or NIL
		if (var == NIL && valid_key(WORD_BUF[SZ_WBUF - 1])) {
			T(WARN, "WORD_BUF is overflowed (max. capcity %d); file ptr at %d", SZ_WBUF, ftell(f) - SZ_WBUF);
		}
			// var = swipe_buf(buf, i, len, 1);
			// i += var;
			// clean_buf(WORD_BUF, SZ_WBUF);
		// }


		// X(var == NIL && valid_key(WORD_BUF[SZ_WBUF - 1]), T(WARN, "WORD_BUF is overflowed (max. capcity %d); file ptr at %d", SZ_WBUF, ftell(f) + (len - i)), NIL);


		P(var == NIL, NIL);											//< problems with buf's capacity
		i += var;

		P(i >= len - 1 && !param, 0); 								//< it could be an incompleted word 

		INCLUDE:
		P(fn(tri, hsh, WORD_BUF, sz_buf(WORD_BUF, SZ_WBUF)) == NIL, NIL);
		clean_buf(WORD_BUF, SZ_WBUF);
	}

	fflush(stdout);
	R0;
}

//< FILE* f into TRIE and HSH
//< fn: str_hsh_ins or str_tri_ins
//< parsing main loop
UJ txt_process(FILE* f, V* struct_1, V* struct_2, WORD_ADD fn)
{
	LOG("txt_process");
	UJ len, res;

	LOOP:
	len = fread(TEXT_BUF, SZ(C), SZ_TBUF - 1, f);
	TEXT_BUF[len] = 0;
	P(txt_process_buf(f, TEXT_BUF, struct_1, struct_2, len + 1, fn, feof(f)) == NIL, NIL);
	if (!feof(f)) goto LOOP;

	clean_buf(WORD_BUF, SZ_WBUF);
	clean_buf(TEXT_BUF, SZ_TBUF);
	R 0;
}
