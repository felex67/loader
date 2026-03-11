#ifndef _BYTEBUFFER_H_
#define  _BYTEBUFFER_H_ 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#ifndef TRUE
    #define TRUE ((int)1U)
#endif // !TRUE
#ifndef FALSE
    #define FALSE ((int)0U)
#endif // !FALSE


    typedef struct BYTEBUFFER BYTEBUFFER, *LPBYTEBUFFER;

    struct BYTEBUFFER {
        /**
         * Деструктор
         * @param _Array Указатель на динамический массив
         * @returns void
         * */
        void (*const destruct)(LPBYTEBUFFER _Array);
        /**
         * Инициализатор. Выделяет память заданного размера
         * @param _Array Указатель на массив
         * @param _Size Размер в байтах
         * @returns int(TRUE) - при успешной инициализации, в противном случае - int(FALSE)
         */
        int (*const init)(LPBYTEBUFFER _Array, size_t _Size);
        /**
         * Устанавливает новый размер буффера
         * @param _Array
         * @param _NewSize
         * @returns int(TRUE) - при успехе, в противном случае - int(FALSE)
         * */
        int (*const resize)(LPBYTEBUFFER _Array, size_t _NewSize);
        /**
         * Высвобождает указатель на выделенную память. В дальнейшем следует использовать
         * free() на высвобожденом указателе для высвобождения ресурсов
         * @param _Array Указатель на объект массива
         * @param _Dest Указатель на переменную в которую сохраняется указатель на выделенную память
         * @param _DestSize Указатель на переменную типа size_t для сохранения размера выделенной памяти в байтах
         * @returns void
         * */
        void (*const release)(LPBYTEBUFFER _Array, void** _Destination, size_t* _DestSize);
        /**
         * Захватывает указатель на выделенную память
         * @param _Dest Указатель на объект массива
         * @param _Src Указатель на выделенную память
         * @param _SrcSize Размер выделенной памяти в байтах
         * @returns void
         */
        void (*const assign)(LPBYTEBUFFER _Dest, void* _Src, size_t _SrcSize);
        /**
         * Полностью копирует _Src с выделением памяти
         * @param _Dest Указатель на целевой объект массива
         * @param _Src Указатель на исходный объект массива
         * @returns int(TRUE) - в случае успеха, в противном случае - int(FALSE)
         */
        int (*const clone)(LPBYTEBUFFER _Dest, const LPBYTEBUFFER _Src);
        /**
         * Меняет местами два массива
         * @param _Left
         * @param _Right
         * @returns void
         */
        void (*const swap)(LPBYTEBUFFER _Left, LPBYTEBUFFER _Right);
        /** Указатель на выделенну память */
        u_int8_t *const data;
        /** Используемый размер массива */
        const size_t size;
        /** Зарезервированно под служебные нужды */
        void *const zero[4];
    };

    LPBYTEBUFFER new_ByteBuffer(size_t _BuffSize);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !_BYTEBUFFER_H_