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
/************Function Prototypes******************************************/
/* initialize a new page */
void init_page(kma_page_t* page);
/* change an entry */
void change_entry(void* entry, int size);
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
      init_page(gpage_entry);
    } 
  void* _first_fit = first_fit(size);
  return _first_fit;
}

void add_entry(void* entry, int size)
{
  ((entry_t*)entry)->size = size;
  ((entry_t*)entry)->prev = NULL;
  ((entry_t*)entry)->next = NULL;
}

void init_page(kma_page_t* page)
{
  page_t* frontpage = NULL;
  frontpage->first = (page_t*)(page->ptr + sizeof(page_t*));
  add_entry(frontpage->first, PAGESIZE - sizeof(page_t*));  
}

void delete_entry(entry_t* entry)
{
}

void change_entry(void* entry, int offset, int size)
{
}

void* first_fit(kma_size_t size) 
{
  int min_size = sizeof(entry_t*);
  entry_t* entry = (entry_t*)(gpage_entry->ptr);
  while(entry)
    {
      if(size < min_size)
	size = min_size;
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
