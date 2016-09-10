#include <stdint.h>
#include <limits.h>
#include "vga_io.h"
#include "malloc.h"
#include "debug.h"
#include "keyboard.h"
#include "sprintf.h"
#include "asm.h"

#include <lua.h>
#include <lauxlib.h>

int errno;

static int tostring(lua_State *L, int n)
{
	luaL_checkany(L, n);
	switch (lua_type(L, n))
	{
		case LUA_TNUMBER:
			lua_pushstring(L, lua_tostring(L, n));
			break;
		case LUA_TSTRING:
			lua_pushvalue(L, n);
			break;
		case LUA_TBOOLEAN:
			lua_pushstring(L, (lua_toboolean(L, n) ? "true" : "false"));
			break;
		case LUA_TNIL:
			lua_pushliteral(L, "nil");
			break;
		default:
			lua_pushfstring(L, "%s: %p", luaL_typename(L, n), lua_topointer(L, n));
			break;
	}
	return 1;
}

void print_stack(lua_State *L, int n)
{
	int i, top = lua_gettop(L);
	for(i = top - n + 1; i <= top; i++)
	{
		tostring(L, i);
		vga_printf("%s", luaL_checkstring(L, -1));
		lua_pop(L, 1);
		if(i != top)
			vga_printf(" ");
	}
	vga_printf("\n");
}

void main()
{
	init_alloc();

	lua_State *L = luaL_newstate();

	char str[4096];
	vga_printf("> ");
	str[0] = 0;

	while(1)
	{
		while(poll_string(str))
		{
			if(luaL_loadbuffer(L, str, strlen(str), "console"))
			{
				vga_printf("%s\n", luaL_checkstring(L, -1));
				lua_pop(L, 1);
			}
			else
			{
				lua_pcall(L, 0, LUA_MULTRET, 0);
				print_stack(L, lua_gettop(L));
				lua_settop(L, 0);
			}

			vga_printf("> ");
			str[0] = 0;
		}
		pause();
	}
	lua_close(L);
	vga_set_color(0xC, 0x0);
	vga_printf("HALT");
}
