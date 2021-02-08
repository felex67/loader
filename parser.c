#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __config_parser_c__ 1

#include "parser.h"

#ifndef nullptr
    #define nullptr ((void*)0)
#endif // nullptr

/** Конструкторы и инциализаторы */
    
    // Инициализатор переменной
    struct __config_parser_var __config_parser_struct_init_var = {
        .Var = nullptr,
        .Val = nullptr
    };
    // Инициализатор группы переменных
    struct __config_parser_var_group __config_parser_struct_init_group = {
        .VarCnt = 0,
        .Name = nullptr,
        .Vars = nullptr
    };

    /** Конструктор интерпритатора */
    ConfigParser* config_parser_construct(ConfigParser *Ptr) {
        ConfigParser (*Inst) = Inst;
        if (nullptr == Inst) { Inst = (ConfigParser*)malloc(sizeof(ConfigParser)); }
        if (nullptr != Inst) {
            if (!config_parser_preinit_instance(Inst)) {
                if (nullptr == Ptr) config_parser_set_flag(Inst, CP_FLAG_DYNINS);
            }
            else {
                if (nullptr == Ptr) free(Inst);
                Inst = nullptr;
            }
        }
        return Inst;
    }
    /** Инициализатор интерпритатора */
    int config_parser_preinit_instance(ConfigParser *Inst) {
        if (nullptr != Inst) {
            *((void**)&Inst->destruct) = config_parser_destruct;
            *((void**)&Inst->reset) = config_parser_reset;
            *((int**)&Inst->init) = config_parser_init;
            *((int**)&Inst->set_counting_method) = config_parser_set_counting_method;
            *((int**)&Inst->set_building_method) = config_parser_set_building_method;

            *((int**)&Inst->get_var) = config_parser_get_var;
            *((int**)&Inst->parse_i) = config_parser_parse_i;
            *((int**)&Inst->parse_ui) = config_parser_parse_ui;
            *((int**)&Inst->parse_l) = config_parser_parse_l;
            *((int**)&Inst->parse_ul) = config_parser_parse_ul;
            *((int**)&Inst->parse_f) = config_parser_parse_f;
            *((int**)&Inst->parse_d) = config_parser_parse_f;
            *((char**)&Inst->parse_str) = config_parser_parse_str;

            *((int**)&Inst->__private_init_map) = config_parser_init_map;
            *((int**)&Inst->__private_count) = config_parser_count_map;
            *((int**)&Inst->__private_build) = config_parser_build_map;
            *((int**)&Inst->__private_counter) = nullptr;
            *((int**)&Inst->__private_builder) = nullptr;

            *((size_t*)&(Inst->__private_gc)) = *((size_t*)&(Inst->__private_vc)) = 0;
            *((void**)&(Inst->__private_src)) = nullptr;
            *((void**)&(Inst->__private_map)) = nullptr;
            *((void**)&(Inst->__private_vars)) = nullptr;
            *((u_int32_t*)&(Inst->__private_flags)) = 0;
            *((int*)&(Inst->Error)) = 0;

            config_parser_set_flag(Inst, CP_FLAG_PREINT);
            
            return 0;
        }
        return -1;
    }
    /** Инициализатор исходного массива */
    int config_parser_init_source(ConfigParser *Inst, const char *Src) {
        if ((nullptr != Inst) && (nullptr != Src)) {
            if (Inst->__private_flags.SrcInit) config_parser_destruct_src(Inst);
            size_t len = strlen(Src);
            char *src = malloc(len + 1);
            if (nullptr != src) {
                strcpy(src, Src);
                *((char**)&(Inst->__private_src)) = src;
                config_parser_set_flag(Inst, CP_FLAG_SRCINT);
                return 0;
            }
        }
        return -1;
    }
    /** Инициализатор-конструктор выходного массива */
    int config_parser_init_map(ConfigParser *Inst) {
        if ((nullptr != Inst) && (Inst->__private_flags.SrcInit)) {
            if (!Inst->__private_count(Inst)) {
                size_t gbytes = (sizeof(parser_group_t) * Inst->__private_gc);
                size_t vbytes = (sizeof(parser_variable_t) * Inst->__private_vc);
                unsigned char *barray = malloc(gbytes + vbytes);
                if (nullptr != barray) {
                    parser_group_t *pGrps = (parser_group_t*)barray;
                    parser_variable_t *pVars = (parser_variable_t*)(barray + gbytes);
                    *((void**)&(Inst->__private_map)) = (void*)barray;
                    for (size_t i = 0; i < Inst->__private_gc; i++) {
                        pGrps[i] = __config_parser_struct_init_group;
                    }
                    pGrps[0].Vars = pVars;
                    for (size_t i = 0; i < Inst->__private_gc; i++) {
                        pVars[i] = __config_parser_struct_init_var;
                    }
                    config_parser_set_flag(Inst, CP_FLAG_MAPINT);
                    return 0;
                }
            }
        }
        return -1;
    }
    /** Возвращает интерпритатор в исходное состояние */
    int config_parser_reset(ConfigParser *Inst) {
        if (nullptr != Inst) {
            u_int32_t flags = Inst->__private_flags.DynInst;
            config_parser_preinit_instance(Inst);
            config_parser_set_flag(Inst, (enum __config_parser_flags_e)flags);
            return 0;
        }
        return -1;
    }

/** Деструкторы */

    /** Деструктор интерпритатора */
    void config_parser_destruct(ConfigParser *Inst) {
        if (nullptr != Inst) {
            if (Inst->__private_flags.SrcInit) free(Inst->__private_src);
            if (Inst->__private_flags.MapInit) free(Inst->__private_map);
            if (Inst->__private_flags.DynInst) free(Inst);
        }
    }
    /** Освобождает массив */
    void config_parser_destruct_src(ConfigParser *Inst) {
        if (nullptr != Inst) {
            if (Inst->__private_flags.SrcInit) free(Inst->__private_src);
            *((void**)&(Inst->__private_src)) = nullptr;
            config_parser_unset_flag(Inst, CP_FLAG_SRCINT);
            if (Inst->__private_flags.MapInit) config_parser_destruct_map(Inst);
        }
    }
    /** Освобождает только карту */
    void config_parser_destruct_map(ConfigParser *Inst) {
        if (nullptr != Inst) {
            if (Inst->__private_flags.MapInit) free(Inst->__private_map);
            *((void**)&(Inst->__private_map)) = nullptr;
            config_parser_unset_flag(Inst, CP_FLAG_MAPINT | CP_FLAG_MAPBLD);
        }
    }

/** Основные методы */

    int config_parser_count_map(ConfigParser *Inst) {
        int result = -1;
        if ((nullptr != Inst) && (Inst->__private_flags.SrcInit)) {
            if (!Inst->__private_counter) Inst->set_counting_method(Inst, config_parser_count_default);
            result = Inst->__private_counter(Inst);
        }
        return result;
    }

/** Установщики */

    /** Устанавливает флаги */
    void config_parser_set_flag(ConfigParser* Inst, enum __config_parser_flags_e Flags) {
        if (nullptr != Inst) {
            u_int32_t *flags = (u_int32_t*)&(Inst->__private_flags);
            if (CP_FLAG_BTPUSR & Flags) (*((struct __config_parser_flags*)&(flags))).BldType = 0;
            if (CP_FLAG_CTPUSR & Flags) (*((struct __config_parser_flags*)&(flags))).CntType = 0;
            (*(flags)) |= Flags;
        }
    }
    /** Снимает флаги */
    void config_parser_unset_flag(ConfigParser* Inst, enum __config_parser_flags_e Flags) {
        if (nullptr != Inst) {
            u_int32_t *flags = (u_int32_t*)&(Inst->__private_flags);
            if (CP_FLAG_BTPUSR & Flags) (*((struct __config_parser_flags*)flags)).BldType = 0;
            if (CP_FLAG_CTPUSR & Flags) (*((struct __config_parser_flags*)flags)).CntType = 0;
            (*flags) |= (Flags & 0xfffffff0);   // чтобы не получилось что мы не удалим, а - поставим флаг
            (*flags) ^= (0xfffffff0 & Flags);
        }
    }

    int config_parser_set_counting_method (
        ConfigParser *Inst,
        int (*callback)(const unsigned char *Src, const size_t SrcSz, size_t *GrpCnt, size_t *VarCnt)
    ) {
        if ((nullptr != Inst) && (nullptr != callback)) {
            *((int**)&(Inst->__private_builder)) = callback;
            config_parser_set_flag(Inst, CP_FLAG_CTPUSR);
            return 0;
        }
        return -1;
    }
    /** Устанавливает метод подсчёта групп и переменных */
    int config_parser_set_building_method (
        ConfigParser *Inst,
        int (*callback)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt)
    ) {
        if ((nullptr != Inst) && (nullptr != callback)) {
            *((int**)&(Inst->__private_counter)) = callback;
            config_parser_set_flag(Inst, CP_FLAG_BTPUSR);
            return 0;
        }
        return -1;
    }

/** Вывод */

    /** Выполняет поиск переменой в массиве.
     * Если переменная с таким именем не найдена, возвращается нулевой указатель: ((config_variable_t*)0) */
    parser_variable_t* config_parser_get_var(const struct ConfigParser *Inst, const char *Grp, const char *Var) { return ((parser_variable_t*)0); }
    /** Сканирует значение как целое со знаком 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_i(void *Val, char *Var) { return -1; }
    /** Сканирует значение как целое без знака 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ui(void *Val, char *Var) { return -1; }
    /** Сканирует значение как целое со знаком 8 байт и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_l(void *Val, char *Var) { return -1; }
    /** Сканирует значение как целое без знака 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ul(void *Val, char *Var) { return -1; }
    /** Сканирует значение как вещественное 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_f(void *Val, char *Var) { return -1; }
    /** Сканирует значение как вещественное 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_d(void *Val, char *Var) { return -1; }
    // Возвращает указатель на первый символ после ковычки
    const char *config_parser_parse_str(void *Val, char *Var) { return nullptr; }

    /** */
    int config_parser_count_default(ConfigParser *Inst) {
        size_t pi = 0;
        const unsigned char *in = Inst->__private_src;
        const size_t SrcSz = Inst->__private_srcsz;
        size_t *Grps = (size_t*)&(Inst->__private_gc);
        size_t *Vars = (size_t*)&(Inst->__private_vc);
        int equals = 0;
        int tabs = 0;
        int same = 0;
        int result = 0;

        /**
         * анализируем первую строку если в ней присутсвует один знак равно, значит это просто набор переменных
         * если нет ни того ни другого, то скорее всего это имя группы, причём в квадратных скобках
         * а если знаков равно много, да ещё и с табуляцией, то, тогда каждая строка - это отдельная группа
         * в этом мы убедимся при дальнейшем разборе, а может и не станем заморачиваться =) */
        for (int i = 0; (0 != in[i]) && (i < SrcSz); i++) {
            if ('=' < in[i]) {}
            // знак равно
            else if ('=' == in[i]) { ++equals; }
            // новая строка, указатель сохраним =)
            else if ('\n' == in[i]) { break; }
            else if (' ' < in[i]) { errno = EILSEQ; return -1; }
        }
        /*  теперь смотрим что у нас получилось... */
        if (!equals) {
            config_parser_set_flag(Inst, CP_FLAG_CTPD1);
        }
        else if ((1 == equals)/* && !(tabs) */) {
            config_parser_set_flag(Inst, CP_FLAG_CTPD0);
        }
        return result;
    }
    int config_parser_scan_var(unsigned char **v, unsigned char *s, size_t *p) {
        int r = -1;
        size_t l = 0;
        *v = (s + *p);
        while (
            (('a' <= s[*p]) && ('z' >= s[*p]))
            || (('A' <= s[*p]) && ('Z' >= s[*p]))
            || (('0' <= s[*p]) && ('9' >= s[*p]))
            || ('_' == s[*p])
        ) {
            ++l;
            ++(*p);
        }
        if (((0 < l) && ('=' == s[*p]))) {
            s[(*p)++] = 0;
            r = 0;
        }
        else { errno = EILSEQ; }
        return r;
    }
    int config_parser_scan_val(unsigned char **v, unsigned char *s, size_t *p) {
        char *t;
        int r = 0;
        char Q;
        t = strchr("\"'`", s[*p]);
        if (nullptr != t) {
            Q = *t;
            s[(*p)++] = 0;
            *v = (s + *p);
            while ((0 != s[*p]) && ((Q != s[*p]) && ('\\' != s[(*p) - 1]))) { ++(*p); }
            if (0 != s[*p]) {
                s[(*p)++] = 0;
                r = 0;
            }
        }
        else {
            size_t c = *p;
            for ( ; (0 != s[*p]) && ('\n' != s[*p]) && ('\t' != s[*p]); (*p)++) {
                if (('A' <= s[*p]) && ('E' >= s[*p]) || ('X' == s[*p]));
                else if (('a' <= s[*p]) && ('e' >= s[*p]) || ('x' == s[*p]));
                else if (('0' <= s[*p]) && ('9' >= s[*p]));
                else if (
                    ('.' == s[*p]) || ('+' == s[*p])
                    || ('-' == s[*p]) || ('*' == s[*p])
                );
                else {
                    errno = EILSEQ;
                    r = -1;
                    break;
                }
                if (!r) s[(*p)++] = 0;
            }
        }
        return r;
    }
    int config_parser_scan_grp(unsigned char **g, unsigned char *s, size_t *p) {
        int r = -1;
        if (s[*p] == '[') {
            s[(*p)++] = 0;
            *g = (s + *p);
            while ((0 != s[*p]) &&((('a' <= s[*p]) && ('z' >= s[*p]))
                || (('A' <= s[*p]) && ('Z' >= s[*p]))
                || (('0' <= s[*p]) && ('9' >= s[*p]))
                || ('_' == s[*p]))) {
                ++(*p);
            }
            if (']' == s[*p]) {
                s[(*p)++] = 0;
                r = 0;
            }
        }
        if (0 != r) errno = EILSEQ;
        return r;
    }
