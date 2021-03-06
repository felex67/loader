#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __config_parser_c__ 1

#include "headers/parser.h"

#ifndef nullptr
    #define nullptr (0)
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
    ConfigParser* new_config_parser(ConfigParser *Ptr) {
        ConfigParser (*Inst) = Ptr;
        if (nullptr == Inst) {
            Inst = (ConfigParser*)malloc(sizeof(ConfigParser));
        }
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
            *((void*(*))&Inst->destruct) = (void*)config_parser_destruct;
            *((void*(*))&Inst->reset) = (void*)config_parser_reset;
            *((int*(*))&Inst->init) = (int*)config_parser_init_source;
            *((int**)&Inst->set_building_method) = (int*)config_parser_set_building_method;

            *((int**)&Inst->parse_i) = (int*)config_parser_parse_i;
            *((int**)&Inst->parse_ui) = (int*)config_parser_parse_ui;
            *((int**)&Inst->parse_l) = (int*)config_parser_parse_l;
            *((int**)&Inst->parse_ul) = ((int*)config_parser_parse_ul);
            *((int**)&Inst->parse_f) = ((int*)config_parser_parse_f);
            *((int**)&Inst->parse_d) = ((int*)config_parser_parse_d);
            *((char**)&Inst->parse_str) = ((char*)config_parser_parse_str);

            *((int**)&Inst->__private_init_map) = ((int*)config_parser_init_map);
            *((int**)&Inst->__private_count) = ((int*)config_parser_count_map);
            *((int**)&Inst->__private_build) = ((int*)config_parser_build_map);
            *((int**)&Inst->__private_counter) = (int*)nullptr;
            *((int**)&Inst->__private_builder) = (int*)nullptr;

            *((size_t*)&(Inst->__private_gc)) = *((size_t*)&(Inst->__private_vc)) = *((size_t*)&(Inst->__private_srcsz)) = 0;
            *((void**)&(Inst->__private_src)) = (void*)nullptr;
            *((void**)&(Inst->__private_map)) = (void*)nullptr;
            *((void**)&(Inst->__private_vars)) = (void*)nullptr;
            *((u_int32_t*)&(Inst->__private_flags)) = 0;
            *((int*)&(Inst->Error)) = 0;

            config_parser_set_flag(Inst, CP_FLAG_PREINT);
            
            return 0;
        }
        else {
            errno = EINVAL;
        }
        return -1;
    }
    /** */
    int config_parser_construct_var(const parser_variable_t *Var, const char *N, const char *V) {
        if (Var) {
            errno = EXIT_SUCCESS;
            *(const char**)&Var->Var = N;
            *(const char**)&Var->Val = V;
            return 0;
        }
        errno = EINVAL;
        return -1;
    }
    /** */
    int config_parser_construct_grp(const parser_group_t *Grp, const char *N, const size_t Cnt, const parser_variable_t *Vars) {
        if (Grp) {
            errno = EXIT_SUCCESS;
            *((const char**)&Grp->Name) = N;
            *((size_t*)&Grp->VarCnt) = Cnt;
            *((const parser_variable_t**)&Grp->Vars) = Vars;
            return 0;
        }
        errno = EINVAL;
        return -1;
    }
    /** Инициализатор исходного массива */
    int config_parser_init_source(ConfigParser *Inst, const char *File) {
        ConfigCleaner *Cleaner;
        int result = -1;
        if ((nullptr != Inst) && (nullptr != File)) {
            if (Inst->__private_flags.SRCINT) config_parser_destruct_src(Inst);
            if (nullptr != (Cleaner = new_config_cleaner(0))) {
                if (-1 != (result = Cleaner->load(Cleaner, File))) {
                    if (-1 != (result = Cleaner->release(Cleaner, (void**)&Inst->__private_src, (size_t*)&Inst->__private_srcsz))) {
                        Cleaner->resetBuff(Cleaner);
                        config_parser_set_flag(Inst, CP_FLAG_SRCINT);
                        if (-1 != (result = Inst->__private_count(Inst))) {
                            if (-1 != (result = config_parser_init_map(Inst))) {
                                result = Inst->__private_build(Inst);
                            }
                        }
                    }
                }
                Cleaner->destruct(Cleaner);
            }
        }
        else {
            errno = EINVAL;
        }
        return result;
    }
    /** Инициализатор-конструктор выходного массива */
    int config_parser_init_map(ConfigParser *Inst) {
        size_t gbytes = 0, vbytes = 0;
        unsigned char *barray = nullptr;
        parser_group_t *pGrps = nullptr;
        parser_variable_t *pVars = nullptr;
        int result = -1;

        if ((nullptr != Inst) && (Inst->__private_flags.SRCINT)) {
            gbytes = (sizeof(parser_group_t) * Inst->__private_gc);
            vbytes = (sizeof(parser_variable_t) * Inst->__private_vc);
            barray = (unsigned char*)malloc(gbytes + vbytes);
            if ((0 != gbytes) && (0 != vbytes) && (nullptr != barray)) {
                pGrps = (parser_group_t*)barray;
                pVars = *((parser_variable_t**)&Inst->__private_vars) = (parser_variable_t*)(barray + gbytes);
                *((void**)&(Inst->__private_map)) = (void*)barray;
                for (size_t i = 0; i < Inst->__private_gc; i++) {
                    if (-1 != (result = config_parser_construct_grp(pGrps + i, nullptr, 0, nullptr)));
                    else { break; }
                }
                if (!result && (-1 != (result = config_parser_construct_grp(pGrps, nullptr, 0, pVars)))) {
                    for (size_t i = 0; i < Inst->__private_vc; i++) {
                        if (-1 != (result = config_parser_construct_var(pVars + i, nullptr, nullptr)));
                        else { break; }
                    }
                }
                if (!result) {
                    config_parser_set_flag(Inst, CP_FLAG_MAPINT);
                    return 0;
                }
            }
        }
        else { errno = EINVAL; }
        return -1;
    }
    /** Возвращает интерпритатор в исходное состояние */
    int config_parser_reset(ConfigParser *Inst) {
        if (nullptr != Inst) {
            errno = EXIT_SUCCESS;
            u_int32_t flags = Inst->__private_flags.DYNINS;
            config_parser_preinit_instance(Inst);
            config_parser_set_flag(Inst, flags);
            return 0;
        }
        else { errno = EINVAL; }
        return -1;
    }

/** Деструкторы */

    /** Деструктор интерпритатора */
    void config_parser_destruct(ConfigParser *Inst) {
        if (nullptr != Inst) {
            if (Inst->__private_flags.SRCINT) free(*((void**)&Inst->__private_src));
            if (Inst->__private_flags.MAPINT) free(*((void**)&Inst->__private_map));
            if (Inst->__private_flags.DYNINS) free(Inst);
        }
    }
    /** Освобождает массив */
    void config_parser_destruct_src(ConfigParser *Inst) {
        if (nullptr != Inst) {
            if (Inst->__private_flags.SRCINT) free(*((void**)&Inst->__private_src));
            *((void**)&(Inst->__private_src)) = nullptr;
            config_parser_unset_flag(Inst, CP_FLAG_SRCINT);
            if (Inst->__private_flags.MAPINT) config_parser_destruct_map(Inst);
        }
    }
    /** Освобождает только карту */
    void config_parser_destruct_map(ConfigParser *Inst) {
        if (nullptr != Inst) {
            if (Inst->__private_flags.MAPINT) free(*((void**)&Inst->__private_map));
            *((void**)&(Inst->__private_map)) = nullptr;
            config_parser_unset_flag(Inst, CP_FLAG_MAPINT | CP_FLAG_MAPBLD);
        }
    }

/** Основные методы */

    int config_parser_count_map(ConfigParser *Inst) {
        int result = -1;
        if ((nullptr != Inst) && (Inst->__private_flags.SRCINT)) {
            if (!Inst->__private_flags.BTPUSR) {
                result = config_parser_count_default(Inst);
            }
            if (!result) {
                result = Inst->__private_counter(Inst->__private_src, Inst->__private_srcsz, (size_t*)&(Inst->__private_gc), (size_t*)&(Inst->__private_vc));
            }
        }
        else { errno = EINVAL; }
        return result;
    }
    /** */
    int config_parser_count_default(ConfigParser *Inst) {
        size_t pi = 0;
        const unsigned char *in = Inst->__private_src;
        const size_t SrcSz = Inst->__private_srcsz;
        int equals = 0;
        int tabs = 0;
        int same = 0;
        int result = 0;

        /**
         * анализируем первую строку если в ней присутсвует один знак равно, значит это просто набор переменных
         * если нет ни того ни другого, то скорее всего это имя группы, причём в квадратных скобках
         * а если знаков равно много, да ещё и с табуляцией, то, тогда каждая строка - это отдельная группа
         * в этом мы убедимся при дальнейшем разборе, а может и не станем заморачиваться =) */
        for (size_t i = 0; (0 != in[i]) && (i < SrcSz); i++) {
            if ('=' < in[i]) {}
            // знак равно
            else if ('=' == in[i]) { ++equals; }
            // новая строка, указатель сохраним =)
            else if ('\n' == in[i]) { break; }
            else if (' ' > in[i]) { errno = EILSEQ; return -1; }
        }
        /*  теперь смотрим что у нас получилось... */
        if (!equals) {
            config_parser_set_flag(Inst, CP_FLAG_BTPDT1);
            *((int**)&(Inst->__private_counter)) = ((int*)config_parser_count_default1);
            *((int**)&(Inst->__private_builder)) = ((int*)config_parser_build_default1);
        }
        else if ((1 == equals)/* && !(tabs) */) {
            config_parser_unset_flag(Inst, CP_FLAG_BTPDT1);
            *((int**)&(Inst->__private_counter)) = ((int*)config_parser_count_default0);
            *((int**)&(Inst->__private_builder)) = ((int*)config_parser_build_default0);
        }
        return result;
    }
    /** Строит карту групп и переменных:
     * Если не установлен пользовательсий метод,
     * то используются метод по умолчанию */
    int config_parser_build_map(ConfigParser *Inst) {
        int result = -1;
        if (Inst->__private_flags.MAPINT && Inst->__private_flags.SRCINT) {
            result = Inst->__private_builder (
                *((parser_group_t**)&Inst->__private_map)
                , *((unsigned char**)&Inst->__private_src)
                , Inst->__private_srcsz - 1
                , Inst->__private_gc
                , Inst->__private_vc
            );
            if (!result) {
                config_parser_set_flag(Inst, CP_FLAG_MAPBLD);
            }
        }
        else { errno = EINVAL; }
        return result;
    }
    int config_parser_scan_var(const parser_variable_t *pv, unsigned char *s, size_t *p) {
        int r = -1;
        if (pv && s && p) {
            // need for asign name
            size_t np = *p;
            size_t l = 0;
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
                // here
                *((unsigned char**)&pv->Var) = (s + np);
                s[(*p)++] = 0;
                r = 0;
            }
            else { errno = EILSEQ; }
        }
        else { errno = EINVAL; }
        return r;
    }
    int config_parser_scan_val(const parser_variable_t *v, unsigned char *s, size_t *p) {
        char Qts[] = "\"'`";
        char *t = nullptr;
        // need for asign var
        size_t vp = *p;
        int r = 0;
        char Q = 0;
        if (v && s && p) {
            t = strchr(Qts, (char)s[*p]);
            if (nullptr != t) {
                Q = *t;
                s[(*p)++] = 0;
                *((unsigned char**)&v->Val) = (s + vp + 1);
                while ((0 != s[*p]) && ((Q != s[*p]) || ('\\' == s[(*p) - 1]))) { ++(*p); }
                if (Q == s[(*p)]) s[(*p)++] = 0;
            }
            else {
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
                }
                if (!r) {
                    *((unsigned char**)&v->Val) = (s + vp);
                    s[(*p)++] = 0;
                }
            }
        }
        else { errno = EINVAL; }
        return r;
    }
    int config_parser_scan_grp(const parser_group_t *g, unsigned char *s, size_t *p) {
        int r = -1;
        size_t np = 0;
        if (g && s && p) {
            if (s[*p] == '[') {
                s[(*p)++] = 0;
                np = *p;
                while ((0 != s[*p]) &&((('a' <= s[*p]) && ('z' >= s[*p]))
                    || (('A' <= s[*p]) && ('Z' >= s[*p]))
                    || (('0' <= s[*p]) && ('9' >= s[*p]))
                    || ('_' == s[*p])))
                {
                    ++(*p);
                }
                if (']' == s[*p]) {
                    *((unsigned char**)&g->Name) = s + np;
                    s[(*p)++] = 0;
                    r = 0;
                }
            }
            if (0 != r) errno = EILSEQ;
        }
        else { errno = EINVAL; }
        return r;
    }
    /** Считаем количество переменных в файле формата:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    int config_parser_count_default0(const unsigned char *s, const size_t sz, size_t *g, size_t *v) {
        if (s && sz && g && v) {
            errno = EXIT_SUCCESS;
            for (size_t i = 0; i < sz; i++) {
                if ('\n' == s[i]) { ++(*v); }
            }
            if ('\n' != s[sz - 1]) ++(*v);
            (*g) = 1;
            return 0;
        }
        errno = EINVAL;
        return -1;
    }
    /** Считаем количество переменных в файле формата:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_parser_count_default1(const unsigned char *Buff, const size_t SrcSz, size_t *Grps, size_t *Vars) {
        unsigned char gprstart[] = { '\n', 0x00, 0x00 };
        size_t gslen = 0;
        unsigned char *in;
        size_t pi = 1;
        int result = -1;
        if (Buff && SrcSz && Grps && Vars) {
            in = *(unsigned char **)&Buff;
            gprstart[1] = Buff[0];
            gslen = strlen((const char*)gprstart);
            ++(*Grps);
            
            for (pi = 1; 0 != in[pi]; pi++) {
                if ((gprstart[0] != in[pi]) && ('=' != in[pi])) {}
                else if ('=' == in[pi]) {
                    while ((pi < SrcSz) && ('\n' != in[pi])) { ++pi; }
                    --pi; ++(*Vars);
                }
                else if ((gprstart[0] == in[pi]) && (0 == strncmp((char*)gprstart, (char*)(in + pi), gslen))) {
                    pi += gslen;
                    while ((pi < SrcSz) && ('\n' != in[pi])) { ++pi; }
                    --pi; ++(*Grps);
                }
            }
            errno = result = EXIT_SUCCESS;
        }
        errno = EINVAL;
        return result;
    }
    /** Создаёт структуру из конфига вида:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    int config_parser_build_default0(parser_group_t *m, unsigned char *s, const size_t ssz, const size_t gc, const size_t vc) {
        const parser_variable_t *v;
        size_t p = 0;
        int r = -1;
        if (m && s && ssz && gc && vc) {
            r = 0;
            v = m->Vars;
            *((size_t*)&(m->VarCnt)) = vc;
            // поехали!!!
            for (size_t i = 0; i < vc; i++) {
                if (-1 != (r = config_parser_scan_var(v + i, s, &p))) {
                    if (-1 != (r = config_parser_scan_val(v + i, s, &p)));
                    else { break; }
                    if ('\n' == s[p]) s[p++] = 0;
                }
            }
        }
        else { errno = EINVAL; }
        return r;
    }
    /** Создаёт структуру из конфига вида:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_parser_build_default1(parser_group_t *m, unsigned char *s, const size_t Sz, const size_t gc, const size_t vc) {
        const parser_variable_t *vs = m->Vars;
        size_t p = 0;
        int r = 0;
        for (size_t g = 0; (p < Sz) && (g < gc) && !r; g++) {
            if (s[p] == '[') {
                *((const parser_variable_t**)&m[g].Vars) = vs;
                if (!(r = config_parser_scan_grp(m + g, s, &p))) {
                    for (size_t v = 0; (p < Sz) && ('[' != s[p]) && !r; v++) {
                        if ('\n' == s[p])s[p++] = 0;
                        if (!(r = config_parser_scan_var(m[g].Vars + v, s, &p))) {
                            r = config_parser_scan_val(m[g].Vars + v, s, &p);
                            if ('\n' == s[p]) s[p++] = 0;
                            if (!r) {
                                ++(*(size_t*)&(m[g].VarCnt));
                                ++vs;
                            }
                        }
                    }
                }
            }
            else if (s[p]) {
                errno = EILSEQ;
                r = -1;
            }
        }
        return r;
    }

/** Установщики */

    /** Устанавливает флаги */
    void config_parser_set_flag(ConfigParser* Inst, u_int32_t Flags) {
        if (nullptr != Inst) {
            *((u_int32_t*)&Inst->__private_flags) |= Flags;
        }
    }
    /** Снимает флаги */
    void config_parser_unset_flag(ConfigParser* Inst, u_int32_t Flags) {
        if (nullptr != Inst) {
            u_int32_t *flags = (u_int32_t*)&(Inst->__private_flags);
            (*flags) |= (Flags & 0xffffffff);   // чтобы не получилось что мы не удалим, а - поставим флаг
            (*flags) ^= (0xffffffff & Flags);
        }
    }

    /** Устанавливает метод подсчёта групп и переменных */
    int config_parser_set_building_method (
        ConfigParser *Inst,
        int (*counter)(const unsigned char *Src, const size_t SrcSz, size_t *GrpCnt, size_t *VarCnt),
        int (*builder)(parser_group_t *Map, unsigned char *Src, const size_t SrcSz, const size_t GrpCnt, const size_t VarCnt)
    ) {
        if ((nullptr != Inst) && (nullptr != counter) && (nullptr != builder)) {
            *((int**)&(Inst->__private_counter)) = (int*)counter;
            *((int**)&(Inst->__private_builder)) = (int*)builder;
            config_parser_set_flag(Inst, CP_FLAG_BTPUSR);
            return 0;
        }
        errno = EINVAL;
        return -1;
    }

/** Вывод */

    /** Выполняет поиск переменой в массиве.
     * Если переменная с таким именем не найдена, возвращается нулевой указатель: ((config_variable_t*)0) */
    const parser_variable_t* config_parser_get_var(const ConfigParser *Inst, const char *Var, const char *Grp) {
        const parser_variable_t *v = nullptr;
        if (Inst && Var && Grp) {
            for (size_t ig = 0; !v && (ig < Inst->__private_gc); ig++) {
                parser_group_t *g = (*(parser_group_t**)&Inst->__private_map) + ig;
                if (g->Name && !strcmp(g->Name, Grp)) {
                    for (size_t iv = 0; iv < g->VarCnt; iv++) {
                        if (!strcmp(g->Vars[iv].Var, Var)) {
                            v = g->Vars + iv;
                            break;
                        }
                    }
                }
            }
            if (!v) { errno = EINVAL; }
        }
        else if (Inst && Var) {
            for (size_t i = 0; i < Inst->__private_vc; i++) {
                if (!strcmp(Var, Inst->__private_vars[i].Var)) {
                    v = (parser_variable_t*)(Inst->__private_vars + i);
                    break;
                }
            }
            if (!v) { errno = EINVAL; }
        }
        else {
            errno = EINVAL;
        }
        return v;
    }
    /** Сканирует значение как целое со знаком 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_i(int *T, const ConfigParser *I, const char *V, const char *G) {
        if (T && I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return sscanf(v->Val, "%i", T);
            }
        }
        errno = EINVAL;
        return -1;
    }
    /** Сканирует значение как целое без знака 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ui(unsigned int *T, const ConfigParser *I, const char *V, const char *G) {
        if (T && I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return sscanf(v->Val, "%u", T);
            }
        }
        errno = EINVAL;
        return -1;
    }
    /** Сканирует значение как целое со знаком 8 байт и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_l(long *T, const ConfigParser *I, const char *V, const char *G) {
        if (T && I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return sscanf(v->Val, "%li", T);
            }
        }
        errno = EINVAL;
        return -1;
    }
    /** Сканирует значение как целое без знака 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ul(unsigned long *T, const ConfigParser *I, const char *V, const char *G) {
        if (T && I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return sscanf(v->Val, "%lu", T);
            }
        }
        errno = EINVAL;
        return -1;
    }
    /** Сканирует значение как вещественное 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_f(float *T, const ConfigParser *I, const char *V, const char *G) {
        if (T && I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return sscanf(v->Val, "%f", T);
            }
        }
        errno = EINVAL;
        return -1;
    }
    /** Сканирует значение как вещественное 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_d(double *T, const ConfigParser *I, const char *V, const char *G) {
        if (T && I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return sscanf(v->Val, "%lf", T);
            }
        }
        errno = EINVAL;
        return -1;
    }
    // Возвращает указатель на первый символ после ковычки
    const char *config_parser_parse_str(const ConfigParser *I, const char *V, const char *G) {
        if (I && V) {
            const parser_variable_t* v = config_parser_get_var(I, V, G);
            if (v) {
                return v->Val;
            }
        }
        errno = EINVAL;
        return 0;
    }
