#include "NateUtf8.h"
#include <assert.h>

int utf8_validate(const char *str, size_t length)
{
	int len;
	const char *s = str;
	const char *e = str + length;
	
	for (; s < e; s += len)
  {
		len = utf8_validate_char(s, e);
		if (len == 0)
    {
			return 0; // false
    }
	}
	assert(s == e);
	
	return 1; // true
}

/*
 * This function implements the syntax given in RFC3629, which is
 * the same as that given in The Unicode Standard, Version 6.0.
 *
 * It has the following properties:
 *
 *  * All codepoints U+0000..U+10FFFF may be encoded,
 *    except for U+D800..U+DFFF, which are reserved
 *    for UTF-16 surrogate pair encoding.
 *  * UTF-8 byte sequences longer than 4 bytes are not permitted,
 *    as they exceed the range of Unicode.
 *  * The sixty-six Unicode "non-characters" are permitted
 *    (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF).
 */
int utf8_validate_char(const char *s, const char *e)
{
	unsigned char c = *s++;
	
	if (c <= 0x7F) {        /* 00..7F */
		return 1;
	} else if (c <= 0xC1) { /* 80..C1 */
		/* Disallow overlong 2-byte sequence. */
		return 0;
	} else if (c <= 0xDF) { /* C2..DF */
		/* Make sure the character isn't clipped. */
		if (e - s < 1)
			return 0;
		
		/* Make sure subsequent byte is in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 2;
	} else if (c <= 0xEF) { /* E0..EF */
		/* Make sure the character isn't clipped. */
		if (e - s < 2)
			return 0;
		
		/* Disallow overlong 3-byte sequence. */
		if (c == 0xE0 && (unsigned char)*s < 0xA0)
			return 0;
		
		/* Disallow U+D800..U+DFFF. */
		if (c == 0xED && (unsigned char)*s > 0x9F)
			return 0;
		
		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 3;
	} else if (c <= 0xF4) { /* F0..F4 */
		/* Make sure the character isn't clipped. */
		if (e - s < 3)
			return 0;
		
		/* Disallow overlong 4-byte sequence. */
		if (c == 0xF0 && (unsigned char)*s < 0x90)
			return 0;
		
		/* Disallow codepoints beyond U+10FFFF. */
		if (c == 0xF4 && (unsigned char)*s > 0x8F)
			return 0;
		
		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 4;
	} else {                /* F5..FF */
		return 0;
	}
}

int utf8_read_char(const char *s, Unicode_Codepoint_t *out)
{
	const unsigned char *c = (const unsigned char*) s;

	if (c[0] <= 0x7F) {
		/* 00..7F */
		*out = c[0];
		return 1;
	} else if (c[0] <= 0xDF) {
		/* C2..DF (unless input is invalid) */
		*out = ((Unicode_Codepoint_t)c[0] & 0x1F) << 6 |
		       ((Unicode_Codepoint_t)c[1] & 0x3F);
		return 2;
	} else if (c[0] <= 0xEF) {
		/* E0..EF */
		*out = ((Unicode_Codepoint_t)c[0] &  0xF) << 12 |
		       ((Unicode_Codepoint_t)c[1] & 0x3F) << 6  |
		       ((Unicode_Codepoint_t)c[2] & 0x3F);
		return 3;
	} else {
		/* F0..F4 (unless input is invalid) */
		*out = ((Unicode_Codepoint_t)c[0] &  0x7) << 18 |
		       ((Unicode_Codepoint_t)c[1] & 0x3F) << 12 |
		       ((Unicode_Codepoint_t)c[2] & 0x3F) << 6  |
		       ((Unicode_Codepoint_t)c[3] & 0x3F);
		return 4;
	}
}

int utf8_write_char(Unicode_Codepoint_t unicode, char *out)
{
	unsigned char *o = (unsigned char*) out;

	if (unicode <= 0x7F) {
		/* U+0000..U+007F */
		*o++ = (char)unicode;
		return 1;
	} else if (unicode <= 0x7FF) {
		/* U+0080..U+07FF */
		*o++ = (char)(0xC0 | (unicode >> 6));
		*o++ = (char)(0x80 | (unicode & 0x3F));
		return 2;
	} else if (unicode <= 0xFFFF) {
		/* U+0800..U+FFFF */
		if (unicode >= 0xD800 && unicode <= 0xDFFF)
			unicode = REPLACEMENT_CHARACTER;
	three_byte_character:
		*o++ = (char)(0xE0 | unicode >> 12);
		*o++ = (char)(0x80 | (unicode >> 6 & 0x3F));
		*o++ = (char)(0x80 | (unicode & 0x3F));
		return 3;
	} else if (unicode <= 0x10FFFF) {
		/* U+10000..U+10FFFF */
		*o++ = (char)(0xF0 | unicode >> 18);
		*o++ = (char)(0x80 | (unicode >> 12 & 0x3F));
		*o++ = (char)(0x80 | (unicode >> 6 & 0x3F));
		*o++ = (char)(0x80 | (unicode & 0x3F));
		return 4;
	} else {
		/* U+110000... */
		unicode = REPLACEMENT_CHARACTER;
		goto three_byte_character;
	}
}

Unicode_Codepoint_t from_surrogate_pair(unsigned int uc, unsigned int lc)
{
	if (uc >= 0xD800 && uc <= 0xDBFF && lc >= 0xDC00 && lc <= 0xDFFF)
		return 0x10000 + ((((Unicode_Codepoint_t)uc & 0x3FF) << 10) | (lc & 0x3FF));
	else
		return REPLACEMENT_CHARACTER;
}

int to_surrogate_pair(Unicode_Codepoint_t unicode, unsigned int *uc, unsigned int *lc)
{
	if (unicode >= 0x10000 && unicode <= 0x10FFFF) {
		Unicode_Codepoint_t n = unicode - 0x10000;
		*uc = ((n >> 10) & 0x3FF) | 0xD800;
		*lc = (n & 0x3FF) | 0xDC00;
		return 1; // true
	} else {
		*uc = *lc = REPLACEMENT_CHARACTER;
		return 0; // false
	}
}
