#ifndef __byte_buffer_h__
#define  __byte_buffer_h__ 1
#include <sys/types.h>

typedef struct __byte_buffer {
    /** Уничтожает объект 'Inst', освобождает память */
    void (*const destruct)(struct __byte_buffer *Inst);
    void (*const reset)(struct __byte_buffer *Inst);
    /** Инициализирует буффер */
    int (*const init)(struct __byte_buffer *Inst, const size_t Size);
    /** Изменяет размер буффера.
     * При увеличении размера сначала выделяется память, затем
     * производится её копирование в новый массив */
    int (*const resize)(struct __byte_buffer *Inst, const size_t NewSize);
    /** Передаёт массив в адрес Tgt, а размер в Size.
     * Для предотвращения утечек памяти по завершению работы с ней
     * необходимо вызвать функцию 'free(Tgt)' */
    int (*const release)(struct __byte_buffer *Inst, void ** Tgt, size_t *Size);
    /** Захватывает массив 'Bytes'.
     * при вызове деструктора память занятая 'Bytes' будет
     * освобождена вызовом функции 'free(Bytes)' */
    int (*const assign)(struct __byte_buffer *Inst, void* Bytes, const size_t Size);
    /** Создаёт по адресу 'Dest' точную копию 'Source'.
     * !!! Метод освобождает память в Dest !!!
     * Если ранне объекту 'Dest' была выделена память методами 'Dest.init()'
     * или 'Dest.assign()', то для продолжения работы с ней перед вызовом
     * 'Dest.clone()' необходимо вызвать метод 'Dest.release()',
     * в противном случае может возникнуть ошибка 'SIGSEGV'.
     * В случае ошибки 'Dest' останется не тронутым. */
    int (*const clone)(struct __byte_buffer *Dest, const struct __byte_buffer *Source);
    int (*const swap)(struct __byte_buffer *Left, struct __byte_buffer *Right);
    
    /** Массив байтов */
    unsigned char *bytes;
    /** Актуальный размер массива */
    const size_t size;
    /** Реальный размер массива */
    const size_t __private_rsz;
    const size_t __private_flags;
} bytebuffer_t;

/** Коеструктор класса буффер.
 * При вызове с параметром 'Tgt = 0' выделяет динамическую память,
 * в противном случае - инициализирует объект по указателю */
bytebuffer_t* new_byte_buffer(bytebuffer_t* Tgt);
/** Коеструктор класса буффер.
 * создаёт точную копию 'Src' и размещает её в динамической
 * памяти. Если память выделять не требуется, то стоит
 * воспользоваться методом 'bytebuffer_t::clone()' */
bytebuffer_t* new_byte_buffer_clone(bytebuffer_t *Src);

#ifdef __byte_buffer_c__
    enum enum_byte_buffer_flags {
        BYTEBUFF_FLAG_DYNINST = 0b1
    };
    int byte_buffer_init(bytebuffer_t *Inst, const size_t Size);
    void byte_buffer_destruct(bytebuffer_t *Inst);
    void byte_buffer_reset(bytebuffer_t *Inst);
    int byte_buffer_resize(bytebuffer_t *Inst, const size_t NewSize);
    int byte_buffer_release(bytebuffer_t *Inst, void **Tgt, size_t *Size);
    int byte_buffer_assign(bytebuffer_t *Inst, void* Bytes, const size_t Size);
    int byte_buffer_copy(bytebuffer_t *Dest, const bytebuffer_t *Src);
    int byte_buffer_swap(bytebuffer_t *Left, bytebuffer_t *Right);
#endif // __byte_buffer_c__

#endif // __byte_buffer_h__