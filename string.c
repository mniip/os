#include <stdint.h>

uint32_t strlen(char const *s)
{
	char const *p = s;
	while(*s)
		s++;
	return s - p;
}

char *strcpy(char *dest, char const *src)
{
	uint32_t i;
	for(i = 0; src[i]; i++)
		dest[i] = src[i];
	dest[i] = 0;
	return dest;
}

int strcmp(char const *s1, char const *s2)
{
	while(*s1 && *s2 && *s1 == *s2)
	{
		s1++;
		s2++;
	}
	if(*s1 > *s2)
		return 1;
	if(*s1 < *s2)
		return -1;
	return 0;
}

int strncmp(char const *s1, char const *s2, int n)
{
	while(n && *s1 && *s2 && *s1 == *s2)
	{
		s1++;
		s2++;
		n--;
	}
	if(!n)
		return 0;
	if(*s1 > *s2)
		return 1;
	if(*s1 < *s2)
		return -1;
	return 0;
}

int strcoll(char const *s1, char const *s2)
{
	return strcmp(s1, s2);
}

char const *strchr(char const *s, int c)
{
	while(*s)
	{
		if(*s == c)
			return s;
		s++;
	}
	return 0;
}

char const *strrchr(char const *s, int c)
{
	char const *r = 0;
	while(*s)
	{
		if(*s == c)
			r = s;
		s++;
	}
	return r;
}

char const *strpbrk(char const *s, char const *accept)
{
	while(*s)
	{
		if(strchr(accept, *s))
			return s;
		s++;
	}
	return 0;
}

uint32_t strspn(char const *s, char const *accept)
{
	uint32_t r = 0;
	while(strchr(accept, s[r]))
		r++;
	return r;
}

char const *strerror(int errno)
{
	return "Error";
}

char const *strstr(char const *haystack, char const *needle)
{
	while(*haystack)
	{
		uint32_t i;
		for(i = 0; needle[i]; i++)
			if(haystack[i] != needle[i])
				break;
		if(!needle[i])
			return haystack;
		haystack++;
	}
	return 0;
}

void *memcpy(void *dest, void const *src, uint32_t sz)
{
	uint32_t i;
	for(i = 0; i < sz; i++)
		((char *)dest)[i] = ((char const *)src)[i];
	return dest;
}

int memcmp(void const *s1, void const *s2, uint32_t n)
{
	while(n && *(char const *)s1 == *(char const *)s2)
	{
		s1++;
		s2++;
		n--;
	}
	if(!n)
		return 0;
	if(*(char const *)s1 > *(char const *)s2)
		return 1;
	if(*(char const *)s1 < *(char const *)s2)
		return -1;
	return 0;
}

void *memchr(void const *s, int c, uint32_t n)
{
	while(n)
	{
		if(*(char const *)s == (char)c)
			return s;
		s++;
		n--;
	}
	return 0;
}


double strtod(char const *str, char const **end)
{
	*end = str;
	int seendot = 0;
	double res = 0.0;
	double mul = 1.0;
	if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
	}
	else
	{
		while(1)
		{
			if(*str >= '0' && *str <= '9')
			{
				if(seendot)
				{
					mul /= 10;
					res = res + (*str - '0') * mul;
				}
				else
					res = res * 10 + (*str - '0');
			}
			else if(*str == '.')
			{
				if(seendot)
					break;
				else
					seendot = 1;
			}
			else
				break;
			str++;
		}
	}
	*end = str;
	return res;
}
