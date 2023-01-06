#include "CircularBuffer.h"
#include "../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/crtdbg.h"
#include "../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_malloc.h"
#include <vcruntime_string.h>

void cb_init(circular_buffer* cb, size_t capacity, size_t sz)
{
    cb->buffer = malloc(capacity * sz);
    if (cb->buffer == NULL)
        // handle error
        cb->buffer_end = (char*)cb->buffer + capacity * sz;
    cb->capacity = capacity;
    cb->count = 0;
    cb->sz = sz;
    cb->head = cb->buffer;
    cb->tail = cb->buffer;
}

void cb_free(circular_buffer* cb)
{
    free(cb->buffer);
    // clear out other fields too, just to be safe
}


void cb_push(circular_buffer* cb, const void* item)
{
    if (cb->count == cb->capacity) {
        // handle error
    }
    memcpy(cb->head, item, cb->sz);
    cb->head = (char*)cb->head + cb->sz;
    if (cb->head == cb->buffer_end)
        cb->head = cb->buffer;
    cb->count++;
}

void cb_pop(circular_buffer* cb, void* item)
{
    if (cb->count == 0) {
        // handle error
    }
    memcpy(item, cb->tail, cb->sz);
    cb->tail = (char*)cb->tail + cb->sz;
    if (cb->tail == cb->buffer_end)
        cb->tail = cb->buffer;
    cb->count--;
}
