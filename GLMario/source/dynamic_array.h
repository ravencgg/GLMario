#pragma once
#include <assert.h>
#include "types.h"



// Dynamic array implementation
template<class T>
class Array
{
public:
    typedef int (*CompareFunction)(const void* left, const void* right);

    CompareFunction arrayCompareFunction;

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
        return this->Get(loc);
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

    T& GetBack()
    {
        return data[write_pos - 1];
    }

    void Remove(uint32 position)
    {
        assert(position < write_pos);

        //TODO(chris): Replace this with a memmove call!
        for (uint32 i = position; i < write_pos; ++i)
        {
            data[i] = data[i + 1];
        }

        --write_pos;
    }

    int32 Find(T element)
    {
        for(uint32 i = 0; i < Size(); ++i)
        {
            if(data[i] == element)
            {
                return i;
            }
        }
        return -1;
    }


    // NOTE: does not clear, only removes access
    void RemoveBack()
    {
        --write_pos;
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

    void SetComparisonFunction(CompareFunction acf)
    {
        this->arrayCompareFunction = acf;
    }

	uint32 Size()
	{
		return write_pos;
	}

    bool Sort(CompareFunction acf)
    {
        if (acf)
        {
            std::qsort(data, Size(), sizeof(T), acf);
            return true;
        }
        return false;
    }

    bool Sort()
    {
        if (arrayCompareFunction)
        {
            std::qsort(data, Size(), sizeof(T), acf);
            return true;
        }
        return false;
    }

    //bool SortDescending(CompareFunction acf)
    //{
    //    if (acf)
    //    {
    //        std::qsort(data, write_pos - 1, sizeof(T), -acf);
    //        return true;
    //    }
    //    return false;
    //}

    //bool SortDescending()
    //{
    //    if (arrayCompareFunction)
    //    {
    //        std::qsort(data, write_pos - 1, sizeof(T), arrayCompareFunction);
    //        return true;
    //    }
    //    return false;
    //}

private:

	void reallocate()
	{
		T* temp = data;
		uint32 old_size = capacity;
		capacity *= 2;
		data = new T[capacity] {};

		//TODO(chris): Replace this with a memcopy call!
		for(uint32 i = 0; i < old_size; ++i)
		{
			data[i] = temp[i];
		}

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
    uint16 index;
    uint16 generation;

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
};

template<typename T>
struct Element
{
    T data;
    uint16 index; // Pointless, but matches the RArrayRef
    uint16 generation;
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
        elements = new Element<T>[MAX];
        memset(elements, 0, sizeof(elements[0]) * MAX);
        for (int i = MAX - 1; i >= 0; --i)
        {
            inactive_elements.AddBack(i);
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
            destination->index = location;
            destination->data = element;

            result.index = destination->index;
            result.generation = destination->generation;
            result.ptr = &destination->data;// &elements[location].data;

            return result;
        }

        assert(!"RArray is full!");
        return result;
    }

    bool Remove(RArrayRef<T> ref)
    {
        int32 location = ref.index;
        if (location >= 0)
        {
            //elements[location].id = 0;
            ++elements[location].generation; // Increment the generation on removal to invalidate
                                             // references to this location

            int active_loc = active_elements.Find(location);
            assert(active_loc >= 0);

            active_elements.Remove(active_loc);

            bool found = false;
            uint32 size = inactive_elements.Size();
            for (uint32 i = 0; i < size; ++i)
            {
                uint32 index = size - i;
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
    bool Valid(RArrayRef<T> ref)
    {
        if(ref.generation != elements[ref.index].generation) return false;
        assert(ref.ptr == &elements[ref.index].data);
        return (ref.ptr == &elements[ref.index].data);
    }

    // TODO: have the bottom digits of the ID be the location in the array?!
    //
    // NOTE: Redundant now that there are indexes on each element
//    int32 FindLocation(RArrayRef<T> ref)
//    {
//        for(uint32 i = 0; i < active_elements.Size(); ++i)
//        {
//            if (elements[active_elements[i]].id == ref.id )
//            {
//                if (elements[active_elements[i]].data == *ref.ptr)
//                {
//                    return i;
//                }
//            }
//        }
//        return -1;
//    }

    T& operator[](uint32 loc)
    {
        assert(loc < active_elements.Size());
        return elements[active_elements[i]].data;
    }

    RArrayRef<T> NullRef()
    {
        RArrayRef<T> result = {};
        return result;
    }
};
