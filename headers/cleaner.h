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

/** class __config_cleaner_errors */

    /** Флаги состояния загрузчика */
    typedef struct __config_cleaner_flags {
        // Выделена динамическая память
        u_int32_t DYNINST : 1;
        // Массивы инициализированы
        u_int32_t BUFINIT : 1;
        // Файл очищен
        u_int32_t FILECLR : 1;
    } config_cleaner_flags_t;

    /* Обработчик ошибок исполнения процесса загрузки */
    struct __config_cleaner_errors {
        // массив для обработки ошибок
        char Buff[2048];
        // номер ошибки
        int error;
        // Флаги состояния (union)
        struct __config_cleaner_flags unFl;
    };

    /* Структура-инициализатор обработчика ошибок */
    extern struct __config_cleaner_errors __config_cleaner_init_struct_errors;
    
    // выводит отформатированные ошибки в std::err
    void config_cleaner_perror(struct __config_cleaner *I, const char * What);

#ifdef __config_cleaner_c__
    // Инициализатор Структуры массивов
    extern struct __config_cleaner_buffers __config_cleaner_init_struct_buffers;
    
    /**Выделяет память рабочему: Wrk и входному: In массивам,
     * затем загружает файл во входной массив: In */
    int config_cleaner_init_buffers(struct __config_cleaner *CL, const char *FileName);
    /**Деструктор структуры массивов:
     * освобождает раннее выделенную память */
    void config_cleaner_destruct_buffers(struct __config_cleaner *Inst);
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
    int config_cleaner_construct_charset(struct __config_cleaner_charset *CS);
#endif // __config_cleaner_c__

/* class config_cleaner */
    typedef struct __config_cleaner {
        /* Деструктор загрузчика */
        void (*destruct)(struct __config_cleaner *Inst);
        // Очищает буферы, устанавливает значения по умолчанию
        void (*reset)(struct __config_cleaner* Inst);
        // Загружает и отчищает файл от коментариев, удаляет пустые строки
        int (*load)(struct __config_cleaner *Inst, const char *FileName);
        // устанавливает типы строчных комментариев
        int (*set_string_comments)(struct __config_cleaner *I, const char* Keys);
        // очищает буферы
        void (*clear)(struct __config_cleaner *I);
        // Выводит информацию об ошибке в std::cerr
        void (*perror)(struct __config_cleaner *Inst, const char *What);
        

        /* Атрибуты, или поля, или как там ещё =D */
        // Структура массивов
        bytebuffer_t *__private_in;
        bytebuffer_t *__private_wrk;

        // Набор символов
        struct __config_cleaner_charset Charset;
        // Обработчик ошибок
        struct __config_cleaner_errors Errors;
        // Выходная структура группированных переменных
        //struct __config_cleaner_var_groups Config;
        // Указатель на входной массив
        const char * const *Result;
        // Указатель на "полезный" размер входного массива
        const size_t *const Size;
    } ConfigCleaner;
    /* конструктор загрузчика конфигурационных файлов */
    ConfigCleaner* config_cleaner_construct(void* ptr);
    
#ifdef __config_cleaner_c__
/* config_cleaner */
        extern ConfigCleaner __config_cleaner_init_struct;
        /* Деструктор загрузчика конфигурационных файлов */
        void config_cleaner_destruct(ConfigCleaner *Inst);
        // Очищает массивы, устанавливает значения по умолчанию
        void config_cleaner_reset(ConfigCleaner *Inst);
        // Main method of config_cleaner, loads & processes config file
        int config_cleaner_process_file(ConfigCleaner *CL, const char *FileName);
        // set user comment types
        int config_cleaner_set_scomments(ConfigCleaner *I, const char *Keys);
        // cleans the file from comments and spaces
        int config_cleaner_clean_file(ConfigCleaner *I);
        // 1 - если успешно загружен, 0 - в случае ошибки
        int config_cleaner_isInit(ConfigCleaner *Inst);
        // удаляет пустые строки
        int config_cleaner_delete_emty_strings(struct __config_cleaner_buffers *I);

/* Buffers */
        // инициализирует буфферы и считывает в них файл
        int config_cleaner_init_buffers(struct __config_cleaner *CL, const char *FileName);
        // меняет буфферы местами
        void config_cleaner_swap_buffers(struct __config_cleaner_buffers *B);
/* Charset */
        // sort characters
        int config_cleaner_sort_charset(struct __config_cleaner_charset *CS);
        
/* Other */
        // сортирует строку по возрастанию
        int config_cleaner_strsort_az(char *a);
        // сортирует строку по убыванию
        int config_cleaner_strsort_za(char *a);
        // разворачивает строку
        int config_cleaner_strinverse(char *a);
#endif //__config_cleaner_c__

#endif // __config_cleaner_h__