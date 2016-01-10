#pragma once

#include <assert.h>

#include "types.h"
#include "containers.h"

// Dynamic array implementation
template<class T>
class Array
{
public:

    Array(uint32 start_size = 16)
        : capacity(start_size),
        write_pos(0)
    {
        data = new T[start_size] {};
    }

    ~Array()
    {
        delete[] data;
    }

    T& operator[](uint32 loc)
    {
        assert(loc < write_pos);
        return this->Get(loc);
    }

    void AddEmpty(uint32 num_to_add = 1)
    {
        assert(write_pos <= capacity);
        if (write_pos + (num_to_add - 1) >= capacity)
        {
            reallocate(num_to_add);
        }
        size_t clear_size = sizeof(T) * num_to_add;
        memset(&data[write_pos], 0, clear_size);
        write_pos += num_to_add;
    }

    T* AddNew()
    {
        assert(write_pos <= capacity);
        if (write_pos + (num_to_add - 1) >= capacity)
        {
            reallocate(num_to_add);
        }

        T* result = new(&data[write_pos]);
        write_pos += 1;
        return result;
    }

    void Add(T value)
    {
        AddBack(value);
    }

    void Reserve(uint32 count)
    {
        if(count > capacity)
        {
            reallocate(count);
        }
    }

    void AddBack(T value)
    {
        if (write_pos >= capacity)
        {
            reallocate();
        }

        data[write_pos++] = value;
    }

    void AddAt(uint32 index, T value)
    {
        if (index > write_pos)
        {
            index = write_pos;
        }
        if (write_pos >= capacity)
        {
            reallocate();
        }

        memmove(&data[index + 1], &data[index], sizeof(T) * (write_pos - index));
        data[index] = value;

        write_pos++;
    }

    T& Get(uint32 position)
    {
        assert(position < write_pos);
        return data[position];
    }

    T* GetBackPtr()
    {
        return &data[write_pos - 1];
    }

    T& GetBack()
    {
        return data[write_pos - 1];
    }

    void Remove(uint32 position)
    {
        assert(position < write_pos);

        if(position == write_pos - 1)
        {
            size_t move_size = (write_pos - position) * sizeof(T);
            memmove(&data[position], &data[position + 1], move_size);
        }
        --write_pos;
    }

    int32 Find(T element)
    {
        int32 result = -1;
        for(uint32 i = 0; i < Size(); ++i)
        {
            if(data[i] == element)
            {
                result = i;
                break;
            }
        }
        return result;
    }

    // NOTE: does not clear, only removes access
    void RemoveBack()
    {
        if(write_pos > 0)
        {
            --write_pos;
        }
    }

    void SwapRemove(uint32 loc)
    {
        data[loc] = data[write_pos - 1];
        RemoveBack();
    }

    void Clear()
    {
        //NOTE(chris): not actually clearing, but items are not accessible
        write_pos = 0;
    }

	uint32 Size()
	{
		return write_pos;
	}

    bool IsEmpty()
    {
        bool result = (write_pos == 0);
        return result;
    }

private:

	void reallocate(uint32 new_element_count = 1)
	{
		T* temp = data;
		uint32 old_size = capacity;
        uint32 half_size = capacity >> 1;
        uint32 grow_size = Maximum(new_element_count, half_size);
        capacity += grow_size;
		data = new T[capacity] {};
        memcpy(data, temp, old_size * sizeof(T));
		delete[] temp;
	}

	T* data;
	uint32 capacity;
	uint32 write_pos;
};



// Structure referenced by the RArray;
template<typename T>
struct RArrayRef
{
    T* ptr;
    uint32 generation;
    uint16 index;

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }

    T* RawPointer() { return ptr; }
};


#define VALID_MASK 0x80000000
// Storing the index here will help with loading save games
template<typename T>
struct Element
{
    T data;
                  // Top bit of the index is off when valid
    uint16 index; // Pointless, but matches the RArrayRef
    uint32 generation;
};
// Referenced Array
// Array that uses a level of indirection to access data so that pointers are never invalidated.
template <typename T, int MAX>
class RArray
{
public:

    Element<T>* elements;

    // NOTE: these don't need to by dynamic arrays, they won't grow beyond MAX
    Array<int32> active_elements;
    Array<int32> inactive_elements;

    RArray()
        : elements(nullptr)
    {
        assert(MAX < (uint32)0xFFFFFFFF - 2);
        elements = new Element<T>[MAX];
        memset(elements, 0, sizeof(elements[0]) * MAX);
        for (int i = MAX - 1; i >= 0; --i)
        {
            inactive_elements.AddBack(i);
            elements[i].generation = 1;
        }
    }

    ~RArray()
    {
        delete[] elements;
    }

    RArrayRef<T> Add(T element)
    {
        RArrayRef<T> result = {};

        if (inactive_elements.Size() != 0)
        {
            int32 location = inactive_elements.GetBack();
            inactive_elements.RemoveBack();

            bool found = false;
            for(uint32 i = 0; i < active_elements.Size(); ++i)
            {
                if(location < active_elements[i])
                {
                    active_elements.AddAt(i, location);
                    found = true;
                    break;
                }
                assert(location != active_elements[i]);
            }
            if (!found)
            {
                active_elements.AddBack(location);
            }

            Element<T>* destination = (elements + location);
            destination->index = (uint16)location;
            destination->data = element;
            destination->generation &= ~VALID_MASK;
            //if (destination->generation == 0) ++destination->generation;
            if (destination->generation == ~VALID_MASK) destination->generation = 1;

            result.index = destination->index;
            result.generation = destination->generation;
            result.ptr = &destination->data;// &elements[location].data;

            return result;
        }

        assert(!"RArray is full!");
        return result;
    }

    uint32 Size()
    {
        uint32 result = active_elements.Size();
        return result;
    }

    bool Remove(RArrayRef<T> ref)
    {
        int32 location = ref.index;
        if (location >= 0)
        {
            //elements[location].id = 0;

            if(elements[location].generation == ~VALID_MASK)
            {
                elements[location].generation = 1;
            }
            else
            {
                ++elements[location].generation; // Increment the generation on removal to invalidate
                                                 // references to this location
            }

            elements[location].index ^= VALID_MASK;

            int active_loc = active_elements.Find(location);
            assert(active_loc >= 0);

            active_elements.Remove(active_loc);

            bool found = false;
            uint32 size = inactive_elements.Size();
            for (uint32 i = 0; i < size; ++i)
            {
                uint32 index = size - i - 1;
                if(location < inactive_elements[index])
                {
                    inactive_elements.AddAt(index, location);
                    found = true;
                    break;
                }
                assert(location != inactive_elements[index]);
            }
            if (!found)
            {
                // TODO: Should these be sorted large to small as well, and then iterate over this list backwards?
                // Could have faster inserts/deletes...
                inactive_elements.AddAt(0, location);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    // Checks to see if the pointer is still pointing to the same data as it expects to
    bool IsValid(RArrayRef<T> ref)
    {
        if(ref.generation != elements[ref.index].generation) return false;
        assert(ref.ptr == &elements[ref.index].data);
        return (ref.ptr == &elements[ref.index].data);
    }

    RArrayRef<T> GetRef(uint32 index)
    {
        if ((elements[active_elements[index]].generation & ~VALID_MASK))
        {
            RArrayRef<T> result;
            result.generation = elements[active_elements[index]].generation;
            result.index = active_elements[index];
            result.ptr = &elements[active_elements[index]].data;
            return result;
        }
        else
        {
            return NullRef();
        }
    }

    T& operator[](uint32 loc)
    {
        assert(loc < active_elements.Size());
        return elements[active_elements[loc]].data;
    }

    RArrayRef<T> NullRef()
    {
        RArrayRef<T> result = {};
        return result;
    }
};



/******************************************************************************
*
*   List.h
*
*
*   By Patrick Wyatt - 5/16/2010
*
***/


/******************************************************************************
*
*   WHAT IT IS
*
*   This module defines a linked-list implementation that uses "embedded"
*   links rather than separately allocated link-nodes as does STL and
*   more or less all other linked-list implementations.
*
*   Why is this cool:
*       1. No additional memory allocations (malloc) required to link
*           an object into a linked list.
*       2. Not necessary to traverse an additional pointer references
*           to get to the object being dereferenced.
*       3. Probably most importantly, when objects get deleted, they
*           automatically unlink themselves from the lists they're
*           linked to, eliminating many common types of bugs.
*
*   HOW TO USE IT
*
*   Declare a structure that will be contained in one or more linked lists:
*       class CFoo {
*           LIST_LINK(CFoo) m_linkByData;
*           LIST_LINK(CFoo) m_linkByType;
*           int             m_data;
*           int             m_type;
*           ...
*       };
*
*   Declare list variables:
*       LIST_DECLARE(CFoo, m_linkByData) listByData;
*       LIST_DECLARE(CFoo, m_linkByType) listByType;
*       LIST_PTR(CFoo) m_listPtr = foo ? &listByData : &listByType;
*
*   Operations on links:
*       T * Prev ();
*       T * Next ();
*       void Unlink ();
*       bool IsLinked () const;
*
*   Operations on lists:
*       bool Empty () const;
*       void UnlinkAll ();
*       void DeleteAll ();
*
*       T * Head ();
*       T * Tail ();
*       T * Prev (T * node);
*       T * Next (T * node);
*
*       void InsertHead (T * node);
*       void InsertTail (T * node);
*       void InsertBefore (T * node, T * before);
*       void InsertAfter (T * node, T * after);
*
*   NOTES
*
*   Limitations:
*       All nodes must be allocated on (at least) two-byte boundaries
*       because the low bit is used as a sentinel to signal end-of-list.
*
*   Thanks to:
*       Something like this code was originally implemented by Mike
*       O'Brien in storm.dll for Diablo in 1995, and again at ArenaNet
*       for Guild Wars.
*
***/


#ifdef LIST_H
#error "Header included more than once"
#endif
#define LIST_H


/******************************************************************************
*
*   List definition macros
*
***/

// Define a linked list:
// T    = type of object being linked
// link = member within object which is the link field
#define LIST_DECLARE(T, link) TListDeclare<T, offsetof(T, link)>

// Define a field within a structure that will be used to link it into a list
#define LIST_LINK(T) TLink<T>

// Define a pointer to a list
#define LIST_PTR(T) TList<T> *


/******************************************************************************
*
*   TLink
*
***/

//=============================================================================
template<class T>
class TLink {
public:
    ~TLink ();
    TLink ();

    bool IsLinked () const;
    void Unlink ();

    T * Prev ();
    T * Next ();
    const T * Prev () const;
    const T * Next () const;

    // For use by list-type classes, not user code;
    // the alternative is to friend TList<T>, THash<T>,
    // and (eventually) many other structures.
    TLink (size_t offset);
    void SetOffset (size_t offset);
    TLink<T> * NextLink ();
    TLink<T> * PrevLink ();
    void InsertBefore (T * node, TLink<T> * nextLink);
    void InsertAfter (T * node, TLink<T> * prevLink);

private:
    T *         m_nextNode; // pointer to the next >object<
    TLink<T> *  m_prevLink; // pointer to the previous >link field<
    void RemoveFromList ();

    // Hide copy-constructor and assignment operator
    TLink (const TLink &);
    TLink & operator= (const TLink &);
};

//=============================================================================
template<class T>
TLink<T>::~TLink () {
    RemoveFromList();
}

//=============================================================================
template<class T>
TLink<T>::TLink () {
    // Mark this node as the end of the list, with no link offset
    m_nextNode = (T *) ((size_t) this + 1 - 0);
    m_prevLink = this;
}

//=============================================================================
template<class T>
TLink<T>::TLink (size_t offset) {
    // Mark this node as the end of the list, with the link offset set
    m_nextNode = (T *) ((size_t) this + 1 - offset);
    m_prevLink = this;
}

//=============================================================================
template<class T>
void TLink<T>::SetOffset (size_t offset) {
    // Mark this node as the end of the list, with the link offset set
    m_nextNode = (T *) ((size_t) this + 1 - offset);
    m_prevLink = this;
}

//=============================================================================
template<class T>
TLink<T> * TLink<T>::NextLink () {
    // Calculate the offset from a node pointer to a link structure
    size_t offset = (size_t) this - ((size_t) m_prevLink->m_nextNode & ~1);

    // Get the link field for the next node
    return (TLink<T> *) (((size_t) m_nextNode & ~1) + offset);
}

//=============================================================================
template<class T>
void TLink<T>::RemoveFromList () {
    NextLink()->m_prevLink = m_prevLink;
    m_prevLink->m_nextNode = m_nextNode;
}

//=============================================================================
template<class T>
void TLink<T>::InsertBefore (T * node, TLink<T> * nextLink) {
    RemoveFromList();

    m_prevLink = nextLink->m_prevLink;
    m_nextNode = m_prevLink->m_nextNode;

    nextLink->m_prevLink->m_nextNode = node;
    nextLink->m_prevLink = this;
}

//=============================================================================
template<class T>
void TLink<T>::InsertAfter (T * node, TLink<T> * prevLink) {
    RemoveFromList();

    m_prevLink = prevLink;
    m_nextNode = prevLink->m_nextNode;

    prevLink->NextLink()->m_prevLink = this;
    prevLink->m_nextNode = node;
}

//=============================================================================
template<class T>
bool TLink<T>::IsLinked () const {
    return m_prevLink != this;
}

//=============================================================================
template<class T>
void TLink<T>::Unlink () {
    RemoveFromList();

    // Mark this node as the end of the list with no link offset
    m_nextNode = (T *) ((size_t) this + 1);
    m_prevLink = this;
}

//=============================================================================
template<class T>
TLink<T> * TLink<T>::PrevLink () {
    return m_prevLink;
}

//=============================================================================
template<class T>
T * TLink<T>::Prev () {
    T * prevNode = m_prevLink->m_prevLink->m_nextNode;
    if ((size_t) prevNode & 1)
        return NULL;
    return prevNode;
}

//=============================================================================
template<class T>
const T * TLink<T>::Prev () const {
    const T * prevNode = m_prevLink->m_prevLink->m_nextNode;
    if ((size_t) prevNode & 1)
        return NULL;
    return prevNode;
}

//=============================================================================
template<class T>
T * TLink<T>::Next () {
    if ((size_t) m_nextNode & 1)
        return NULL;
    return m_nextNode;
}

//=============================================================================
template<class T>
const T * TLink<T>::Next () const {
    if ((size_t) m_nextNode & 1)
        return NULL;
    return m_nextNode;
}


/******************************************************************************
*
*   TList
*
***/

//=============================================================================
template<class T>
class TList {
public:
    ~TList ();
    TList ();

    bool Empty () const;
    void UnlinkAll ();
    void DeleteAll ();

    T * Head ();
    T * Tail ();
    const T * Head () const;
    const T * Tail () const;

    T * Prev (T * node);
    T * Next (T * node);
    const T * Prev (const T * node) const;
    const T * Next (const T * node) const;

    void InsertHead (T * node);
    void InsertTail (T * node);
    void InsertBefore (T * node, T * before);
    void InsertAfter (T * node, T * after);

private:
    TLink<T>    m_link;
    size_t      m_offset;

    TList (size_t offset);
    TLink<T> * GetLinkFromNode (const T * node) const;
    template<class T, size_t offset> friend class TListDeclare;

    // Hide copy-constructor and assignment operator
    TList (const TList &);
    TList & operator= (const TList &);
};

//=============================================================================
template<class T>
TList<T>::~TList () {
    UnlinkAll();
}

//=============================================================================
template<class T>
TList<T>::TList () :
    m_link(),
    m_offset((size_t) -1)
{}

//=============================================================================
template<class T>
TList<T>::TList (size_t offset) :
    m_link(offset),
    m_offset(offset)
{}

//=============================================================================
template<class T>
bool TList<T>::Empty () const {
    return m_link.Next() == NULL;
}

//=============================================================================
template<class T>
void TList<T>::UnlinkAll () {
    for (;;) {
        TLink<T> * link = m_link.PrevLink();
        if (link == &m_link)
            break;
        link->Unlink();
    }
}

//=============================================================================
template<class T>
void TList<T>::DeleteAll () {
    while (T * node = m_link.Next())
        delete node;
}

//=============================================================================
template<class T>
T * TList<T>::Head () {
    return m_link.Next();
}

//=============================================================================
template<class T>
T * TList<T>::Tail () {
    return m_link.Prev();
}

//=============================================================================
template<class T>
const T * TList<T>::Head () const {
    return m_link.Next();
}

//=============================================================================
template<class T>
const T * TList<T>::Tail () const {
    return m_link.Prev();
}

//=============================================================================
template<class T>
T * TList<T>::Prev (T * node) {
    return GetLinkFromNode(node)->Prev();
}

//=============================================================================
template<class T>
const T * TList<T>::Prev (const T * node) const {
    return GetLinkFromNode(node)->Prev();
}

//=============================================================================
template<class T>
T * TList<T>::Next (T * node) {
    return GetLinkFromNode(node)->Next();
}

//=============================================================================
template<class T>
const T * TList<T>::Next (const T * node) const {
    return GetLinkFromNode(node)->Next();
}

//=============================================================================
template<class T>
void TList<T>::InsertHead (T * node) {
    InsertAfter(node, NULL);
}

//=============================================================================
template<class T>
void TList<T>::InsertTail (T * node) {
    InsertBefore(node, NULL);
}

//=============================================================================
template<class T>
void TList<T>::InsertBefore (T * node, T * before) {
    ASSERT(!((size_t) node & 1));
    GetLinkFromNode(node)->InsertBefore(
        node,
        before ? GetLinkFromNode(before) : &m_link
    );
}

//=============================================================================
template<class T>
void TList<T>::InsertAfter (T * node, T * after) {
    ASSERT(!((size_t) node & 1));
    GetLinkFromNode(node)->InsertAfter(
        node,
        after ? GetLinkFromNode(after) : &m_link
    );
}

//=============================================================================
template<class T>
TLink<T> * TList<T>::GetLinkFromNode (const T * node) const {
    ASSERT(m_offset != (size_t) -1);
    return (TLink<T> *) ((size_t) node + m_offset);
}

/******************************************************************************
*
*   TListDeclare - declare a list with a known link offset
*
***/

//=============================================================================
template<class T, size_t offset>
class TListDeclare : public TList<T> {
public:
    TListDeclare ();
};

//=============================================================================
template<class T, size_t offset>
TListDeclare<T, offset>::TListDeclare () : TList<T>(offset)
{}


//===================================
// MIT License
//
// Copyright (c) 2010 by Patrick Wyatt
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//===================================

