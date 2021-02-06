/**
 * Загрузчик версии 2. Расчитан на динамическую компановку,
 * для статической компановки необходимр убрать макрос
 * #ifdef __config_loader_c__
 * 
*/

#ifndef __config_loader_h__
#define __config_loader_h__ 1

#include <sys/types.h>

extern int errno;

inline int *INVALID_FUNCTION(void* argv[]) { return -1; }

/** Загрузчик конфигурационных файлов */
struct __config_loader;


/** class __config_loader_var_groups */
    
    /* Переменная в строковом виде */
    typedef struct __config_loader_var {
        // Переменная
        char *Var;
        // Значение
        char *Val;
    } config_variable_t;
    
    // Инициализатор строковой переменной
    extern struct __config_loader_var __config_loader_init_struct_var;

    /** Группа строковых переменных */
    typedef struct __config_loader_var_group {
        // Количество переменных
        size_t count;
        // имя группы
        char *Name;
        // Массив переменных
        config_variable_t *Vars;
    } config_group_t;
    
    // Инициализатор группы строковых переменных
    extern struct __config_loader_var_group __config_loader_init_struct_group;
    
    /* Список групп и переменных в них */
    struct __config_loader_var_groups {
        // Находит переменную и возвращает указатель на неё
        // если переменная не найдена вернётся ((config_variable_t*)0)
        config_variable_t* (*get_var)(const char *Name, const char *GrpName);
        /** Сканирует значение как целое со знаком 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*parse_i)(void *Val, struct __config_loader_var *Var);
        /** Сканирует значение как целое без знака 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*parse_ui)(void *Val, struct __config_loader_var *Var);
        /** Сканирует значение как целое со знаком 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*parse_l)(void *Val, struct __config_loader_var *Var);
        /** Сканирует значение как целое без знака 8 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*parse_ul)(void *Val, struct __config_loader_var *Var);
        /** Сканирует значение как вещественное 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*parse_f)(void *Val, struct __config_loader_var *Var);
        /** Сканирует значение как вещественное 8 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*parse_d)(void *Val, struct __config_loader_var *Var);
        // Возвращает указатель на первый символ после ковычки
        int (*parse_str)(void *Val, struct __config_loader_var *Va);
        // Найдено групп
        size_t GrpCnt;
        // Найдено переменных
        size_t VarCnt;
        // Указатель на массив указателей
        config_group_t **Groups;
    };
    // Инициализатор списка групп и перменных
    extern struct __config_loader_var_groups __config_loader_init_struct_groups;
    // Деструктор списка групп и переменных
    void config_loader_destruct_groups(struct __config_loader_var_groups *Gps);

    /** Считает количество символьных переменных в Buff разделённых между собой символом Sep:
     * 'Var1=Value'
     * 'Var2=Value'
     * 'Var3=Value'
     * ... */
    size_t config_loader_count_simple(const unsigned char *Buff, const unsigned char C);

    /** Считает количество групп Grps и переменных Vars из строки Buff:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_loader_count_multi1(size_t *Grps, size_t *Vars, const unsigned char *Buff);
    
    /** Считает количество групп Grps и переменных Vars из строки Buff:
     * BEGIN Var1=Value ... VarN=Value END ... */
    int config_loader_count_multi2(size_t *Grps, size_t *Vars, const unsigned char *Buff);

/** class __config_loader_errors */

    /** Флаги состояния загрузчика */
    typedef struct __config_loader_flags {
        // Флаг ошибки
        u_int32_t Error : 1;
        // Выделена динамическая память
        u_int32_t DynInst : 1;
        // Массивы инициализированы
        u_int32_t Init : 1;
        // Файл очищен
        u_int32_t FileClear : 1;
        // Списрк построен
        u_int32_t ListBuild : 1;
    } config_loader_flags_t;

    /** Объединение флагов состояния загрузчика и unsigned int32 */
    typedef union __config_loader_union_flags {
        u_int32_t ui32;
        config_loader_flags_t s_flags;
    } config_loader_flags_un;
    
    /* Обработчик ошибок исполнения процесса загрузки */
    struct __config_loader_errors {
        // массив для обработки ошибок
        char Buff[2048];
        // номер ошибки
        int error;
        // Флаги состояния (union)
        config_loader_flags_un unFl;
    };

    /* Инициализатор обработчика ошибок */
    extern struct __config_loader_errors __config_loader_init_struct_errors;
    
    // выводит отформатированные ошибки в std::err
    void config_loader_perror(struct __config_loader *I, const char * What, int Error);

/** class __config_loader_buffers */

    /** Структура массивов включающая в себя входной и рабочий массивы,
     * информацию о размере и позициях */
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
    // Инициализатор Структуры массивов
    extern struct __config_loader_buffers __config_loader_init_struct_buffers;
    
    /**Выделяет память рабочему: Wrk и входному: In массивам,
     * затем загружает файл во входной массив: In */
    int config_loader_init_buffers(struct __config_loader *CL, const char *FileName);
    /**Деструктор структуры массивов:
     * освобождает раннее выделенную память */
    void config_loader_destruct_buffers(struct __config_loader *Inst);

/** class __config_loader_charset */

    /** Строка ковычек.
     * По умолчанию: ` ' " */
    extern const char __config_loader_default_quotes[];
    
    /** Строка символов обозначающих начало строкового комментария.
     * По умолчанию: # ; */
    extern const char __config_loader_default_lcomm[];
    
    /** Строка символо "не пробелов".
     * По умолчанию: /t /n */
    extern const char __config_loader_default_nonspace[];
    
    /**Строка определяющая начало группы переменных.
     * По умолчанию: [ */
    extern const char __config_loader_default_group[];

    /** Набор символов необходимых для интерпритации файла*/
    struct __config_loader_charset {
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
    /* Функция-инициализатор набора символов */
    int config_loader_construct_charset(struct __config_loader_charset *CS);

/* class config_loader */
    struct __config_loader {
        /* Деструктор загрузчика */
        void (*destruct)(struct __config_loader *I);
        // Загружает и отчищает файл от коментариев, удаляет пустые строки
        int (*load)(struct __config_loader *Inst, const char *FileName);
        // устанавливает типы строчных комментариев
        int (*set_string_comments)(struct __config_loader *I, const char* Keys);
        // очищает буферы
        void (*clear)(struct __config_loader *I);
        // Построить структуру
        int (*build)(struct __config_loader *this);
        // 1 - если успешно загружен, 0 - в случае ошибки
        int (*isInit)(struct __config_loader *I);
        // Очищает список и массивы, сбрасывается на настройки по умолчанию
        void (*reset)();
    /* Атрибуты, или поля, или как там ещё =D */
        // Структура массивов
        struct __config_loader_buffers Buffers;
        // Набор символов
        struct __config_loader_charset Charset;
        // Обработчик ошибок
        struct __config_loader_errors Errors;
        // Выходная структура группированных переменных
        struct __config_loader_var_groups Config;
        // Указатель на входной массив
        char **pBuffer;
        // Указатель на "полезный" размер входного массива
        size_t *pBuffSz;
    };
    /* конструктор загрузчика конфигурационных файлов */
    struct __config_loader* config_loader_construct(void* ptr);
    
    /* Деструктор загрузчика конфигурационных файлов */
    void config_loader_destruct(struct __config_loader *Inst);
    
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

/* __config_loader_var_groups */

/* Other */
        // сортирует строку по возрастанию
        int config_loader_strsort_az(char *a);
        // сортирует строку по убыванию
        int config_loader_strsort_za(char *a);
        // разворачивает строку
        int config_loader_strinverse(char *a);
    #endif //__config_loader_c__

#endif // __config_loader_h__