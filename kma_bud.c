/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the buddy algorithm
 *    Author: Stefan Birrer
 *    Copyright: 2004 Northwestern University
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    Revision 1.2  2009/10/31 21:28:52  jot836
 *    This is the current version of KMA project 3.
 *    It includes:
 *    - the most up-to-date handout (F'09)
 *    - updated skeleton including
 *        file-driven test harness,
 *        trace generator script,
 *        support for evaluating efficiency of algorithm (wasted memory),
 *        gnuplot support for plotting allocation and waste,
 *        set of traces for all students to use (including a makefile and README of the settings),
 *    - different version of the testsuite for use on the submission site, including:
 *        scoreboard Python scripts, which posts the top 5 scores on the course webpage
 *
 *    Revision 1.1  2005/10/24 16:07:09  sbirrer
 *    - skeleton
 *
 *    Revision 1.2  2004/11/05 15:45:56  sbirrer
 *    - added size as a parameter to kma_free
 *
 *    Revision 1.1  2004/11/03 23:04:03  sbirrer
 *    - initial version for the kernel memory allocator project
 *
 ***************************************************************************/
#ifdef KMA_BUD
#define __KMA_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
/************Private include**********************************************/
#include "kma_page.h"
#include "kma.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */
typedef struct{
  unsigned char size;
  void* next;
  void* prev;
  kma_page_t* page;
} buf_t;

typedef struct free_list_t{
  unsigned char size;
  buf_t* first;
  struct free_list_t* up;
} freelst_t;

typedef struct{
  freelst_t buf32;
  freelst_t buf64;
  freelst_t buf128;
  freelst_t buf256;
  freelst_t buf512;
  freelst_t buf1024;
  freelst_t buf2048;
  freelst_t buf4096;
  freelst_t buf8192;
  int in_use;
} buf_list_t;
/************Global Variables*********************************************/
kma_page_t* gpage = NULL;
/************Function Prototypes******************************************/
/* initialize the control page of buddy system */
void init_page();
/* get buffer from the free list */
void* get_buf(freelst_t* lst);
/* remove a buffer from the free list */	
void delete_buf(buf_t*, freelst_t*);
/* find the right list for certain size */
freelst_t* find_list(int);
/* combine two free buddy */
void buf_union(void*);
/* get the address of buddy */
void* getBud(void*, unsigned char);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{ 
  //printf("REQUEST %d\n", size);
  if(gpage == NULL)
    init_page();
  //free_list_t* freelist = NULL;
  int _size = size + sizeof(buf_t); 
  if(_size > PAGESIZE)
    return NULL;
  //freelst_t* freelist = find_list(malloc_size);
  freelst_t* freelist = NULL;
  buf_list_t* mainlist = (buf_list_t*)((void*)gpage->ptr + sizeof(buf_t));
  if(_size <= 32)
    freelist = &mainlist->buf32;
  else if(_size <= 64)
    freelist = &mainlist->buf64;
  else if(_size <= 128)
    freelist = &mainlist->buf128;
  else if(_size <= 256)
    freelist = &mainlist->buf256;
  else if(_size <= 512)
    freelist = &mainlist->buf512;
  else if(_size <= 1024)
    freelist = &mainlist->buf1024;
  else if(_size <= 2048)
    freelist = &mainlist->buf2048;
  else if(_size <= 4096)
    freelist = &mainlist->buf4096;
  else if(_size <= 8192)
    freelist = &mainlist->buf8192;
  //printf("SS %d\n", 1<<freelist->size);
  void* _buf = get_buf(freelist);
  return _buf;
}
/*
freelst_t* find_list(int size)
{
  buf_list_t* mainlist = (buf_list_t*)((void*)gpage->ptr + sizeof(buf_t));
  freelst_t* freelist = NULL;
  if(size <= 32)
    freelist = &mainlist->buf32;
  else if(size <= 64)
    freelist = &mainlist->buf64;
  else if(size <= 128)
    freelist = &mainlist->buf128;
  else if(size <= 256)
    freelist = &mainlist->buf256;
  else if(size <= 512)
    freelist = &mainlist->buf512;
  else if(size <= 1024)
    freelist = &mainlist->buf1024;
  else if(size <= 2048)
    freelist = &mainlist->buf2048;
  else if(size <= 4096)
    freelist = &mainlist->buf4096;
  else if(size <= 8192)
    freelist = &mainlist->buf8192;
  return freelist;
}
*/
void init_page()
{
  gpage = get_page();
  buf_list_t* mainlist = (buf_list_t*)((void*)gpage->ptr + sizeof(buf_t));
  mainlist->buf8192.size = 13;
  mainlist->buf4096.size = 12;
  mainlist->buf2048.size = 11;
  mainlist->buf1024.size = 10;
  mainlist->buf512.size = 9;
  mainlist->buf256.size = 8;
  mainlist->buf128.size = 7;
  mainlist->buf64.size = 6;
  mainlist->buf32.size = 5;
  
  mainlist->buf8192.first = NULL;
  mainlist->buf4096.first = NULL;
  mainlist->buf2048.first = NULL;
  mainlist->buf1024.first = NULL;
  mainlist->buf512.first = NULL;
  mainlist->buf256.first = NULL;
  mainlist->buf128.first = NULL;
  mainlist->buf64.first = NULL;
  mainlist->buf32.first = NULL;

  mainlist->buf8192.up = NULL;
  mainlist->buf4096.up = &mainlist->buf8192;
  mainlist->buf2048.up = &mainlist->buf4096;
  mainlist->buf1024.up = &mainlist->buf2048;
  mainlist->buf512.up = &mainlist->buf1024;
  mainlist->buf256.up = &mainlist->buf512;
  mainlist->buf128.up = &mainlist->buf256;
  mainlist->buf64.up = &mainlist->buf128;
  mainlist->buf32.up = &mainlist->buf64;
  
  buf_t* buf = gpage->ptr;
  buf->next = NULL;
  buf->prev = NULL;
  mainlist->buf8192.first = buf;
  kma_malloc(sizeof(buf_list_t));
  mainlist->in_use = 0;
}

void* get_buf(freelst_t* lst)
{
  buf_list_t* list = (buf_list_t*)((void*)gpage->ptr + sizeof(buf_t));
  list->in_use++;
  if(lst->first == NULL)
    {
      if(lst->up == NULL)
	{
	  //printf("||\n");
	  kma_page_t* page = get_page();
	  buf_t* buf = page->ptr;
	  buf->next = (void*)lst;
	  buf->size = lst->size;
	  buf->page = page;
	  return ((void*)buf + sizeof(buf_t));
	}
      else
	{
	  //printf("----\n");
	  buf_t* buf_l = (buf_t*)(get_buf(lst->up) - sizeof(buf_t));
	  //printf("LS %d\n", 1<<lst->size);
	  uintptr_t offset = 0x1<<lst->size;
	  buf_t* buf_r = (buf_t*)((void*)buf_l + offset);
	  if(lst->first != NULL)
	    lst->first->prev = buf_r;
	  buf_r->next = (void*)lst->first;
	  lst->first = buf_r;
	  buf_r->prev = NULL;
	  buf_r->size = lst->size;
	  buf_l->size = lst->size;
	  buf_l->next = (void*)lst;
 	  return ((void*)buf_l + sizeof(buf_t));
	}
    }
  //printf("==\n");
  buf_t* buf = lst->first;
  lst->first = buf->next;
  buf->next = (void*)lst;
  buf->size = lst->size;
  if(lst->first != NULL)
    lst->first->prev = NULL;

  return ((void*)buf + sizeof(buf_t));
}

void 
kma_free(void* ptr, kma_size_t size)
{
  buf_union(ptr);
  buf_list_t* list = (buf_list_t*)((void*)gpage->ptr + sizeof(buf_t)); 
  if(list->in_use == 0)
    {
      free_page(gpage);
      gpage = NULL;
    }
}

void buf_union(void* ptr)
{
  buf_list_t* list = (buf_list_t*)((void*)gpage->ptr + sizeof(buf_t));
  list->in_use--;

  buf_t* buf = (buf_t*)((void*)ptr - sizeof(buf_t));
  freelst_t* lst = (freelst_t*)buf->next;
  buf_t* bud = (buf_t*) getBud((void*)buf, lst->size);
  if(bud->next == buf->next || lst->up == NULL || buf->size != bud->size)
    // bud in use | in 8192 levle | not its bud
    {
      if(lst->up == NULL)
	free_page(buf->page);
      else
	{
	  if(lst->first != NULL)
	    lst->first->prev = buf;
	  buf->next = (void*)lst->first;
	  lst->first = buf;
	  buf->prev = NULL;
	}
    }
  else
    {
      delete_buf(bud, lst);
      if(buf > bud)
	{
	  
	  buf_t* tmp = buf;
	  buf = bud;
	  bud = tmp;
	}
      buf->next = lst->up;
      buf->size++;
      buf_union((void*)buf + sizeof(buf_t));
    }
}

void* getBud(void* buf, unsigned char size)
{
  uintptr_t size_t= 1<<size;
  uintptr_t head= (uintptr_t)buf;
  return (void*)(size_t ^ head);
}

void delete_buf(buf_t* buf, freelst_t* lst)
{
  buf_t* next = (buf_t*)buf->next;
  buf_t* prev = (buf_t*)buf->prev;
  if(lst->first == buf)
      lst->first = next;
  if(next != NULL)
    next->prev = prev;
  if(prev != NULL)
    prev->next = next;
}
#endif // KMA_BUD
