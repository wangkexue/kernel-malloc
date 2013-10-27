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
  int gflag;
  int page_count;
  void* first;
  //void* ptr;
} page_t;

/************Global Variables*********************************************/
kma_page_t* gpage_entry = NULL;
//int gflag = 0;
int gnum = 0; // for debug
/************Function Prototypes******************************************/
/* initialize a new page */
void init_page(kma_page_t* page, int gflag);
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
  //printf("num: %d; size: %d\n", gnum, size);
  //gnum++;
  //printf("begin\n");
  int malloc_size = size + sizeof(void*);
  if(malloc_size > PAGESIZE)
    return NULL;
  if(!gpage_entry)
    {
      gpage_entry = get_page();
      *((kma_page_t**)gpage_entry->ptr) = gpage_entry;      
      init_page(gpage_entry, 0);
      page_t* first_page = (page_t*)(gpage_entry->ptr);
      first_page->gflag = 1;
    }
   
  void* _first_fit = first_fit(size);
  /*
  page_t* first_page = (page_t*)(gpage_entry->ptr);
  entry_t* entry = (entry_t*)(first_page->first);
  if(entry)
    {
      int i;
      if(entry)
	{
	  for(i=0;entry->next != NULL;i++)
	    {
	      printf("%d: %d\n", i, entry->size);
	      entry = (entry_t*)(entry->next);
	    }
	  printf("%d: %d\n", i, entry->size);
	  printf("\n");
	}
    }
  printf("end\n");
  */
  return _first_fit;
}

void add_entry(void* entry, int size)
{
 ((entry_t*)entry)->size = size;
 ((entry_t*)entry)->prev = NULL;
 page_t* first_page = (page_t*)(gpage_entry->ptr);
 void** first_entry = first_page->first;
 //printf("++this entry %x will be added\n", entry);
 //printf("++first entry %x\n", first_entry);
 if(first_entry == NULL)
   {
     //printf("NULL\n");
     first_page->first = entry;
     ((entry_t*)entry)->next = NULL;
   }
 else if(entry == first_entry)
   (((entry_t*)entry)->next) = NULL;
 else if ((void**)entry < first_entry)
   {
     ((entry_t*)entry)->next = first_entry;
     ((entry_t*)first_entry)->prev = entry;
     first_page->first = entry;
   }
 else
   {
     while(((entry_t*)first_entry)->next != NULL && (void**)entry > first_entry)
       {
	 first_entry =((void*)(((entry_t*)first_entry)->next));
       }
     void* temp = ((entry_t*)first_entry)->next;
     if(temp == NULL && (void**)entry > first_entry)
       {
	 //printf("temp == NULL\n");
	 ((entry_t*)first_entry)->next = entry;
	 ((entry_t*)entry)->prev = first_entry;
	 ((entry_t*)entry)->next = NULL;
       }
     else
       {
	 //printf("(next)->prev->next %x\n", ((entry_t*)((entry_t*)first_entry)->prev)->next);
	 //printf("%x\n", ((entry_t*)((entry_t*)(*first_entry))->prev)->next);
	 ((entry_t*)entry)->next = first_entry;
	 ((entry_t*)entry)->prev = ((entry_t*)first_entry)->prev;
	 ((entry_t*)((entry_t*)first_entry)->prev)->next = entry;
	 ((entry_t*)first_entry)->prev = entry;
       }
   }
}

void init_page(kma_page_t* page, int gflag)
{
  if(gflag == 0)
    {
      // what if the first page is freed?
      page_t* first_page;
      first_page = (page_t*)page->ptr;
      first_page->first = page->ptr + sizeof(page_t);
      //*((kma_page_t**)(first_page->ptr)) = page;
      add_entry((void*)(page->ptr + sizeof(page_t)), PAGESIZE - sizeof(page_t));
      first_page->page_count = 0;
    }
  else if(gflag == 2)
    {
      kma_page_t* new_page = page;
      page_t* first_page = (page_t*)gpage_entry->ptr;
      first_page->first = (void*)page->ptr;
      first_page->gflag = 1;
      *((kma_page_t**)(new_page->ptr)) = new_page;
      add_entry((void*)(page->ptr + sizeof(kma_page_t*)), PAGESIZE - sizeof(kma_page_t*));
    }
  else
    {
      kma_page_t* new_page = page;
      *((kma_page_t**)(new_page->ptr)) = new_page;
      add_entry((void*)(page->ptr + sizeof(kma_page_t*)), PAGESIZE - sizeof(kma_page_t*));
    }      
}

void delete_entry(entry_t* entry)
{
  //printf("--this entry %x will be deleted\n", entry);
  if(entry->prev == NULL && entry->next == NULL)    
    // delete the only entry
    {
      //printf("delete\n");
      page_t* first_page = (page_t*)(gpage_entry->ptr);
      first_page->first = NULL;
      first_page->gflag = 2;
      //gflag = 2;
      return;
    }
  else if(entry->prev == NULL)   
    // delete the first entry
    {
      //((entry_t*)(entry->next))->prev = (entry_t*)(entry->prev);
      page_t* first_page = (page_t*)(gpage_entry->ptr);
      //printf("first entry before delete_entry %x\n", first_page->first);
      first_page->first = entry->next;
      ((entry_t*)(entry->next))->prev = NULL;
      //printf("first entry after delete_entry %x\n", first_page->first);
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

void* first_fit(kma_size_t size) 
{
  int min_size = sizeof(entry_t);
  page_t* first_page = (page_t*)(gpage_entry->ptr);
  entry_t* entry = (entry_t*)(first_page->first);
  //printf("first fit first entry: %x.\n", entry);
  //entry_t* temp;
  if(size < min_size)
    size = min_size;
  while(entry != NULL)
    {
       if(size > entry->size)
	{
	  //printf("bad-entry-size: %d\n", entry->size);
	  entry = entry->next;
	  continue;
	}
      else if(size == entry->size)
	{
	  delete_entry(entry);
	  return (void*)entry;
	}
      else if(size > entry->size - min_size)
	{
	  entry = entry->next;
	  continue;
	}
      else
	{
	  add_entry((void*)(entry) + size, entry->size - size);
	  delete_entry(entry);
	  return (void*)entry;
	}
    }
  kma_page_t* new_page = get_page();
  *((kma_page_t**)new_page->ptr) = new_page;
  init_page(new_page, first_page->gflag);
  first_page->page_count++;
  return first_fit(size);
}

void
kma_free(void* ptr, kma_size_t size)
{
  //printf("FFthis ptr will be freed %x\n", ptr);
  int min_size = sizeof(entry_t);
  if(size < min_size)
    size = min_size;
  page_t* first_page = (page_t*)gpage_entry->ptr;
  entry_t* first_entry = (entry_t*)first_page->first;
  void* page; 
  if(first_entry == NULL)
    // all the pages are allocated
    {
      add_entry(ptr, size);
      page = ptr;
    }
  else
    {
      while(first_entry->next != NULL && ptr > (void*)first_entry)
	{
	  first_entry = (entry_t*)first_entry->next;
	}
      entry_t* prev = (entry_t*)first_entry->prev;
      entry_t* next = (entry_t*)first_entry->next;
      if(prev == NULL && ptr < (void*)first_entry){
	// free before first entry      
	if(ptr + size == (void*)first_entry && BASEADDR(ptr) == BASEADDR(first_entry))
	  {
	    size += first_entry->size;
	    add_entry(ptr, size);
	    delete_entry(first_entry);
	}
	else
	  add_entry(ptr, size);
	page = ptr;
      }
      else if(next == NULL && ptr > (void*)first_entry)
	{
	  //printf("NEXT==NULL\n");
	  // free after last entry
	  //printf("**first_entry %x, size %d\n", first_entry, first_entry->size);

	  if((void*)first_entry + first_entry->size == ptr && BASEADDR(ptr) == BASEADDR(first_entry))
	    {
	      size += first_entry->size;
	      delete_entry(first_entry);
	      add_entry(first_entry, size);
	      page = (void*)first_entry;
	    }
	  else
	    {
	      add_entry(ptr, size);
	      page = ptr;
	    }
	}
      else
	// free inbetween
	{
	  if(ptr + size == (void*)first_entry &&
	     (void*)prev + prev->size == ptr && 
	     BASEADDR(ptr) == BASEADDR(prev) &&
	     BASEADDR(ptr) == BASEADDR(first_entry))
	    {
	      delete_entry(prev);
	      add_entry((void*)prev, size + prev->size + first_entry->size);
	      delete_entry(first_entry);
	      page = (void*)prev;
	    }
	  else if(ptr + size == (void*)first_entry && BASEADDR(ptr) == BASEADDR(first_entry))
	    {
	      size += first_entry->size;
	      add_entry(ptr, size);
	      page = ptr;
	      delete_entry(first_entry);
	    }
	  else if((void*)prev + prev->size == ptr && BASEADDR(ptr) == BASEADDR(prev))
	    {
	      delete_entry(prev);
	      add_entry((void*)prev, size + prev->size);
	      page = (void*)prev;
	    }
	  else
	    {
	      add_entry(ptr, size);
	      page = ptr;
	    }
	}
    }
  //printf("$$ %d\n", first_page->page_count);
  //page_t* first_page = (page_t*)(gpage_entry->ptr);  
  if(((entry_t*)page)->size == PAGESIZE - sizeof(page_t) &&
     BASEADDR(page) == BASEADDR(gpage_entry->ptr))
    {
      if(first_page->page_count == 0)
	{
	  free_page(gpage_entry);
	  gpage_entry = NULL;
	}
      return;
    }
  if(((entry_t*)page)->size == PAGESIZE - sizeof(kma_page_t*))
    {
      kma_page_t* Fpage;
      Fpage = *((kma_page_t**)(page - sizeof(kma_page_t*)));
      delete_entry((entry_t*)page);
      free_page(Fpage);
      first_page->page_count--;
    }
  if(((entry_t*)first_page->first)->size == PAGESIZE - sizeof(page_t) &&
     BASEADDR(first_page->first) == BASEADDR(gpage_entry->ptr))
    {
      if(first_page->page_count == 0)
	{
	  free_page(gpage_entry);
	  gpage_entry = NULL;
	}
    }

  /*  
  entry_t* entry = (entry_t*)(first_page->first);
  if(entry)
    {
      int i;
      if(entry)
	{
	  for(i=0;entry->next != NULL;i++)
	    {
	      printf("%d: %d\n", i, entry->size);
	      entry = (entry_t*)(entry->next);
	    }
	  printf("%d: %d\n", i, entry->size);
	  printf("\n");
	}
    }
  */
}

#endif // KMA_RM
