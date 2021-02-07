/**
 * Загрузчик версии 2. Расчитан на динамическую компановку,
 * для статической компановки необходимр убрать макрос
 * #ifdef __config_loader_c__
 * 
*/

#ifndef __config_loader_h__
#define __config_loader_h__ 1

#define __test_config_loader_c__ 1

#include <sys/types.h>

/** Загрузчик конфигурационных файлов */
struct __config_loader;

/** class __config_loader_var_groups */
#ifdef __config_loader_c__
/** Структуры-инициализаторы */
    // Инициализатор строковой переменной
    extern struct __config_loader_var __config_loader_init_struct_var;
    // Инициализатор группы строковых переменных
    extern struct __config_loader_var_group __config_loader_init_struct_group;
    // Инициализатор списка групп и перменных
    extern struct __config_loader_var_groups __config_loader_init_struct_groups;

/** Конструкторы */

    /** Конструктор списка груп и переменных */
    int config_loader_construct_var_groups(struct __config_loader *Inst);

    /** Деструктор списка групп и переменных */
    void config_loader_destruct_var_groups(struct __config_loader *Inst);

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
    /** Создаёт структуру из конфига вида:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    int config_loader_var_groups_build0(struct __config_loader *Inst);
    /** Создаёт структуру из конфига вида:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_loader_var_groups_build1(struct __config_loader *Inst);
    /** Создаёт структуру из конфига вида:
     * BEGIN Var1=Value ... VarN=Value END
     * BEGIN Var1=Value ... VarN=Value END
     * ... 
    int config_loader_var_groups_build2(struct __config_loader *Inst);*/

/** Методы */

#endif // __config_loader_c__

/** class __config_loader_errors */

    /** Флаги состояния загрузчика */
    typedef struct __config_loader_flags {
        // Флаг ошибки
        int32_t Error : 1;
        // Выделена динамическая память
        int32_t DynInst : 1;
        // Массивы инициализированы
        int32_t BuffInit : 1;
        // Файл очищен
        int32_t FileClear : 1;
        // Списрк построен
        int32_t ListBuild : 1;
    } config_loader_flags_t;

    /** Объединение флагов состояния загрузчика и unsigned int32 */
    typedef union __config_loader_union_flags {
        int32_t ui32;
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

    /* Структура-инициализатор обработчика ошибок */
    extern struct __config_loader_errors __config_loader_init_struct_errors;
    
    // выводит отформатированные ошибки в std::err
    void config_loader_perror(struct __config_loader *I, const char * What);

/** class __config_loader_buffers */

    /** Структура массивов включающая в себя входной и рабочий массивы,
     * информацию о размере и позициях */
    struct __config_loader_buffers {
        // Деструктор
        void (*destruct)(struct __config_loader *Instance);
        // Сброс
        void (*reset)(struct __config_loader *Instance);

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

#ifdef __config_loader_c__
    // Инициализатор Структуры массивов
    extern struct __config_loader_buffers __config_loader_init_struct_buffers;
    
    /**Выделяет память рабочему: Wrk и входному: In массивам,
     * затем загружает файл во входной массив: In */
    int config_loader_init_buffers(struct __config_loader *CL, const char *FileName);
    /**Деструктор структуры массивов:
     * освобождает раннее выделенную память */
    void config_loader_destruct_buffers(struct __config_loader *Inst);
#endif // __config_loader_c__

/** class __config_loader_charset */

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
#ifdef __config_loader_c__

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

    /* Функция-инициализатор набора символов */
    int config_loader_construct_charset(struct __config_loader_charset *CS);
#endif // __config_loader_c__

/* class config_loader */
    typedef struct __config_loader {
        /* Деструктор загрузчика */
        void (*destruct)(struct __config_loader *Inst);
        // Очищает буферы, устанавливает значения по умолчанию
        void (*reset)(struct __config_loader* Inst);
        // Загружает и отчищает файл от коментариев, удаляет пустые строки
        int (*load)(struct __config_loader *Inst, const char *FileName);
        // устанавливает типы строчных комментариев
        int (*set_string_comments)(struct __config_loader *I, const char* Keys);
        // очищает буферы
        void (*clear)(struct __config_loader *I);
        // Выводит информацию об ошибке в std::cerr
        void (*perror)(struct __config_loader *Inst, const char *What);
        

        /* Атрибуты, или поля, или как там ещё =D */
        // Структура массивов
        struct __config_loader_buffers Buffers;
        // Набор символов
        struct __config_loader_charset Charset;
        // Обработчик ошибок
        struct __config_loader_errors Errors;
        // Выходная структура группированных переменных
        //struct __config_loader_var_groups Config;
        // Указатель на входной массив
        const char * const *Result;
        // Указатель на "полезный" размер входного массива
        const size_t *const Size;
    } ConfigLoader;
    /* конструктор загрузчика конфигурационных файлов */
    ConfigLoader* config_loader_construct(void* ptr);
    
#ifdef __config_loader_c__
/* config_loader */
        extern ConfigLoader __config_loader_init_struct;
        /* Деструктор загрузчика конфигурационных файлов */
        void config_loader_destruct(ConfigLoader *Inst);
        // Очищает массивы, устанавливает значения по умолчанию
        void config_loader_reset(ConfigLoader *Inst);
        // Main method of config_loader, loads & processes config file
        int config_loader_process_file(ConfigLoader *CL, const char *FileName);
        // set user comment types
        int config_loader_set_scomments(ConfigLoader *I, const char *Keys);
        // cleans the file from comments and spaces
        int config_loader_clean_file(ConfigLoader *I);
        // 1 - если успешно загружен, 0 - в случае ошибки
        int config_loader_isInit(ConfigLoader *Inst);
        // удаляет пустые строки
        int config_loader_delete_emty_strings(struct __config_loader_buffers *I);

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