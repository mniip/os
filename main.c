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

int lua_disk_read(lua_State *L)
{
	int disk = luaL_checkinteger(L, 1);
	int sector = luaL_checkinteger(L, 2);
	disk_chs chs;
	int result = disk_get_chs(disk, &chs);
	if(result)
		return luaL_error(L, "Could not get CHS: %02x", result);
	char data[512];
	result = disk_read(disk, &chs, sector, data);
	if(result)
		return luaL_error(L, "Could not read: %02x", result);
	lua_pushlstring(L, data, 512);
	return 1;
}

int lua_disk_write(lua_State *L)
{
	int disk = luaL_checkinteger(L, 1);
	int sector = luaL_checkinteger(L, 2);
	disk_chs chs;
	int result = disk_get_chs(disk, &chs);
	if(result)
		return luaL_error(L, "Could not get CHS: %02x", result);
	char data[512];
	char const *str = luaL_checkstring(L, 3);
	int i;
	for(i = 0; i < 512; i++)
		data[i] = str[i];
	result = disk_write(disk, &chs, sector, data);
	if(result)
		return luaL_error(L, "Could not write: %02x", result);
	return 0;
}

void main()
{
	vga_printf("Initialized C\n");
	init_handlers();
	init_alloc();
	print_regions();
	vga_printf("Booted from drive 0x%02x\n", drive);

	lua_State *L = luaL_newstate();
	vga_printf("Initialized Lua\n");
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
	lua_pushcfunction(L, lua_disk_read);
	lua_setglobal(L, "disk_read");
	lua_pushcfunction(L, lua_disk_write);
	lua_setglobal(L, "disk_write");

	luaopen_base(L);
	lua_pop(L, 1);
	luaopen_string(L);
	lua_setglobal(L, "string");

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
