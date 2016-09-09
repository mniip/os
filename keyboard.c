#include <stdint.h>
#include "keyboard.h"
#include "asm.h"
#include "vga_io.h"

enum
{
	IDLE,
	RECEIVED_E0,
	RECEIVED_F0,
	RECEIVED_E0F0,
	RECEIVED_E1
}
port_state = IDLE;

enum
{
	DOWN,
	E0DOWN,
	F0DOWN,
	E0F0DOWN,
	E1DOWN,
	NUM_STATES
};

uint8_t key_state[0x80] = {0};

static uint8_t last_byte = 0xFE;

static int poll_byte()
{
	uint8_t data = inb(0x60);
	if(last_byte != data)
	{
		last_byte = data;
		return 1;
	}
	return 0;
}

int poll_event(uint32_t *event)
{
	while(poll_byte())
	{
		if(last_byte == 0x00 || last_byte == 0xEE || last_byte >= 0xFA)
			port_state = IDLE;
		else if(last_byte == 0xE0)
			port_state = RECEIVED_E0;
		else if(last_byte == 0xF0)
			port_state = port_state == RECEIVED_E0 ? RECEIVED_E0F0 : RECEIVED_F0;
		else if(last_byte == 0xE1)
			port_state = RECEIVED_E1;
		else
		{
			uint32_t modifier = 0;
			if(port_state == IDLE)
				modifier = DOWN;
			else if(port_state == RECEIVED_E0)
				modifier = E0DOWN;
			else if(port_state == RECEIVED_F0)
				modifier = F0DOWN;
			else if(port_state == RECEIVED_E0F0)
				modifier = E0F0DOWN;
			else if(port_state == RECEIVED_E1)
				modifier = E1DOWN;
			if(last_byte & 0x80)
				key_state[last_byte & 0x7F] &= ~(1 << modifier);
			else
				key_state[last_byte & 0x7F] |= 1 << modifier;
			port_state = IDLE;
			*event = modifier << 8 | last_byte;
			outb(0x60, 0xEE);
			return 1;
		}
	}
	return 0;
}

enum
{
	LSHIFT = 0x2A,
	RSHIFT = 0x36,
};

char scancodes[0x80] =
{
	  0,  0,'1','2','3','4','5','6','7','8','9','0','-','=','\b',  0,
	'q','w','e','r','t','y','u','i','o','p','[',']','\n',  0,'a','s',
	'd','f','g','h','j','k','l',';','\'',  0,  0,'|','z','x','c','v',
	'b','n','m',',','.','/',  0,  0,  0, ' ',  0,  0,  0,  0,  0,  0,
};

char shift_scancodes[0x80] =
{
	  0,  0,'!','@','#','$','%','^','&','*','(',')','_','+','\b',  0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',  0,'A','S',
	'D','F','G','H','J','K','L',':','"',  0,  0,'\\','Z','X','C','V',
	'B','N','M','<','>','?',  0,  0,  0, ' ',  0,  0,  0,  0,  0,  0,
};

int poll_char(char *c)
{
	uint32_t event;
	while(poll_event(&event))
		if(!EVENT_RELEASED(event))
		{
			if(EVENT_CODE2(event) == 0 && scancodes[EVENT_CODE1(event)])
			{
				if(key_state[LSHIFT] & (1 << DOWN) || key_state[RSHIFT] & (1 << DOWN))
					*c = shift_scancodes[EVENT_CODE1(event)];
				else
					*c = scancodes[EVENT_CODE1(event)];
				return 1;
			}
		}
	return 0;
}

int poll_string(char *string)
{
	char *cur = string;
	while(*cur)
		cur++;
	char c;
	while(poll_char(&c))
	{
		if(c == '\b')
		{
			if(cur != string)
			{
				cur--;
				vga_printf("\b \b");
			}
			*cur = 0;
		}
		else if(c == '\n')
		{
			vga_printf("\n");
			return 1;
		}
		else
		{
			*cur = c;
			cur++;
			*cur = 0;
			vga_printf("%c", c);
		}
	}
	return 0;
}
