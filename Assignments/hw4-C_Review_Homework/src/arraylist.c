/**
 * Name: Anthony Wong
 * GTID: 903579250
 */

/*  PART 2: A CS-2200 C implementation of the arraylist data structure.
    Implement an array list.
    The methods that are required are all described in the header file. Description for the methods can be found there.

    Hint 1: Review documentation/ man page for malloc, calloc, and realloc.
    Hint 2: Review how an arraylist works.
    Hint 3: You can use GDB if your implentation causes segmentation faults.
*/

#include "arraylist.h"

/* Student code goes below this point */
arraylist_t *create_arraylist(uint initial_capacity)
{
    arraylist_t *arraylist = malloc(sizeof(arraylist_t));
    if (!arraylist) return NULL;
    if ((arraylist->backing_array = malloc(initial_capacity * sizeof(char*))) == NULL)
    {
        free(arraylist);
        return NULL;
    }
    arraylist->capacity = initial_capacity;
    arraylist->size = 0;
    return arraylist;
}

void add_at_index(arraylist_t *arraylist, char *data, int index)
{
    if (index < 0 || index > arraylist->size)
    {
        return;
    }
    if (arraylist->size == arraylist->capacity)
    {
        resize(arraylist);
    }
    for (int i = arraylist->size; i > index; i--)
    {
        arraylist->backing_array[i] = arraylist->backing_array[i - 1];
    }
    arraylist->size++;
    arraylist->backing_array[index] = data;
}

void append(arraylist_t *arraylist, char *data)
{
    if (arraylist->size == arraylist->capacity)
    {
        resize(arraylist);
    }
    int end = arraylist->size;
    arraylist->size++;
    arraylist->backing_array[end] = data;
}

char *remove_from_index(arraylist_t *arraylist, int index)
{
    if (index < 0 || index >= arraylist->size)
    {
        return NULL;
    }
    char *remove = arraylist->backing_array[index];
    for (int i = index; i < arraylist->size - 1; i++)
    {
        arraylist->backing_array[i] = arraylist->backing_array[i + 1];
    }
    arraylist->backing_array[arraylist->size - 1] = 0;
    free(arraylist->backing_array[arraylist->size - 1]);
    arraylist->size--;
    return remove;
}

void resize(arraylist_t *arraylist)
{
    arraylist->capacity *= 2;
    if ((arraylist->backing_array = realloc(arraylist->backing_array, arraylist->capacity * sizeof(char*))) == NULL)
    {
        return;
    }
}

void destroy(arraylist_t *arraylist)
{
    free(arraylist->backing_array);
}
