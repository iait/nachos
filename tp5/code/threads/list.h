// list.h 
//	Data structures to manage LISP-like lists.  
//
//      As in LISP, a list can contain any type of data structure
//	as an item on the list: thread control blocks, 
//	pending interrupts, etc. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef LIST_H
#define LIST_H

#include "copyright.h"
#include "utility.h"

// The following class defines a "list element" -- which is
// used to keep track of one item on a list.  
//
// Internal data structures kept public so that List operations can
// access them directly.

template <class Item>
class ListElement {
   public:
     ListElement(Item itemPtr, int sortKey);	// initialize a list element

     ListElement *next;		// next element on list, 
				// NULL if this is the last
     int key;		    	// priority, for a sorted list
     Item item; 	    	// item on the list
};

// The following class defines a "list" -- a singly linked list of
// list elements, each of which points to a single item on the list.
//
// By using the "Sorted" functions, the list can be kept in sorted
// in increasing order by "key" in ListElement.

template <class Item>
class List {
  public:
    List();			// initialize the list
    ~List();			// de-allocate the list

    void Prepend(Item item); 	// Put item at the beginning of the list

    void Append(int key, Item item);   // Pone un elemento con clave al final de la lista
    void Append(Item item); 	// Put item at the end of the list

    Item Remove(); 	 	// Take item off the front of the list
    bool Remove(Item item);     // Elimina el elemento de la lista si existe
    bool Remove(int key, Item *item);  // Elimina el elemento por su clave

    void Apply(void (*func)(Item));	// Apply "func" to all elements in list 

    bool IsEmpty();		// is the list empty? 
    

    // Routines to put/get items on/off list in order (sorted by key)
    void SortedInsert(Item item, int sortKey);	// Put item into list
    Item SortedRemove(int *keyPtr); 	  	// Remove first item from list

  private:
    typedef ListElement<Item> ListNode;
    ListNode *first;  		// Head of the list, NULL if list is empty
    ListNode *last;		// Last element of list
};

//----------------------------------------------------------------------
// ListElement::ListElement
// 	Initialize a list element, so it can be added somewhere on a list.
//
//	"anItem" is the item to be put on the list.  
//	"sortKey" is the priority of the item, if any.
//----------------------------------------------------------------------

template <class Item>
ListElement<Item>::ListElement(Item anItem, int sortKey)
{
     item = anItem;
     key = sortKey;
     next = NULL;	// assume we'll put it at the end of the list 
}

//----------------------------------------------------------------------
// List::List
//	Initialize a list, empty to start with.
//	Elements can now be added to the list.
//----------------------------------------------------------------------

template <class Item>
List<Item>::List()
{ 
    first = last = NULL; 
}

//----------------------------------------------------------------------
// List::~List
//	Prepare a list for deallocation.  If the list still contains any 
//	ListElements, de-allocate them.  However, note that we do *not*
//	de-allocate the "items" on the list -- this module allocates
//	and de-allocates the ListElements to keep track of each item,
//	but a given item may be on multiple lists, so we can't
//	de-allocate them here.
//----------------------------------------------------------------------

template <class Item>
List<Item>::~List()
{ 
    // delete all the list elements
    while ( !IsEmpty() ) {
      Remove();
    }
}

//----------------------------------------------------------------------
// List::Append
//      Append an "item" to the end of the list.
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, put it at the end.
//
//      "item" is the thing to put on the list, it can be a pointer to
//              anything.
//      "key" clave con la que se asocia al elemento, si no se proporciona
//              se usa cero.
//----------------------------------------------------------------------
template <class Item>
void
List<Item>::Append(int key, Item item)
{
    ListNode *element = new ListNode(item, key);

    if (IsEmpty()) {            // list is empty
        first = element;
        last = element;
    } else {                    // else put it after last
        last->next = element;
        last = element;
    }
}
template <class Item>
void
List<Item>::Append(Item item)
{
    Append(0, item);
}

//----------------------------------------------------------------------
// List::Prepend
//      Put an "item" on the front of the list.
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, put it at the beginning.
//
//	"item" is the thing to put on the list, it can be a pointer to 
//		anything.
//----------------------------------------------------------------------

template <class Item>
void
List<Item>::Prepend(Item item)
{
    ListNode *element = new ListNode(item, 0);

    if (IsEmpty()) {		// list is empty
	first = element;
	last = element;
    } else {			// else put it before first
	element->next = first;
	first = element;
    }
}

//----------------------------------------------------------------------
// List::Remove
//      Remove the first "item" from the front of the list.
// 
// Returns:
//	Pointer to removed item, NULL if nothing on the list.
//----------------------------------------------------------------------

template <class Item>
Item
List<Item>::Remove()
{
    return SortedRemove(NULL);  // Same as SortedRemove, but ignore the key
}

//----------------------------------------------------------------------
// List::Apply
//	Apply a function to each item on the list, by walking through  
//	the list, one element at a time.
//
//	"func" is the procedure to apply to each element of the list.
//----------------------------------------------------------------------

template <class Item>
void
List<Item>::Apply(void (*func)(Item))
{
    for (ListNode *ptr = first; ptr != NULL; ptr = ptr->next) {
       func(ptr->item);
    }
}

//----------------------------------------------------------------------
// List::IsEmpty
//      Returns true if the list is empty (has no items).
//----------------------------------------------------------------------

template <class Item>
bool
List<Item>::IsEmpty() 
{ 
    if (first == NULL)
        return true;
    else
	return false; 
}

//----------------------------------------------------------------------
// List::SortedInsert
//      Insert an "item" into a list, so that the list elements are
//	sorted in increasing order by "sortKey".
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, walk through the list, one element at a time,
//	to find where the new item should be placed.
//
//	"item" is the thing to put on the list, it can be a pointer to 
//		anything.
//	"sortKey" is the priority of the item.
//----------------------------------------------------------------------

template <class Item>
void
List<Item>::SortedInsert(Item item, int sortKey)
{
    ListNode *element = new ListNode(item, sortKey);
    ListNode *ptr;		// keep track

    if (IsEmpty()) {	// if list is empty, put
        first = element;
        last = element;
    } else if (sortKey < first->key) {	
		// item goes on front of list
	element->next = first;
	first = element;
    } else {		// look for first elt in list bigger than item
        for (ptr = first; ptr->next != NULL; ptr = ptr->next) {
            if (sortKey < ptr->next->key) {
		element->next = ptr->next;
	        ptr->next = element;
		return;
	    }
	}
	last->next = element;		// item goes at end of list
	last = element;
    }
}

//----------------------------------------------------------------------
// List::SortedRemove
//      Remove the first "item" from the front of a sorted list.
// 
// Returns:
//	Pointer to removed item, NULL if nothing on the list.
//	Sets *keyPtr to the priority value of the removed item
//	(this is needed by interrupt.cc, for instance).
//
//	"keyPtr" is a pointer to the location in which to store the 
//		priority of the removed item.
//----------------------------------------------------------------------

template <class Item>
Item
List<Item>::SortedRemove(int *keyPtr)
{
    ListNode *element = first;

    if (IsEmpty()) 
	return Item();

    Item thing = first->item;
    if (first == last) {	// list had one item, now has none 
        first = NULL;
	last = NULL;
    } else {
        first = element->next;
    }
    if (keyPtr != NULL)
        *keyPtr = element->key;
    delete element;
    return thing;
}

//----------------------------------------------------------------------
// List::Remove
//      Elimina el elemento de la lista con clave "key" y lo asigna en
//      "item". Retorna true si el elemento fue encontrado.
//
//      "key" clave del elemento a eliminar.
//      "item" donde se devolverá el elemento eliminado, si se encuentra.
//----------------------------------------------------------------------
template <class Item>
bool
List<Item>::Remove(int key, Item *item)
{
    // si la lista está vacía el elemento no está
    if (IsEmpty()) {
        return false;
    }

    // si la lista tiene solo un elemento
    if (first == last) {
        if (first->key == key) {
            *item = first->item;
            delete first;
            first = NULL;
            last = NULL;
            return true;
        }
        return false;
    }

    // si el item a buscar es el primero (la lista tiene más de un elemento)
    ListNode *current = first;
    if (first->key == key) {
        *item = first->item;
        first = first->next;
        delete current;
        return true;
    }

    // itera sobre la lista
    ListNode *prev = current;
    current = current->next;
    while (current->key != key) {
        if (current == last) {
            return false; // llegó al final de la lista y no se encontró
        }
        prev = current;
        current = current->next;
    }
    // se encontró el item en current
    *item = current->item;
    if (current == last) {
        prev->next = NULL;
        last = prev;
    } else {
        prev->next = current->next;
    }
    delete current;
    return true;
}

//----------------------------------------------------------------------
// List::Remove
//      Elimina el elemento "item" de la lista.
//      Retorna true si el elemento fue encontrado.
//
//      "item" elemento a eliminar.
//----------------------------------------------------------------------
template <class Item>
bool
List<Item>::Remove(Item item)
{
    // si la lista está vacía el elemento no está
    if (IsEmpty()) {
        return false;
    }

    // si la lista tiene solo un elemento
    if (first == last) {
        if (first->item == item) {
            delete first;
            first = NULL;
            last = NULL;
            return true;
        }
        return false;
    }

    // si el item a buscar es el primero (la lista tiene más de un elemento)
    ListNode *current = first;
    if (first->item == item) {
        first = first->next;
        delete current;
        return true;
    }

    // itera sobre la lista
    ListNode *prev = current;
    current = current->next;
    while (current->item != item) {
        if (current == last) {
            return false; // llegó al final de la lista y no se encontró
        }
        prev = current;
        current = current->next;
    }
    // se encontró el item en current
    if (current == last) {
        prev->next = NULL;
        last = prev;
    } else {
        prev->next = current->next;
    }
    delete current;
    return true;
}

#endif // LIST_H