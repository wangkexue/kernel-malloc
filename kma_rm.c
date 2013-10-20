/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the resource map algorithm
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
#ifdef KMA_RM
#define __KMA_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>
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
  int size;
  //void* entry;
  void* next;
  void* prev;
} entry_t;

typedef struct{
  void* first;
} page_t;

/************Global Variables*********************************************/
kma_page_t* gpage_entry = NULL;
int gflag = 0;
/************Function Prototypes******************************************/
/* initialize a new page */
void init_page(kma_page_t* page);
/* change an entry */
void* change_entry(void* entry, int offset, int size);
/* add an entry */
void add_entry(void* entry, int size);
/* delete an entry */
void delete_entry(entry_t* entry);
/* search free memory based on first-fit  */
void* first_fit(kma_size_t size);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  int malloc_size = size + sizeof(void*);
  if(malloc_size > PAGESIZE)
    return NULL;
  if(!gpage_entry)
    {
      gpage_entry = get_page();
      *((kma_page_t**)gpage_entry->ptr) = gpage_entry;      
      init_page(gpage_entry);
      printf("init success\n");
      gflag = 1;
    } 
  void* _first_fit = first_fit(size);
  printf("first_fit success\n");
  return _first_fit;
}

void add_entry(void* entry, int size)
{
 ((entry_t*)entry)->size = size;
 ((entry_t*)entry)->prev = NULL;

 page_t* first_page = (page_t*)(gpage_entry->ptr);
 void* first_entry = first_page->first;
 if(entry == first_entry)
   // add first entry
   (((entry_t*)entry)->next) = NULL;
 else
   {
     while(((entry_t*)first_entry)->next != NULL && entry > first_entry)
       {
	 first_entry = ((entry_t*)first_entry)->next;
       }
     entry_t* temp = ((entry_t*)first_entry)->next;

   	  if(temp == NULL)
     	// temp->prev = entry;
  {	     
     ((entry_t*)first_entry)->next = entry;
     ((entry_t*)entry)->prev = first_entry;
     ((entry_t*)entry)->next = temp;
   }
/*   else
    {
	entry_t* temp = ((entry_t*)first_entry)->prev;
	temp->next = entry;
	((entry_t*)first_entry)->prev = entry;
     ((entry_t*)entry)->next = first_entry;
     ((entry_t*)entry)->first = temp;
     }
*/
  printf("add entry success\n");
     /* 
     if(temp == NULL)
       {
	 ((entry_t*)entry)->next = NULL;
	 ((entry_t*)entry)->prev = first_entry;
	 temp = entry;
       }
     else
       {
	 ((entry_t*)entry)->next = first_entry;
	 ((entry_t*)entry)->prev = ((entry_t*)first_entry)->prev;
	 ((entry_t*)first_entry)->prev = entry;
	 ((entry_t*)((entry_t*)first_entry)->prev)->next = entry;
       }
     */
   }
  
}

void init_page(kma_page_t* page)
{
  if(gflag == 0)
    {
      // what if the first page is freed?
      page_t* first_page;
      first_page = (page_t*)page->ptr;
      first_page->first = (void*)(page->ptr + sizeof(page_t*));
      add_entry(page->ptr + sizeof(page_t*), PAGESIZE - sizeof(page_t*));
    }
  else
    add_entry(page->ptr, PAGESIZE);  
}

void delete_entry(entry_t* entry)
{
  if(entry->prev == NULL && entry->next == NULL)    
    // delete the only entry
    {
      //free_page(gpage_entry);
      gpage_entry = NULL;
      return;
    }
  else if(entry->prev == NULL)   
    // delete the first entry
    {
      ((entry_t*)(entry->next))->prev = NULL;
      page_t* first_page = (page_t*)(gpage_entry->ptr);
      first_page->first = (void*)(entry->next);
      return;
    }
  else if(entry->next == NULL)
    // delete the last entry
    {
      ((entry_t*)(entry->prev))->next = NULL;
      return;
    }
  else
    // delete the middle entry
    {
      ((entry_t*)(entry->prev))->next = entry->next;
      ((entry_t*)(entry->next))->prev = entry->prev;
    }
}
/*
void* change_entry(void* entry, int offset, int size)
{
}
*/
void* first_fit(kma_size_t size) 
{
  int min_size = sizeof(entry_t*);
  page_t* first_page = (page_t*)(gpage_entry->ptr);
  entry_t* entry = (entry_t*)(first_page->first);
  if(size < min_size)
    size = min_size;
  while(entry)
    {
       if(size > entry->size)
	{
	  entry = entry->next;
	  continue;
	}
      else if(size >=  entry->size - min_size)
	{
	  delete_entry(entry);
	  return (void*)entry;
	}
      else
	{
	  add_entry((void*)(entry + size), entry->size - size);
	  delete_entry(entry);
	  return (void*)entry;
	}
    }
  kma_page_t* new_page = get_page();
  init_page(new_page);
  return first_fit(size);
}

void
kma_free(void* ptr, kma_size_t size)
{  
}

#endif // KMA_RM
