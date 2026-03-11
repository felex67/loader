#include "ByteBuffer.h"

#include <stdlib.h>
#include <string.h>

#ifndef NULL
    #define NULL ((void*)0U)
#endif // !NULL

typedef struct BYTEBUFFER_INSTANCE {
    void (*const destruct)(LPBYTEBUFFER _Array);
    int (*const init)(LPBYTEBUFFER _Array, size_t _Size);
    int (*const resize)(LPBYTEBUFFER _Array, size_t _NewSize);
    void (*const release)(LPBYTEBUFFER _Array, void** _Destination, size_t* _DestSize);
    void (*const assign)(LPBYTEBUFFER _Dest, void* _Src, size_t _SrcSize);
    int (*const clone)(LPBYTEBUFFER _Dest, const LPBYTEBUFFER _Src);
    void (*const swap)(LPBYTEBUFFER _Left, LPBYTEBUFFER _Right);
    u_int8_t* data;
    size_t size;
    /* reserverd region sizeof(void*) * 4 */
    size_t rsize;   // real size
    size_t isstatic;
    size_t reserved[2];
} BYTEBUFFER_INSTANCE, *LPBYTEBUFFER_INSTANCE;

void buffer_destruct(LPBYTEBUFFER _Buff);
int buffer_init(LPBYTEBUFFER _Buff, size_t _Size);
int buffer_resize(LPBYTEBUFFER _Buff, size_t _NewSize);
void buffer_release(LPBYTEBUFFER _Buff, void** _Dest, size_t* _DestSize);
void buffer_assign(LPBYTEBUFFER _Dest, void* _Src, size_t _SrcSize);
int buffer_clone(LPBYTEBUFFER _Dest, const LPBYTEBUFFER _Src);
void buffer_swap(LPBYTEBUFFER _Left, LPBYTEBUFFER _Right);

BYTEBUFFER_INSTANCE BYTEBUFFER_INSTANCE_INIT = {
    .destruct = buffer_destruct,
    .init = buffer_init,
    .resize = buffer_resize,
    .release = buffer_release,
    .assign = buffer_assign,
    .clone = buffer_clone,
    .swap = buffer_swap,
    .data = NULL,
    .size = 0,
    .rsize = 0,
    .isstatic = 0,
    .reserved = { 0UL, 0UL }
};

LPBYTEBUFFER new_ByteBuffer(size_t _Size) {
    LPBYTEBUFFER_INSTANCE r = malloc(sizeof(BYTEBUFFER_INSTANCE));
    if (r) {
        if (NULL != (r->data = (u_int8_t*)malloc(_Size))) {
            memcpy(r, &BYTEBUFFER_INSTANCE_INIT, sizeof(BYTEBUFFER_INSTANCE_INIT));
            memset(r->data, 0, _Size);
            r->size = r->rsize = _Size;
        }
        else {
            free(r);
        }
    }
    return (LPBYTEBUFFER)r;
}

void buffer_destruct(LPBYTEBUFFER _Buff) {
    LPBYTEBUFFER_INSTANCE b = (LPBYTEBUFFER_INSTANCE)_Buff;
    if (b->data) free(b->data);
    if (!b->isstatic) free(b);
}

int buffer_init(LPBYTEBUFFER _Buff, size_t _Size) {
    LPBYTEBUFFER_INSTANCE b = (LPBYTEBUFFER_INSTANCE)_Buff;
    int res = FALSE;
    memcpy(b, &BYTEBUFFER_INSTANCE_INIT, sizeof(BYTEBUFFER_INSTANCE_INIT));
    if (NULL != (b->data = (u_int8_t*)malloc(_Size))) {
        memset(b->data, 0, _Size);
        b->rsize = b->size = _Size;
        b->isstatic = 1;
        res = TRUE;
    }
    return res;
}

int buffer_resize(LPBYTEBUFFER _Buff, size_t _NewSize) {
    LPBYTEBUFFER_INSTANCE b = (LPBYTEBUFFER_INSTANCE)_Buff;
    u_int8_t* t = (u_int8_t*)malloc(_NewSize);
    size_t cpsize = _NewSize < b->size ? _NewSize : b->size;
    if (NULL != t) {
        if (b->data) {
            if (cpsize) {
                memcpy(t, b->data, cpsize);
                if (cpsize < _NewSize)
                    memset(t + cpsize, 0, _NewSize - cpsize);
            }
            free(b->data);
        }
        b->data = t;
        b->rsize = b->size = _NewSize;
    }
    return t ? TRUE : FALSE;
}

void buffer_release(LPBYTEBUFFER _Src, void** _Dest, size_t* _DestSize) {
    LPBYTEBUFFER_INSTANCE b = (LPBYTEBUFFER_INSTANCE)_Src;
    *_Dest = b->data;
    *_DestSize = b->rsize;
    b->rsize = b->size = 0;
    b->data = NULL;
}

void buffer_assign(LPBYTEBUFFER _Dest, void* _Src, size_t _SrcSize) {
    LPBYTEBUFFER_INSTANCE b = (LPBYTEBUFFER_INSTANCE)_Dest;
    b->data = _Src;
    b->rsize = b->size = _SrcSize;
}

int buffer_clone(LPBYTEBUFFER _Dest, LPBYTEBUFFER _Src) {
    LPBYTEBUFFER_INSTANCE
        d = (LPBYTEBUFFER_INSTANCE)_Dest,
        s = (LPBYTEBUFFER_INSTANCE)_Src;
    u_int8_t* t = (u_int8_t*)malloc(s->rsize);
    int res = FALSE;
    if (t) {
        if (d->data) free(d->data);
        memcpy(t, s->data, s->rsize);
        d->data = t;
        d->size = s->size;
        d->rsize = s->rsize;
        res = TRUE;
    }
    return res;
}

void buffer_swap(LPBYTEBUFFER _Left, LPBYTEBUFFER _Right) {
    BYTEBUFFER_INSTANCE t,
        *l = (LPBYTEBUFFER_INSTANCE)_Left,
        *r = (LPBYTEBUFFER_INSTANCE)_Right;
    t.data = l->data;
    t.size = l->size;
    t.rsize = l->rsize;
    l->data = r->data;
    l->size = r->size;
    l->rsize = r->rsize;
    r->data = t.data;
    r->size = t.size;
    r->rsize = t.rsize;
}