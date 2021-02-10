#ifndef __config_parser_h__
#define  __config_parser_h__ 1

#include <sys/types.h>

#include "headers/cleaner.h"

    struct __config_parser_flags {
        u_int32_t BTPDT1 : 1;
        u_int32_t BTPUSR : 1;
        u_int32_t DYNINS : 1;
        u_int32_t SRCINT : 1;
        u_int32_t MAPINT : 1;
        u_int32_t MAPBLD : 1;
        u_int32_t PREINT : 1;
    };
    union __config_parser_flag_set {
        struct __config_parser_flags s;
        u_int32_t ui;
    };
    /* Переменная в строковом виде */
    typedef struct __config_parser_var {
        // Переменная
        const char *const Var;
        // Значение
        const char *const Val;
    } parser_variable_t;

    /** Группа строковых переменных */
    typedef struct __config_parser_var_group {
        // Количество переменных
        const size_t VarCnt;
        // имя группы
        const char * const Name;
        // Массив переменных
        const parser_variable_t *const Vars;
    } parser_group_t;

/** Интерпритатор конфигурационных файлов */
    
    /* Список групп и переменных в них */
    typedef struct __config_parser {
/** public: */
        /** Деструктор */
        void (*const destruct)(struct __config_parser *Inst);
        /** Очистка (удаление рабочего массива и установка параметров по умолчанию) */
        void (*const reset)(struct __config_parser *Inst);
        /** Инициализатор исходного массива */
        int (*const init)(struct __config_parser *Inst, const char *FileName);
        /** Устанавливает метод построения */
        int (*const set_building_method)(
            struct __config_parser *Inst,
            int (*counter)(const unsigned char *Src, const size_t SrcSz, size_t *TotalGrps, size_t *TotalVars),
            int (*builder)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt)
        );
        /** Сканирует значение как целое со знаком 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_i)(int32_t *Val, const struct __config_parser *Inst, const char *VarName, const char *GrpName);
        /** Сканирует значение как целое без знака 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_ui)(u_int32_t *Val, const struct __config_parser *Inst, const char *VarName, const char *GrpName);
        /** Сканирует значение как целое со знаком 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_l)(int64_t *Val, const struct __config_parser *Inst, const char *VarName, const char *GrpName);
        /** Сканирует значение как целое без знака 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_ul)(u_int64_t *Val, const struct __config_parser *Inst, const char *VarName, const char *GrpName);
        /** Сканирует значение как вещественное 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_f)(float *Val, const struct __config_parser *Inst, const char *VarName, const char *GrpName);
        /** Сканирует значение как вещественное 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_d)(double *Val, const struct __config_parser *Inst, const char *VarName, const char *GrpName);
        /** В случае успеха возвращает указатель на строку значения переменной,
         * иначе - возвращается нулевой указатель ((char*)0)!!! */
        const char *(*const parse_str)(const struct __config_parser *Inst, const char *VarName, const char *GrpName);
/** private: */
        /** Расчитывает количество групп и переменных:
         * Если не установлен пользовательсий метод,
         * то используются метод по умолчанию */
        int (*const __private_count)(struct __config_parser *Inst);
        /** Строит карту групп и переменных:
         * Если не установлен пользовательсий метод,
         * то используются метод по умолчанию */
        int (*const __private_build)(struct __config_parser *Inst);
        /** Инициализатор-конструктор выходного массива */
        int (*const __private_init_map)(struct __config_parser *Inst);
        /** Переменная для хранения метода подсчёта групп и переменных */
        int (*const __private_counter)(const unsigned char *Src, const size_t SrcSize, size_t *Grps, size_t *Vars);
        /** Переменная для хранения метода подсчёта групп и переменных */
        int (*const __private_builder)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt);

        // Найдено групп
        const size_t __private_gc;
        // Найдено переменных
        const size_t __private_vc;
        // Указатель на массив групп
        const parser_group_t *const __private_map;
        // Указатель на массив переменных
        const parser_variable_t *const __private_vars;

        // Рабочий массив
        const unsigned char *const __private_src;
         // Размер рабочего массива
        const size_t __private_srcsz;
        // Флаги состояния
        const struct __config_parser_flags __private_flags;
        // Ошибки выполнения
        const int Error;
    } ConfigParser;

    /** Конструктор интерпритатора */
    ConfigParser* new_config_parser(ConfigParser *Inst);

#ifdef __config_parser_c__
/** Флаги состояния */
    enum __config_parser_flags_e {
        CP_FLAG_BTPDT1 = 0b1,
        CP_FLAG_BTPUSR = 0b10,
        CP_FLAG_DYNINS = 0b100,
        CP_FLAG_SRCINT = 0b1000,
        CP_FLAG_MAPINT = 0b10000,
        CP_FLAG_MAPBLD = 0b100000,
        CP_FLAG_PREINT = 0b1000000
    };
/** Инициализаторы и конструкторы */

    /** Инициализирует созданный объект */
    int config_parser_preinit_instance(ConfigParser *Inst);
    /** Инициализатор исходного массива.
     * Перед инициализацией вызывает деструкторы
     * карты переменных и рабочего массива */
    int config_parser_init_source(ConfigParser *Inst, const char *FileName);
    /** Инициализатор-конструктор карты */
    int config_parser_init_map(ConfigParser *Inst);
    /** Возвращает интерпритатор в исходное состояние */
    int config_parser_reset(ConfigParser *Inst);
    /** */
    int config_parser_construct_var(const parser_variable_t *Var, const char *N, const char *V);
    /** */
    int config_parser_construct_grp(const parser_group_t *Grp, const char *N, const size_t Cnt, const parser_variable_t *Vars);
/** Структуры-инициализаторы */

    // Инициализатор строковой переменной
    extern struct __config_parser_var __config_parser_struct_init_var;
    // Инициализатор группы строковых переменных
    extern struct __config_parser_var_group __config_parser_struct_init_group;

/** Деструкторы */

    /** Деструктор интерпритатора */
    void config_parser_destruct(ConfigParser *Inst);
    /** Деструктор исходного массива,
     * перед освобождением рабочего массива
     * вызывает деструктор карты переменных */
    void config_parser_destruct_src(ConfigParser *Inst);
    /** Деструктор карты переменных */
    void config_parser_destruct_map(ConfigParser *Inst);

/** Установщики */

    /** Устанавливает флаги */
    void config_parser_set_flag(ConfigParser *Inst, unsigned int flags);
    /** Снимает флаги */
    void config_parser_unset_flag(ConfigParser *Inst, unsigned int flags);
    /** Устанавливает метод подсчёта и построения групп и переменных */
    int config_parser_set_building_method(
        ConfigParser *Inst,
        int (*counter)(const unsigned char *Src, const size_t SrcSz, size_t *GrpCnt, size_t *VarCnt),
        int (*builder)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt)
    );

/** Основные методы */

    /** Расчитывает количество групп и переменных:
     * Если не установлен пользовательсий метод,
     * то используются метод по умолчанию */
    int config_parser_count_map(ConfigParser *Inst);
    /** Строит карту групп и переменных:
     * Если не установлен пользовательсий метод,
     * то используются метод по умолчанию */
    int config_parser_build_map(ConfigParser *Inst);

    int config_parser_count_default(ConfigParser *Inst);
    /** Считает количество символьных переменных в Buff разделённых между собой символом Sep:
     * 'Var1=Value'
     * 'Var2=Value'
     * 'Var3=Value'
     * ... */
    int config_parser_count_default0(const unsigned char *Src, const size_t SrcSz, size_t *Grps, size_t *Vars);
    /** Создаёт структуру из конфига вида:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    int config_parser_build_default0(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt);

    /** Считает количество групп Grps и переменных Vars из строки Buff:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_parser_count_default1(const unsigned char *Src, const size_t SrcSz, size_t *Grps, size_t *Vars);
    /** Создаёт структуру из конфига вида:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_parser_build_default1(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt);
    /** */
    int config_parser_scan_var(const parser_variable_t *v, unsigned char *s, size_t *p);
    int config_parser_scan_val(const parser_variable_t *v, unsigned char *s, size_t *p);
    int config_parser_scan_grp(const parser_group_t *v, unsigned char *s, size_t *p);

    /** Выполняет поиск переменой в массиве.
     * Если переменная с таким именем не найдена, возвращается нулевой указатель: ((config_variable_t*)0) */
    const parser_variable_t* config_parser_get_var(const ConfigParser *Inst, const char *Var, const char *Grp);
    /** Сканирует значение как целое со знаком 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_i(int *Val, const ConfigParser *Inst, const char *VarName, const char *GrpName);
    /** Сканирует значение как целое без знака 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ui(unsigned int *Val, const ConfigParser *Inst, const char *VarName, const char *GrpName);
    /** Сканирует значение как целое со знаком 8 байт и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_l(long *Val, const ConfigParser *Inst, const char *VarName, const char *GrpName);
    /** Сканирует значение как целое без знака 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ul(unsigned long *Val, const ConfigParser *Inst, const char *VarName, const char *GrpName);
    /** Сканирует значение как вещественное 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_f(float *Val, const ConfigParser *Inst, const char *VarName, const char *GrpName);
    /** Сканирует значение как вещественное 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_d(double *Val, const ConfigParser *Inst, const char *VarName, const char *GrpName);
    // Возвращает указатель на первый символ после ковычки
    const char *config_parser_parse_str(const ConfigParser *Inst, const char *VarName, const char *GrpName);

#endif // __config_parser_c__

#endif // __config_parser_h__