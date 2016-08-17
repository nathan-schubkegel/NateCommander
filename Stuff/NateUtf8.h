#ifndef NATE_UTF8_H
#define NATE_UTF8_H

#include <SDL.h>

#define REPLACEMENT_CHARACTER 0xFFFD

/*
 * Type for Unicode codepoints.
 * We need our own because wchar_t might be 16 bits.
 */
typedef Uint32 Unicode_Codepoint_t;

/*
 * Validate the given UTF-8 string.
 * If it contains '\0' characters, it is still valid.
 */
int utf8_validate(const char *str, size_t length);

/*
 * Validate a single UTF-8 character.
 * @s: Beginning of UTF-8 character.
 * @e: End of string.
 *
 * If it's valid, return its length (1 thru 4).
 * If it's invalid or clipped, return 0.
 */
int utf8_validate_char(const char *s, const char *e);

/*
 * Read a single UTF-8 character starting at @s,
 * returning the length, in bytes, of the character read.
 *
 * This function assumes input is valid UTF-8,
 * and that there are enough characters in front of @s.
 */
int utf8_read_char(const char *s, Unicode_Codepoint_t *out);

/*
 * Write a single UTF-8 character to @s,
 * returning the length, in bytes, of the character written.
 *
 * @unicode should be U+0000..U+10FFFF, but not U+D800..U+DFFF.
 * If @unicode is invalid, REPLACEMENT_CHARACTER will be emitted instead.
 *
 * This function will write up to 4 bytes to @out.
 */
int utf8_write_char(Unicode_Codepoint_t unicode, char *out);

/*
 * Compute the Unicode codepoint of a UTF-16 surrogate pair.
 *
 * @uc should be 0xD800..0xDBFF, and @lc should be 0xDC00..0xDFFF.
 * If they aren't, this function returns REPLACEMENT_CHARACTER.
 */
Unicode_Codepoint_t from_surrogate_pair(unsigned int uc, unsigned int lc);

/*
 * Construct a UTF-16 surrogate pair given a Unicode codepoint.
 *
 * @unicode should be U+10000..U+10FFFF.
 * If it's not, this function returns false,
 * and sets *uc and *lc to REPLACEMENT_CHARACTER.
 */
int to_surrogate_pair(Unicode_Codepoint_t unicode, unsigned int *uc, unsigned int *lc);

#endif
