#include <stdint.h>
#include <limits.h>
#include "vga_io.h"
#include "malloc.h"
#include "debug.h"
#include "keyboard.h"
#include "sprintf.h"
#include "asm.h"
#include "disk.h"

#include "string.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

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

int lua_vga_set_color(lua_State *L)
{
	vga_set_color(luaL_optinteger(L, 1, 7), luaL_optinteger(L, 2, 0));
	return 0;
}

int lua_vga_print(lua_State *L)
{
	vga_printf("%s\n", luaL_optstring(L, 1, ""));
	return 0;
}

int lua_getline(lua_State *L)
{
	char str[4096];
	strcpy(str, "");
	while(1)
	{
		if(poll_string(str))
		{
			lua_pushstring(L, str);
			return 1;
		}
		pause();
	}
}

int lua_peek(lua_State *L)
{
	uint32_t pointer = luaL_checkinteger(L, 1);
	uint8_t value = *(uint8_t *)pointer;
	lua_pushinteger(L, value);
	return 1;
}

int lua_poke(lua_State *L)
{
	uint32_t pointer = luaL_checkinteger(L, 1);
	uint8_t value = luaL_checkinteger(L, 2);
	*(uint8_t *)pointer = value;
	return 0;
}

int lua_file_list(lua_State *L)
{
	void const *path = luaL_checkstring(L, 1);
	dir_entry *dir = file_list(path);
	if(!dir)
		return luaL_error(L, "Could not read directory");
	lua_newtable(L);
	int i;
	for(i = 0; dir[i].type; i++)
	{
		char filename[sizeof dir[i].name + 2];
		memcpy(filename, dir[i].name, sizeof dir[i].name);
		filename[sizeof dir[i].name] = 0;
		if(dir[i].type == TYPE_DIR)
		{
			int len = strlen(filename);
			filename[len] = '/';
			filename[len + 1] = 0;
		}
		lua_pushstring(L, filename);
		lua_rawseti(L, -2, i + 1);
	}
	free(dir);
	return 1;
}

int lua_file_mkdir(lua_State *L)
{
	file_mkdir(luaL_checkstring(L, 1));
	return 0;
}

int lua_file_remove(lua_State *L)
{
	file_remove(luaL_checkstring(L, 1));
	return 0;
}

void main()
{
	init_handlers();
	init_alloc();

	lua_State *L = luaL_newstate();
	lua_pushcfunction(L, lua_vga_set_color);
	lua_setglobal(L, "vga_set_color");
	lua_pushcfunction(L, lua_vga_print);
	lua_setglobal(L, "vga_print");
	lua_pushcfunction(L, lua_getline);
	lua_setglobal(L, "getline");
	lua_pushcfunction(L, lua_peek);
	lua_setglobal(L, "peek");
	lua_pushcfunction(L, lua_poke);
	lua_setglobal(L, "poke");
	lua_pushcfunction(L, lua_file_list);
	lua_setglobal(L, "file_list");
	lua_pushcfunction(L, lua_file_mkdir);
	lua_setglobal(L, "file_mkdir");
	lua_pushcfunction(L, lua_file_remove);
	lua_setglobal(L, "file_remove");
	luaL_dostring(L, "function dirtree(path, tab) tab = tab or '' path = path or '/' vga_print(tab .. path:match'[^/]*/?$') if path:sub(-1) == '/' then for _, v in ipairs(file_list(path)) do dirtree(path .. v, tab .. '    ') end end end");

	luaopen_base(L);
	lua_pop(L, 1);
	luaopen_string(L);
	lua_setglobal(L, "string");
	luaopen_io(L);
	lua_setglobal(L, "io");

	char str[4096];
	strcpy(str, "");
	vga_printf("> ");

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
				if(lua_gettop(L))
					print_stack(L, lua_gettop(L));
				lua_settop(L, 0);
			}

			vga_printf("> ");
			strcpy(str, "");
		}
		pause();
	}
	lua_close(L);
	vga_set_color(0xC, 0x0);
	vga_printf("HALT");
}
