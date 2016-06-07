#include <stdint.h>
#include "vga_io.h"

typedef struct
{
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t extra;
}
ram_entry;

extern uint32_t mem_info_sz;
extern ram_entry mem_info[];

void filter_regions()
{
	// TODO: handle intersecting and out of order regions
	uint32_t in, out = 0;
	for(in = 0; in < mem_info_sz; in++)
		if(mem_info[in].type == 1 && mem_info[in].base < 0x100000000ULL)
		{
			mem_info[out] = mem_info[in];
			out++;
		}
	mem_info_sz = out;
}

void *find_slot(void *after, uint32_t min)
{
	uint32_t i;
	for(i = 0; i < mem_info_sz; i++)
		if((uint32_t)mem_info[i].base + (uint32_t)mem_info[i].length >= (uint32_t)after + min)
		{
			if((uint32_t)mem_info[i].base >= (uint32_t)after)
				return (void *)(uint32_t)mem_info[i].base;
			else
				return after;
		}
	return 0;
}

uint32_t region_after(void *after)
{
	uint32_t i;
	for(i = 0; i < mem_info_sz; i++)
		if((uint32_t)mem_info[i].base <= (uint32_t)after)
			if((uint32_t)mem_info[i].base + (uint32_t)mem_info[i].length >= (uint32_t)after)
				return ((uint32_t)mem_info[i].base + (uint32_t)mem_info[i].length - (uint32_t)after);
	return 0;
}

void print_regions()
{
	vga_printf("RAM map:\n");
	ram_entry *list = mem_info;
	uint32_t i;
	for(i = 0; i < mem_info_sz; i++)
	{
		vga_printf("0x%016llx-0x%016llx (0x%llx) (type %u)\n", list[i].base, list[i].base + list[i].length - 1, list[i].length, list[i].type);
	}
}

typedef struct free_list_
{
	struct free_list_ *next;
	uint32_t size;
}
free_list;

free_list *list_head = 0x0;

void print_free_list()
{
	vga_printf("FREE LIST: ");
	free_list *list = list_head;
	while(list)
	{
		vga_printf("0x%x (0x%x) --> ", (uint32_t)list, list->size);
		list = list->next;
	}
	vga_printf("0x%x\n", (uint32_t)list);
}

void init_alloc()
{
	filter_regions();

	list_head = find_slot((void *)0x100000, sizeof(free_list));
	free_list *list = list_head;
	do
	{
		uint32_t size = region_after(list);
		free_list *next = find_slot((void *)list + size, sizeof(free_list)); 
		list->next = next;
		list->size = size;
		list = next;
	}
	while(list);
	// TODO: error handling
}

void *malloc(uint32_t size)
{
	free_list **prevptr = &list_head;
	free_list *list = list_head;
	uint32_t act_size = size + sizeof(uint32_t);
	if(act_size < sizeof(free_list))
		act_size = sizeof(free_list);
	while(list)
	{
		if(list->size >= act_size)
		{
			if(list->size - act_size < sizeof(free_list))
				act_size = list->size;
			if(act_size == list->size)
			{
				*prevptr = list->next;
			}
			else
			{
				free_list *new = (void *)list + act_size;
				*prevptr = new;
				new->next = list->next;
				new->size = list->size - act_size;
			}
			*(uint32_t *)list = act_size;
			//vga_printf("malloc(0x%x) = 0x%x\n", size, (uint32_t)((void *)list + sizeof(uint32_t)));
			return (void *)list + sizeof(uint32_t);
		}
		prevptr = &list->next;
		list = list->next;
	}
	return 0;
	// TODO: error handling
}

void free(void *ptr)
{
	//vga_printf("free(0x%x)\n", ptr);
	void *act_ptr = ptr - sizeof(uint32_t);
	uint32_t size = *(uint32_t *)act_ptr;
	free_list *list = list_head;
	if((void *)list < act_ptr && list)
	{
		while((void *)list->next < act_ptr && list->next)
			list = list->next;
		if((void *)list + list->size == act_ptr)
			// merge with preceeding
			if(list->next == act_ptr + size)
			{
				// merge with following
				list->size = list->size + size + list->next->size;
				list->next = list->next->next;
			}
			else
			{
				list->size = list->size + size;
			}
		else
			if(list->next == act_ptr + size)
			{
				// merge with following
				free_list *new = act_ptr;
				new->next = list->next->next;
				new->size = size + list->next->size;
				list->next = new;
			}
			else
			{
				free_list *new = act_ptr;
				new->next = list->next;
				new->size = size;
				list->next = new;
			}
	}
	else
	{
		if(list == act_ptr + size)
		{
			// merge with following
			free_list *new = act_ptr;
			new->next = list->next;
			new->size = size + list->size;
			list_head = new;
		}
		else
		{
			free_list *new = act_ptr;
			new->next = list;
			new->size = size;
			list_head = new;
		}
	}
}
