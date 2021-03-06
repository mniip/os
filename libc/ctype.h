#ifndef LIBC_CTYPE_H_
#define LIBC_CTYPE_H_

#define isalpha(c) (ctable[(unsigned char)c] & 0x0001)
#define iscntrl(c) (ctable[(unsigned char)c] & 0x0002)
#define isdigit(c) (ctable[(unsigned char)c] & 0x0004)
#define isgraph(c) (ctable[(unsigned char)c] & 0x0008)
#define islower(c) (ctable[(unsigned char)c] & 0x0010)
#define ispunct(c) (ctable[(unsigned char)c] & 0x0020)
#define isspace(c) (ctable[(unsigned char)c] & 0x0040)
#define isupper(c) (ctable[(unsigned char)c] & 0x0080)
#define isalnum(c) (ctable[(unsigned char)c] & 0x0100)
#define isxdigit(c) (ctable[(unsigned char)c] & 0x0200)

static inline int toupper(int c) { return c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c; };
static inline int tolower(int c) { return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c; };

static int ctable[256] = {
	0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	0x0002, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0002, 0x0002,
	0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	0x0040, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028,
	0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028,
	0x030c, 0x030c, 0x030c, 0x030c, 0x030c, 0x030c, 0x030c, 0x030c,
	0x030c, 0x030c, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028,
	0x0028, 0x0389, 0x0389, 0x0389, 0x0389, 0x0389, 0x0389, 0x0189,
	0x0189, 0x0189, 0x0189, 0x0189, 0x0189, 0x0189, 0x0189, 0x0189,
	0x0189, 0x0189, 0x0189, 0x0189, 0x0189, 0x0189, 0x0189, 0x0189,
	0x0189, 0x0189, 0x0189, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028,
	0x0028, 0x0319, 0x0319, 0x0319, 0x0319, 0x0319, 0x0319, 0x0119,
	0x0119, 0x0119, 0x0119, 0x0119, 0x0119, 0x0119, 0x0119, 0x0119,
	0x0119, 0x0119, 0x0119, 0x0119, 0x0119, 0x0119, 0x0119, 0x0119,
	0x0119, 0x0119, 0x0119, 0x0028, 0x0028, 0x0028, 0x0028, 0x0002,
};

#endif
