/**
 * Загрузчик версии 2. Расчитан на динамическую компановку,
 * для статической компановки необходимр убрать макрос
 * #ifdef __config_loader_c__
*/

#ifndef __config_loader_h__
#define __config_loader_h__ 1

#include <sys/types.h>

    typedef struct __config_var {
        char *Var;
        char *Val;
    } cfgvar_t;
    typedef struct __config_group {
        char *Name;
        size_t count;
        cfgvar_t *Vars;
        void (*destruct)(struct __config_group*);
    } cfggrp_t;
    struct Config_Groups {
        size_t count;
        cfggrp_t **Groups;
        void (*destruct)(struct Config_Groups*);
    };

    struct __loader_flags {
        u_int32_t DynInst : 1;
        u_int32_t Init : 1;
    };
    
    typedef union __loader_unFlags {
        u_int32_t ui32;
        struct __loader_flags s_flags;
    } cfgflags_t;

    struct __loader_errors {
        // массив для обработки ошибок
        char Buff[2048];
        // номер ошибки
        int error;
        cfgflags_t unFl;
    };
    
    struct __loader_buffers {
        // входной массив
        unsigned char *In;
        // рабочий массив
        unsigned char *Wrk;
        //размер входного массива
        size_t iSz;
        // позиция во входном массиве
        size_t Pi;
        // позиция в рабочем массиве
        size_t Pw;
    };

    struct __loader_charset {
        // нужные символы - не пробелы(\t\n)
        const char NotSpace[8];
        // строковые коментарии
        const char LineComm[8];
        // ковычки
        const char Quotes[8];
        // наибольшая ковычка
        const char gQ;
        // наибольший коментарий
        const char gC;
    };

    struct __config_loader {
    /* публичные методы */
        // Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые
        int (*load)(struct __config_loader *Inst, const char *FileName);
        // устанавливает типы строчных комментариев
        int (*set_string_comments)(struct __config_loader *I, const char* Keys);
        // очищает буферы
        void (*clear)(struct __config_loader *I);
        // 1 - если успешно загружен, 0 - в случае ошибки
        int (*isInit)(struct __config_loader *I);
    /* деструктор */
        void (*destruct)(struct __config_loader *I);

    /* Атрибуты, или поля, или как там ещё =D */
        struct __loader_buffers Buffers;
        struct __loader_charset Charset;
        struct __loader_errors Errors;
    };
    // конструктор класса
    struct __config_loader* loader_construct(void* ptr);
    
    
/* защищённые функции */
    //#ifdef __config_loader_c__
    /* деструкторы */
        void loader_cleanup_instance(struct __config_loader *Ptr);
        void loader_cleanup_buffers(struct __config_loader *Inst);
        void loader_cleanup_groups(struct Config_Groups *Gps);
        void loader_cleanup_group(cfggrp_t *Grp);
    /* функции(методы) */
        int loader_set_scomments(struct __config_loader *I, const char *Keys);
        // инициализирует набор символов
        int loader_init_charset(struct __loader_charset *CS);
        int loader_sort_charset(struct __loader_charset *CS);
        // 1 - если успешно загружен, 0 - в случае ошибки
        int loader_isInit(struct __config_loader *Inst);
        // инициализирует буфферы и считывает в них файл
        int loader_init_buffers(struct __config_loader *CL, const char *FileName);
        int loader_process_file(struct __config_loader *CL, const char *FileName);
        int loader_clean_file(struct __config_loader *I);
        // вычисляет конец "Си" комментария
        int loader_endof_c_comment(struct __loader_buffers *I);
        // вычисляет конец строчного коментария
        int loader_endof_s_comment(struct __loader_buffers *I);
        // удаляет пустые строки
        int loader_trimm_emty_strings(struct __loader_buffers *I);
        // меняет буфферы местами
        void loader_swap_buffers(struct __loader_buffers *B);
        // выводит ошибки в терминал
        void loader_perror(struct __config_loader *I, const char * What, int Error);
        // меняет символы местами
        int loader_swap_c(char *c1, char *c2);
        // сортирует строку по возрастанию
        int loader_strsort_az(char *a);
        // сортирует строку по убыванию
        int loader_strsort_za(char *a);
        // разворачивает строку
        int loader_strinverse(char *a);
    //#endif //__config_loader_c__

#endif // __config_loader_h__