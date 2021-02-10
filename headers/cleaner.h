/**
 * Загрузчик версии 2. Расчитан на динамическую компановку,
 * для статической компановки необходимр убрать макрос
 * #ifdef __config_cleaner_c__
 * 
*/

#ifndef __config_cleaner_h__
#define __config_cleaner_h__ 1

#include <sys/types.h>

#include "headers/buffer.h"
/** class __config_cleaner_buffers */
    
    /** Рабочий и выходной массивы */
    struct __config_cleaner_buffers {
        void (*const destruct)(struct __config_cleaner_buffers *Buffs);
        int (*const init)(struct __config_cleaner_buffers *Buffs, const size_t Size);
        void (*const reset)(struct __config_cleaner_buffers *Buffs);
        int (*const swap)(struct __config_cleaner_buffers *Buffs);

        bytebuffer_t In;
        bytebuffer_t Wrk;
    };

#ifdef __config_cleaner_c__
    int config_cleaner_buffers_construct(struct __config_cleaner_buffers *Buffs);
    /** Деструктор структуры массивов:
     * освобождает раннее выделенную память */
    void config_cleaner_buffers_destruct(struct __config_cleaner_buffers *Buffs);
    int config_cleaner_buffers_init(struct __config_cleaner_buffers *Buffs, const size_t Size);
    void config_cleaner_buffers_reset(struct __config_cleaner_buffers *Buffs);
    int config_cleaner_buffers_swap(struct __config_cleaner_buffers *Buffs);
#endif // __config_cleaner_c__

/** class __config_cleaner_charset */

    /** Набор символов необходимых для интерпритации файла*/
    struct __config_cleaner_charset {
        // Символы - не пробелы
        const char NotSpace[8];
        // Строковые коментарии
        const char LineComm[8];
        // Ковычки
        const char Quotes[8];
        // Наибольшая ковычка
        const char gQ;
        // Наибольший коментарий
        const char gC;
    };
#ifdef __config_cleaner_c__

    /** Строка ковычек.
     * По умолчанию: ` ' " */
    extern const char __config_cleaner_default_quotes[];
    
    /** Строка символов обозначающих начало строкового комментария.
     * По умолчанию: # ; */
    extern const char __config_cleaner_default_lcomm[];
    
    /** Строка символо "не пробелов".
     * По умолчанию: /t /n */
    extern const char __config_cleaner_default_nonspace[];
    
    /**Строка определяющая начало группы переменных.
     * По умолчанию: [ */
    extern const char __config_cleaner_default_group[];

    /* Функция-инициализатор набора символов */
    int config_cleaner_charset_construct(struct __config_cleaner_charset *CS);
#endif // __config_cleaner_c__

/* class config_cleaner */
    typedef struct __config_cleaner {
        /* Деструктор загрузчика */
        void (*const destruct)(struct __config_cleaner *Inst);
        // Очищает буферы, устанавливает значения по умолчанию
        void (*const reset)(struct __config_cleaner* Inst);
        // очищает буферы
        void (*const resetBuff)(struct __config_cleaner *I);
        // Загружает и отчищает файл от коментариев, удаляет пустые строки
        int (*const load)(struct __config_cleaner *Inst, const char *FileName);
        // устанавливает типы строчных комментариев
        int (*const set_string_comments)(struct __config_cleaner *I, const char* Keys);
        // Отпускает выходной массив
        int (*const release)(struct __config_cleaner *Inst, void **Dest, size_t *DestSz);

        /* Атрибуты, или поля, или как там ещё =D */

        // Структура массивов
        const struct __config_cleaner_buffers __private_buffs;
        // Набор символов
        const struct __config_cleaner_charset __private_char;
        const size_t __private_flags;
    } ConfigCleaner;
    
    /* конструктор загрузчика конфигурационных файлов */
    ConfigCleaner* new_config_cleaner(ConfigCleaner* Inst);
    
#ifdef __config_cleaner_c__
    enum enum_config_cleaner_flags {
        CONFCLNR_FLAGS_DYNINST = 0b1
    };
/* config_cleaner */
        /* Деструктор загрузчика конфигурационных файлов */
        void config_cleaner_destruct(ConfigCleaner *Inst);
        // Очищает массивы, устанавливает значения по умолчанию
        void config_cleaner_reset(ConfigCleaner *Inst);
        // Очищает массивы
        void config_cleaner_reset_buffers(ConfigCleaner *Inst);
        // Загружает файл, очищает от коментариев и удаляет пустые строки
        int config_cleaner_process_file(ConfigCleaner *CL, const char *FileName);
        // set user comment types
        int config_cleaner_set_scomments(ConfigCleaner *I, const char *Keys);
        // cleans the file from comments and spaces
        int config_cleaner_clean_file(ConfigCleaner *I);
        // удаляет пустые строки
        int config_cleaner_delete_emty_strings(struct __config_cleaner *I);
        // Отпускает выходной массив
        int config_cleaner_release(ConfigCleaner *Inst, void **Dest, size_t *DestSize);

/* Charset */
        // sort characters
        int config_cleaner_charset_sort(struct __config_cleaner_charset *CS);
        
/* Other */
        // сортирует строку по возрастанию
        int config_cleaner_strsort_az(char *a);
        // сортирует строку по убыванию
        int config_cleaner_strsort_za(char *a);
        // разворачивает строку
        int config_cleaner_strinverse(char *a);
#endif //__config_cleaner_c__

#endif // __config_cleaner_h__