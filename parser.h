#ifndef __config_parser_h__
#define  __config_parser_h__ 1

#include <sys/types.h>

    struct __config_parser_flags {
        u_int32_t BldType : 2;
        u_int32_t CntType : 2;
        u_int32_t DynInst : 1;
        u_int32_t SrcInit : 1;
        u_int32_t MapInit : 1;
        u_int32_t MapBldd : 1;
        u_int32_t PreInit : 1;
    };
    /* Переменная в строковом виде */
    typedef struct __config_parser_var {
        // Переменная
        char *Var;
        // Значение
        char *Val;
    } parser_variable_t;

    /** Группа строковых переменных */
    typedef struct __config_parser_var_group {
        // Количество переменных
        size_t VarCnt;
        // имя группы
        char *Name;
        // Массив переменных
        parser_variable_t *Vars;
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
        int (*const init)(struct __config_parser *Inst, const char *Src);
        /** Устанавливает метод подсчёта */
        int (*const set_counting_method)(
            struct __config_parser *Inst,
            int (*callback)(const unsigned char *Src, const size_t SrcSz, size_t *TotalGrps, size_t *TotalVars)
        );
        /** Устанавливает метод построения */
        int (*const set_building_method)(
            struct __config_parser *Inst,
            int (*callback)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt)
        );

        /** Находит переменную Name в группе GrpName и возвращает указатель на неё
         * если переменная не найдена вернётся ((parser_variable_t*)0) */
        parser_variable_t *(*const get_var)(const struct __config_parser *Inst, const char *Name, const char *GrpName);
        /** Сканирует значение как целое со знаком 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_i)(int32_t *Val, char *Var);
        /** Сканирует значение как целое без знака 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_ui)(u_int32_t *Val, char *Var);
        /** Сканирует значение как целое со знаком 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_l)(int64_t *Val, char *Var);
        /** Сканирует значение как целое без знака 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_ul)(u_int64_t *Val, char *Var);
        /** Сканирует значение как вещественное 4 байта и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_f)(float *Val, char *Var);
        /** Сканирует значение как вещественное 8 байт и записывает в Val
         * в случае неудачи возвращает -1, а поле остаётся нетронутым */
        int (*const parse_d)(double *Val, char *Var);
        /** В случае успеха возвращает указатель на строку значения переменной,
         * иначе - возвращается нулевой указатель ((char*)0)!!! */
        const char *(*const parse_str)(char *Var);
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
        int (*const __private_counter)(struct __config_parser *Inst);
        /** Переменная для хранения метода подсчёта групп и переменных */
        int (*const __private_builder)(struct __config_parser *Inst);
/** public: */
        // Найдено групп
        const size_t __private_gc;
        // Найдено переменных
        const size_t __private_vc;
        // Указатель на массив групп
        const parser_group_t *const __private_map;
        // Указатель на массив переменных
        const parser_group_t *const __private_vars;

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
    ConfigParser* config_parser_construct(ConfigParser *Inst);

#ifdef __config_parser_c__
/** Флаги состояния */
    enum __config_parser_flags_e {
        CP_FLAG_BTPD0  = 0b0001, CP_FLAG_BTPD1  = 0b0010, CP_FLAG_BTPUSR = 0b0011,
        CP_FLAG_CTPD0  = 0b0100, CP_FLAG_CTPD1  = 0b1000, CP_FLAG_CTPUSR = 0b1100,
        CP_FLAG_DYNINS = 0b10000,
        CP_FLAG_SRCINT = 0b100000,
        CP_FLAG_MAPINT = 0b1000000,
        CP_FLAG_MAPBLD = 0b10000000,
        CP_FLAG_PREINT = 0b100000000
    };
/** Инициализаторы и конструкторы */

    /** Инициализирует созданный объект */
    int config_parser_preinit_instance(ConfigParser *Inst);
    /** Инициализатор интерпритатора */
    int config_parser_init(ConfigParser *Inst, const char *Src);
    /** Инициализатор исходного массива.
     * Перед инициализацией вызывает деструкторы
     * карты переменных и рабочего массива */
    int config_parser_init_source(ConfigParser *Inst, const char *Src);
    /** Инициализатор-конструктор карты */
    int config_parser_init_map(ConfigParser *Inst);
    /** Возвращает интерпритатор в исходное состояние */
    int config_parser_reset(ConfigParser *Inst);

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
    void config_parser_set_flag(ConfigParser *Inst, enum __config_parser_flags_e flags);
    /** Снимает флаги */
    void config_parser_unset_flag(ConfigParser *Inst, enum __config_parser_flags_e flags);
    /** Устанавливает метод построения карты */
    int config_parser_set_counting_method(ConfigParser *Inst, int (*callback)(const unsigned char *Src, const size_t SrcSz, size_t *GrpCnt, size_t *VarCnt));
    /** Устанавливает метод подсчёта групп и переменных */
    int config_parser_set_building_method(ConfigParser *Inst, int (*callback)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt));

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
    int config_parser_count_default0(const unsigned char *Src, const size_t SrcSz, size_t *Grps, size_t *Vars, );
    /** Создаёт структуру из конфига вида:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    int config_parser_build_default0(ConfigParser *Inst);

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
    int config_parser_build_default1(ConfigParser *Inst);
    /** */
    int config_parser_check_var(const unsigned char *Src);
    int config_parser_check_val(const unsigned char *Src);
    int config_parser_check_grp(const unsigned char *Src);

#endif // __config_parser_c__

#endif // __config_parser_h__