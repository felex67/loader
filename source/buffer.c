#define  __byte_buffer_c__ 1

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "headers/buffer.h"

#ifndef nullptr
    #define nullptr (0)
#endif // nullptr

    bytebuffer_t* byte_buffer_construct () {
        bytebuffer_t *Inst = (bytebuffer_t*)malloc(sizeof(bytebuffer_t));
        if (Inst) {
            *((void**)&Inst->destruct) = (void*)byte_buffer_destruct;
            *((int**)&Inst->init) = (int*)byte_buffer_init;
            *((int**)&Inst->resize) = (int*)byte_buffer_resize;
            *((void**)&Inst->release) = (void*)byte_buffer_release;
            *((int**)&Inst->assign) = (int*)byte_buffer_assign;
            *((int**)&Inst->clone) = (int*)byte_buffer_assign;

            *((void**)&Inst->bytes) = nullptr;
            *((size_t*)&Inst->size) = 0;
            *((size_t*)&Inst->__private_rsz) = 0;
        }
        return Inst;
    }
    bytebuffer_t* byte_buffer_clone(bytebuffer_t *Src) {
        bytebuffer_t *Clone = nullptr;
        errno = EINVAL;
        if (Src) {
            if (nullptr != (Clone = byte_buffer_construct())) {
                if (-1 != Clone->clone(Clone, Src)) {
                    errno = EXIT_SUCCESS;
                }
            }
        }
        return Clone;
    }
    void byte_buffer_destruct (bytebuffer_t *Inst) {
        if (Inst && Inst->bytes) { free(*((void**)&Inst->bytes)); }
    }
    int byte_buffer_init (bytebuffer_t *I, const size_t S) {
        void *t = nullptr;
        int result = -1;
        if (I && S) {
            if (nullptr != (t = malloc(S))) {
                *((void**)&I->bytes) = t;
                errno = result = EXIT_SUCCESS;
            }
        }
        else { errno = EINVAL; }
        return result;
    }
    int byte_buffer_resize (bytebuffer_t *Inst, const size_t NewSize) {
        void *t = nullptr;
        int result = -1;
        if (Inst && NewSize && Inst->bytes) {
            if (NewSize > Inst->__private_rsz) {
                if (nullptr != (t = malloc(NewSize))) {
                    memcpy(t, (const void*)Inst->bytes, Inst->size);
                    free(*((void**)&Inst->bytes));
                    *((void**)&Inst->bytes) = t;
                    *((size_t*)&Inst->size) = *((size_t*)&Inst->__private_rsz) = NewSize;
                    errno = result = EXIT_SUCCESS;
                }
            }
            else {
                *((size_t*)&Inst->size) = NewSize;
                errno = result = EXIT_SUCCESS;
            }
        }
        else { errno = EINVAL; }
        return result;
    }
    int byte_buffer_release(bytebuffer_t *Inst, void **Tgt, size_t *Size) {
        int result = -1;
        if (Inst && Tgt && Size) {
            *Tgt = *((void**)&Inst->bytes);
            *Size = Inst->__private_rsz;
            *((void**)&Inst->bytes) = nullptr;
            *((size_t*)&Inst->__private_rsz) = *((size_t*)&Inst->size) = 0;
            errno = result = EXIT_SUCCESS;
        }
        else { errno = EINVAL; }
        return result;
    }
    int byte_buffer_assign(bytebuffer_t *Inst, void* Bytes, const size_t Size) {
        int result = -1;
        if (Inst && Bytes && Size) {
            *((void**)&Inst->bytes) = Bytes;
            *((size_t*)&Inst->size) = *((size_t*)&Inst->__private_rsz) = Size;
            errno = result = EXIT_SUCCESS;
        }
        else { errno = EINVAL; }
        return result;
    }
    int byte_buffer_copy(bytebuffer_t *Tgt, bytebuffer_t *Src) {
        void* t = nullptr;
        int result = -1;
        if (Tgt && Src) {
            if (nullptr != (t = malloc(Src->__private_rsz))) {
                if (nullptr != Tgt->bytes) Tgt->destruct(Tgt);
                memcpy(t, (const void*)Src->bytes, *(size_t*)&Src->__private_rsz);
                *((void**)&Tgt->bytes) = t;
                *(size_t*)&Tgt->size = Src->size;
                *(size_t*)&Tgt->__private_rsz = Src->__private_rsz;
                errno = result = EXIT_SUCCESS;
            }
        }
        else { errno = EINVAL; }
        return result;
    }
