#include "TestUtils.h"
#include "MeasuredString.h"

#pragma warning(disable : 4310) // cast truncates constant value (for 'weirdUnicodeChars' and 'brokenUnicodeChars')
#pragma warning(disable : 4127) // conditional expression is constant

static char weirdUnicodeChars_data[12] = {
                              'w', 'h', (char)0xC2, (char)0xBB, 't', 'e', 'v',
                              (char)0xE0, (char)0xA0, (char)0xA0, 'r', (char)0
                             };

static char brokenUnicodeChars_data[11] = {
                                'w', 'h', (char)0x80, 't', 'e', 'v', (char)0x80,
                                (char)0x80, (char)0x80, 'r', (char)0
                              };

void Test_MeasuredString()
{
  MeasuredString m;
  Utf32Char fakeData[20];
  char weirdUnicodeChars[12];
  char brokenUnicodeChars[11];

  memcpy(weirdUnicodeChars, weirdUnicodeChars_data, sizeof(weirdUnicodeChars));
  memcpy(brokenUnicodeChars, brokenUnicodeChars_data, sizeof(brokenUnicodeChars));

  // Verify that the ascii allocator works
  m.Data = &fakeData[19];
  m.Length = 1;
  MeasuredString_AllocFromAscii(&m, "whatever");
  CHECK(m.Length == 8 &&
    m.Data != NULL &&
    m.Data[0] == 'w' &&
    m.Data[1] == 'h' &&
    m.Data[2] == 'a' &&
    m.Data[3] == 't' &&
    m.Data[4] == 'e' &&
    m.Data[5] == 'v' &&
    m.Data[6] == 'e' &&
    m.Data[7] == 'r', "whatever");

  MeasuredString_Free(&m);
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );

  // Verify that the UTF-8 allocator works
  m.Data = &fakeData[19];
  m.Length = 1;
  CHECK(sizeof(weirdUnicodeChars[0]) == 1, );
  MeasuredString_AllocFromUtf8(&m, weirdUnicodeChars, strlen(weirdUnicodeChars));
  CHECK(m.Length == 8 &&
    m.Data != NULL &&
    m.Data[0] == 'w' &&
    m.Data[1] == 'h' &&
    m.Data[2] == 187 &&
    m.Data[3] == 't' &&
    m.Data[4] == 'e' &&
    m.Data[5] == 'v' &&
    m.Data[6] == 2080 &&
    m.Data[7] == 'r', "whatever utf8");

  MeasuredString_Free(&m);
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );

  // Verify that the UTF-8 allocator works with broken characters
  m.Data = &fakeData[19];
  m.Length = 1;
  CHECK(sizeof(brokenUnicodeChars[0]) == 1, );
  MeasuredString_AllocFromUtf8(&m, brokenUnicodeChars, strlen(brokenUnicodeChars));
  CHECK(m.Length == 8 &&
    m.Data != NULL &&
    m.Data[0] == 'w' &&
    m.Data[1] == 'h' &&
    m.Data[2] == '?' &&
    m.Data[3] == 't' &&
    m.Data[4] == 'e' &&
    m.Data[5] == 'v' &&
    m.Data[6] == '?' &&
    m.Data[7] == 'r', "whatever broken utf8");

  MeasuredString_Free(&m);
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );

  // Verify that the length allocator works
  m.Data = &fakeData[19];
  m.Length = 1;
  MeasuredString_AllocFromLength(&m, 3);
  CHECK(m.Length == 3 &&
    m.Data != NULL &&
    m.Data[0] == 0 &&
    m.Data[1] == 0 &&
    m.Data[2] == 0, "length");

  MeasuredString_Free(&m);
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );

  // Verify that each allocator allocates nothing if length == 0
  MeasuredString_AllocFromAscii(&m, "");
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );

  MeasuredString_AllocFromUtf8(&m, "steve", 0);
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );

  MeasuredString_AllocFromLength(&m, 0);
  CHECK(m.Length == 0, );
  CHECK(m.Data == NULL, );
}