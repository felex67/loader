/**
 * Загрузчик версии 2. Расчитан на динамическую компановку,
 * для статической компановки необходимр убрать макрос
 * #ifdef __config_loader_c__
 * 
*/

#ifndef __config_loader_h__
#define __config_loader_h__ 1

#include <sys/types.h>
struct __config_loader;
/*************************************************************************************************************/
    typedef struct __config_loader_var {
        char *Var;
        char *Val;
    } config_variable_t;
    // инициализатор
    extern struct __config_loader_var __config_loader_init_struct_var;

    typedef struct __config_loader_var_group {
        size_t count;
        char *Name;
        config_variable_t *Vars;
    } config_group_t;
    // инициализатор
    extern struct __config_loader_var_group __config_loader_init_struct_group;

    struct __config_loader_var_groups {
        size_t count;
        config_group_t **Groups;
        // Деструктор
        void (*destruct)(struct __config_loader_var_groups*);
    };
    // инициализатор
    extern struct __config_loader_var_groups __config_loader_init_struct_groups;
    void config_loader_destruct_groups(struct __config_loader_var_groups *Gps);

/*************************************************************************************************************/
    typedef struct __config_loader_flags {
        u_int32_t DynInst : 1;
        u_int32_t Init : 1;
    } config_loader_flags_t;
    
    typedef union __config_loader_union_flags {
        u_int32_t ui32;
        config_loader_flags_t s_flags;
    } config_loader_flags_un;
/* Errors */
    struct __config_loader_errors {
        // массив для обработки ошибок
        char Buff[2048];
        // номер ошибки
        int error;
        config_loader_flags_un unFl;
    };
    /* Инициализатор */
    extern struct __config_loader_errors __config_loader_init_struct_errors;
    // выводит ошибки в std::out
    void config_loader_perror(struct __config_loader *I, const char * What, int Error);
/* Buffers */    
    struct __config_loader_buffers {
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
        // Деструктор
        void (*destruct)(struct __config_loader *Instance);
    };
    extern struct __config_loader_buffers __config_loader_init_struct_buffers;
    int config_loader_init_buffers(struct __config_loader *CL, const char *FileName);
    void config_loader_destruct_buffers(struct __config_loader *Inst);
/* Charset */
    extern const char __config_loader_default_quotes[];
    extern const char __config_loader_default_lcomm[];
    extern const char __config_loader_default_nonspace[];
    extern const char __config_loader_default_group[];

    struct __config_loader_charset {
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
    /* Конструктор */
    int config_loader_construct_charset(struct __config_loader_charset *CS);
/* Config_loader */
    struct __config_loader {
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
        // Массивы
        struct __config_loader_buffers Buffers;
        // Символы
        struct __config_loader_charset Charset;
        // Ошибки и статы
        struct __config_loader_errors Errors;
        // Выходная структура
        struct __config_loader_var_groups Config;
    };
    /* конструктор класса */
    struct __config_loader* config_loader_construct(void* ptr);
    /* Деструктор */
    void config_loader_destruct(struct __config_loader *Inst);
    
/* защищённые функции */
    #ifdef __config_loader_c__
/* config_loader */
        // Main method of config_loader, loads & processes config file
        int config_loader_process_file(struct __config_loader *CL, const char *FileName);
        // set user comment types
        int config_loader_set_scomments(struct __config_loader *I, const char *Keys);
        // cleans the file from comments and spaces
        int config_loader_clean_file(struct __config_loader *I);
        // 1 - если успешно загружен, 0 - в случае ошибки
        int config_loader_isInit(struct __config_loader *Inst);
        // удаляет пустые строки
        int config_loader_delete_emty_strings(struct __config_loader_buffers *I);
        /* TODO */
        //int config_loader_count_entries(struct __config_loader *CL);
/* Buffers */
        // инициализирует буфферы и считывает в них файл
        int config_loader_init_buffers(struct __config_loader *CL, const char *FileName);
        // меняет буфферы местами
        void config_loader_swap_buffers(struct __config_loader_buffers *B);
/* Charset */
        // sort characters
        int config_loader_sort_charset(struct __config_loader_charset *CS);
/* Other */
        // сортирует строку по возрастанию
        int config_loader_strsort_az(char *a);
        // сортирует строку по убыванию
        int config_loader_strsort_za(char *a);
        // разворачивает строку
        int config_loader_strinverse(char *a);
    #endif //__config_loader_c__

#endif // __config_loader_h__