#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define __config_loader_c__ 1

#include "loader_v2.h"

#ifndef nullptr
    #define nullptr ((0))
#endif // nullptr

/** class __config_loader_var_groups */

    struct __config_loader_var __config_loader_init_struct_var = {
        .Var = nullptr,
        .Val = nullptr
    };
    struct __config_loader_var_group __config_loader_init_struct_group = {
        .count = 0,
        .Name = nullptr,
        .Vars = nullptr
    };
    struct __config_loader_var_groups __config_loader_init_struct_groups = {
        .destruct = config_loader_destruct_var_groups,

        .GrpCnt = 0,
        .VarCnt = 0,
        .Groups = nullptr,
    };
    /* деструктор */
    void config_loader_destruct_var_groups(struct __config_loader *Inst) {
        free(Inst->Config.Groups);
    }

/************************************** class __config_loader **************************************/
/* Конструктор */
    ConfigLoader __config_loader_init_struct = {
        /* методы */
        .destruct = config_loader_destruct,
        .reset = config_loader_reset,
        .load = config_loader_process_file,
        .set_string_comments = config_loader_set_scomments,
        .perror = config_loader_perror
    };
    ConfigLoader* config_loader_construct(void *Ptr) {
        ConfigLoader *Inst = (ConfigLoader*)Ptr;
        if (nullptr == Inst) { Inst = (ConfigLoader *)malloc(sizeof(ConfigLoader)); }
        if (nullptr != Inst) {
            Inst->Buffers = __config_loader_init_struct_buffers;
            Inst->Errors = __config_loader_init_struct_errors;
            Inst->Errors.unFl.s_flags.DynInst = ((nullptr == Ptr) ? 1 : 0);

            *((char***)&(Inst->Result)) = (char**)&(Inst->Buffers.In);
            *((size_t**)&(Inst->Size)) = &(Inst->Buffers.iSz);
            config_loader_construct_charset(&Inst->Charset);
        }
        return Inst;
    }
/* Деструктор */
    void config_loader_destruct(struct __config_loader *Inst) {
        Inst->reset(Inst);
        if (Inst->Errors.unFl.s_flags.DynInst) free(Inst);
    }

/* Методы */

    /* void config_loader::clear(): освобождает буфферы */
    void config_loader_reset(ConfigLoader *Inst) {
        if (Inst->Errors.unFl.s_flags.BuffInit) {
            Inst->Buffers.destruct(Inst);
            Inst->Buffers = __config_loader_init_struct_buffers;
            *((size_t*)&(Inst->Size)) = 0;
            *((char**)&(Inst->Result)) = nullptr;
            Inst->Errors.unFl.ui32 = 0;
        }
        config_loader_construct_charset(Inst);
        return;
    }
    /* int load(): Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые */
    int config_loader_process_file(struct __config_loader *Inst, const char* FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        int *error = &(Inst->Errors.error);
        struct __config_loader_buffers *Buff = &(Inst->Buffers);
        //проверяем файл на наличие
        if (-1 == (*error = stat(FileName, &statbuf))) {
            config_loader_perror(Inst, FileName);
            return -1;
        }
        Buff->iSz = statbuf.st_size;
        //зфагружаем в буффер
        if (-1 == (*error = config_loader_init_buffers(Inst, FileName))) {
            config_loader_perror(Inst, "config_loader_init_buffer()");
            return -1;
        }
        else { Inst->Errors.unFl.s_flags.BuffInit = 1; }
        // вычищаем файл
        config_loader_clean_file(Inst);
        config_loader_delete_emty_strings(&(Inst->Buffers));
        if (!Inst->Errors.error) {
            Inst->Errors.unFl.s_flags.FileClear = 1;
        }

        return 0;
    }
    /* int isInit(): 1 - если успешно загружен, 0 - в случае ошибки */
    int config_loader_isInit(struct __config_loader *Inst) {
        return (Inst->Errors.unFl.s_flags.BuffInit);
    }
    /* int set_string_comments(): устанавливает пользовательский набор строковых комментариев */
    int config_loader_set_scomments(struct __config_loader *I, const char *Keys) {
        struct __config_loader_charset *CS = &(I->Charset);
        if (nullptr != Keys) {
            strncpy((char*)CS->LineComm, Keys, sizeof(CS->LineComm) - 1);
            ((char*)CS->LineComm)[sizeof(CS->LineComm) - 1] = 0;
            config_loader_sort_charset(CS);
            return 0;
        }
        errno == EINVAL;
        return -1;
    }
    // отчищает файл от комментариев
    int config_loader_clean_file(struct __config_loader *I) {
        if (!I->Errors.unFl.s_flags.BuffInit) return -1;
        // pos & size
        size_t *pi = &(I->Buffers.Pi), *pw = &(I->Buffers.Pw), *iSz = &(I->Buffers.iSz);
        // buffers
        unsigned char *in = I->Buffers.In, *wrk = I->Buffers.Wrk;
        // chars
        char *Qts = (char*)I->Charset.Quotes;
        char *Com = (char*)I->Charset.LineComm;
        char *NSp = (char*)I->Charset.NotSpace;
        // max chars
        char *gQ = (char*)&(I->Charset.gQ), *gC = (char *)&(I->Charset.gC);
        char *tchar;
        unsigned char Q = 0;
        unsigned char mchar = (unsigned char)(*gQ < *gC ? *gC : *gQ);
        for (*pi = 0, *pw = 0; *pi < *iSz; (*pi)++) {
            // помимо тех что больше максимального искомого символа исключаем алфавит и цифры
            if ((in[*pi] > mchar) || ('A' <= in[*pi] && 'Z' >= in[*pi]) || (('0' <= in[*pi]) && ('9' >= in[*pi]))) {
                wrk[(*pw)++] = in[*pi];
            }
            // это ковычка
            else if (nullptr != (tchar = strchr(Qts, in[*pi]))) {
                do {
                    wrk[(*pw)++] = in[(*pi)++];
                } while ((0 != in[*pi]) && (('"' != in[*pi]) || ('\\' == in[(*pi) - 1])));
                wrk[(*pw)++] = in[*pi];
            }
            // это строковый коммент
            else if (nullptr != (tchar = strchr(Com, in[*pi]))) {
                while ('\n' != in[*pi]) { ++(*pi); }
                wrk[(*pw)++] = in[*pi];
            }
            // возможно Си коммент 
            else if (in[*pi] == '/') {
                if ('/' == in[(*pi) + 1]) {
                    while ('\n' != in[*pi]) { ++(*pi); }
                    wrk[(*pw)++] = in[*pi];
                }
                else if ('*' == in[(*pi) + 1]) {
                    do { (*pi++); } while (('*' != in[*pi]) && ('/' != in[(*pi) + 1]));
                    ++(*pi);
                }
                else {
                    wrk[(*pw)++] = in[*pi];
                }
            }
            // необходимый служебный символ: '\t' || '\n'
            else if ((' ' < in[(*pi)]) || (nullptr != (tchar = strchr(NSp, in[*pi])))) { wrk[(*pw)++] = in[*pi]; }
        }
        config_loader_swap_buffers(&(I->Buffers));
        return 0;
    }
    // удаляет пустые строки
    int config_loader_delete_emty_strings(struct __config_loader_buffers *I) {
        size_t len = 0;
        for (I->Pw = I->Pi = 0; 0 != I->In[I->Pi]; (I->Pi)++) {
            if('\n' != I->In[I->Pi]) { I->Wrk[(I->Pw)++] = I->In[I->Pi]; ++len; }
            else if (len > 0){ I->Wrk[(I->Pw)++] = I->In[I->Pi]; len = 0; }
        }
        I->In[I->Pw] = 0;
        if ('\n' == I->Wrk[I->Pw - 1]) { I->Wrk[--(I->Pw)] = 0; }
        config_loader_swap_buffers(I);
        return 0;
    }
    // меняет буфферы местами
    void config_loader_swap_buffers(struct __config_loader_buffers *b) {
        unsigned char *t = b->In;
        b->In = b->Wrk;
        b->Wrk = t;
        b->iSz = b->Pw;
        b->In[b->Pw] = 0;
        b->Pi = b->Pw = 0;
    }
/* end of class __config_loader */

/********************************* struct __config_loader_charset *********************************/
/* значения по умолчанию */
    const char __config_loader_default_quotes[] = "`'\"";
    const char __config_loader_default_lcomm[] = ";#";
    const char __config_loader_default_nonspace[] = "\n\t";
    const char __config_loader_default_group_begin[] = "[";
    const char __config_loader_default_group_end[] = "[";
/* Конструктор */
    int config_loader_construct_charset(struct __config_loader_charset *CS) {
        strncpy((char*)CS->NotSpace, __config_loader_default_nonspace, sizeof(CS->NotSpace) - 1);
        strncpy((char*)CS->LineComm, __config_loader_default_lcomm, sizeof(CS->LineComm) - 1);
        strncpy((char*)CS->Quotes, __config_loader_default_quotes, sizeof(CS->Quotes) - 1);
        *(char*)&(CS->NotSpace[sizeof(CS->NotSpace) - 1]) = 0;
        *(char*)&(CS->LineComm[sizeof(CS->LineComm) - 1]) = 0;
        *(char*)&(CS->Quotes[sizeof(CS->Quotes) - 1]) = 0;
        config_loader_sort_charset(CS);
        return 0;
    }
    // сортирует символы и выставляет максимумы
    int config_loader_sort_charset(struct __config_loader_charset *CS) {
        config_loader_strsort_za((char*)CS->NotSpace);
        config_loader_strsort_za((char*)CS->LineComm);
        config_loader_strsort_za((char*)CS->Quotes);
        *(char*)&(CS->gQ) = CS->Quotes[0];
        *(char*)&(CS->gC) = CS->LineComm[0];
        return 0;
    }

/********************************* struct __config_loader_errors **********************************/

    struct __config_loader_errors __config_loader_init_struct_errors = {
        .Buff = { 0 },
        .error = 0,
        .unFl = { 0 }
    };
    // выводит ошибки в терминал
    void config_loader_perror(struct __config_loader *Inst, const char * What) {
        errno = Inst->Errors.error;
        sprintf((char *)Inst->Errors.Buff, "ERROR [%i] '%s'", Inst->Errors.error, What);
        perror((char *)Inst->Errors.Buff);
    }

/********************************* class __config_loader_buffers **********************************/

    struct __config_loader_buffers __config_loader_init_struct_buffers = {
        .destruct = config_loader_destruct_buffers,
        .reset = config_loader_reset_buffers,

        .In = 0,
        .Wrk = 0,
        .iSz = 0,
        .Pi = 0,
        .Pw = 0
    };
    // инициализирует буфферы и считывает в них файл
    int config_loader_init_buffers(ConfigLoader *Inst, const char *FileName) {
        struct __config_loader_buffers *B = &(Inst->Buffers);
        Inst->Errors.error = 0;
        int In;
        if (!B->iSz) return -1;
        if (B->In = (unsigned char *)malloc(B->iSz + 1)) {
            if (B->Wrk = (unsigned char *)malloc(B->iSz + 1)) {
                if (-1 != (In = open(FileName, O_RDONLY))) {
                    size_t readed = read(In, B->In, B->iSz);
                    Inst->Errors.error = errno;
                    close(In);
                    if (readed == B->iSz) {
                        Inst->Errors.unFl.s_flags.BuffInit = 1;
                        Inst->Errors.error = 0;
                        B->In[B->iSz] = 0;
                    }
                    else { Inst->perror(Inst, "init_buffer()->fread()"); }
                }
                else { Inst->Errors.error = errno; Inst->perror(Inst, "init_buffer()->fopen()"); }
            }
            else { Inst->Errors.error = errno; Inst->perror(Inst, "init_buffer()->malloc(wBuffer)"); }
        }
        else { Inst->Errors.error = errno; Inst->perror(Inst, "init_buffer()->malloc(iBuffer)"); }
        if (Inst->Errors.error) {
            Inst->Buffers.destruct(Inst);
        }
        return Inst->Errors.error;
    }
    /* void __config_loader_buffers::destruct() */
    void config_loader_destruct_buffers(ConfigLoader *Inst) {
        if (Inst->Errors.unFl.s_flags.BuffInit) {
            free(Inst->Buffers.In);
            free(Inst->Buffers.Wrk);
        }
        return;
    }
    /* void __config_loader_buffers::reset() */
    void config_loader_reset_buffers(ConfigLoader *Inst) {
        struct __config_loader_buffers *Buff = &(Inst->Buffers);
        
        if (Buff->In) free(Buff->In);
        if (Buff->Wrk) free(Buff->In);

        Inst->Errors.unFl.s_flags.BuffInit = 0;
        Buff->iSz = Buff->Pi = Buff->Pw = 0;
        Buff->In = Buff->Wrk = nullptr;

        return;
    }
/* end of class __config_loader_buffers */

/*****************************************************************************************************/
/* Функции */
    // меняет символы местами
    int config_loader_swap_c(char *c1, char *c2) {
        char t = *c1;
        *c1 = *c2;
        *c2 = t;
        return 0;
    }
    // сортирует строку по возрастанию
    int config_loader_strsort_az(char *a) {
            int i, fwd, swapped = 1;
            char t;
            if (0 != a) {
                for ( i = 0, fwd = 1; (0 != a[i]); ) {
                    if (a[i + 1]) {
                        if (a[i] > a[i + 1]) {
                            t = a[i];
                            a[i] = a[i + 1];
                            a[i + 1] = t;
                            swapped = 1;
                        }
                    }
                    else {
                        if (swapped) { fwd = 0; }
                        else break;
                    }
                    if (i);
                    else {
                        if (swapped) {
                            swapped = 0;
                            fwd = 1;
                        }
                        else { break; }
                    }
                    (fwd ? ++i : --i);
                }
                return 0;
            }
            return -1;
        }
    // сортирует строку по убыванию
    int config_loader_strsort_za(char *a) {
        int i = 0, fwd = 1, swapped = 0;
        unsigned char t;
        if (0 != a) {
            for ( i = 0, fwd = 1; (0 != a[i]); ) {
                if ((0 != a[i + 1]) && swapped) { fwd = 0; }
                if (a[i] < a[i + 1]) {
                    t = a[i];
                    a[i] = a[i + 1];
                    a[i + 1] = t;
                    swapped = 1;
                }
                if (i);
                else { swapped = 0; fwd = 1; }
                (fwd ? ++i : --i);
            }
            return 0;
        }
        return -1;
    }
    // инвертирует строку(переворачивает)
    int config_loader_strinverse(char *a) {
        if (0 != a) {
            int len = strlen(a);
            for (int i = 0, j = len - 1; i < j; i++, j--) {
                char t = a[i];
                a[i] = a[j];
                a[j] = t;
            }
            return 0;
        }
        return -1;
    }

    int config_loader_construct_var_groups(struct __config_loader *Inst) {
        unsigned char * in = Inst->Buffers.In;
        size_t *pi = &(Inst->Buffers.Pi);
        struct __config_loader_var_groups *Groups = &(Inst->Config);
        unsigned char* SecStr = 0;
        int buildtype = -1;
        int equals = 0;
        int tabs = 0;
        int same = 0;
        Inst->Errors.error = 0;
        /**
         * анализируем первую строку если в ней присутсвует один знак равно, значит это просто набор переменных
         * если нет ни того ни другого, то скорее всего это имя группы, причём в квадратных скобках
         * а если знаков равно много, да ещё и с табуляцией, то, тогда каждая строка - это отдельная группа
         * в этом мы убедимся при дальнейшем разборе, а может и не станем заморачиваться =) */
        for (int i = 0; '\000' != in[i]; i++) {
            if ('=' < in[i]) {}
            // знак равно
            else if ('=' == in[i]) { ++equals; }
            /*/ знак табуляции
            else if ('\t' == in[i]) { ++tabs; } */
            // новая строка, указатель сохраним =)
            else if ('\n' == in[i]) { SecStr = in + i + 1; break; }
        }
        /*  теперь смотрим что у нас получилось... */
        if (!equals) {
            /*  это название группы предположим что начинается с квадратной скобки...
                рассматриваем файл как разделённый на группы, на одну строку одна переменная */
            Inst->Errors.error = config_loader_count_multi1(&(Groups->GrpCnt), &(Groups->VarCnt), in);
            buildtype = 1;
        }
        else if ((1 == equals)/* && !(tabs) */) {
            // это просто набор переменных по одной в строке
            Groups->VarCnt = config_loader_count_simple(in, '\n');
            Groups->GrpCnt = 1;
            buildtype = 0;
        }
        else {
            // даже и предположений никаких нет...
            Inst->Errors.error = EILSEQ;
        }
        if (-1 != buildtype) {
            size_t bcount = (Groups->GrpCnt * sizeof(struct __config_loader_var_group)) + (Groups->VarCnt * sizeof(config_variable_t));
            Inst->Config.Groups = (struct __config_loader_var_group*)malloc(bcount);
            if (nullptr != Inst->Config.Groups) {
                // инициялизируем группы
                for (size_t i = 0; i < Groups->GrpCnt; i++) {
                    Inst->Config.Groups[i] = __config_loader_init_struct_group;
                }
                // записываем адрес начала массива переменных в первую группу
                Inst->Config.Groups->Vars = (config_variable_t*)(Inst->Config.Groups + Groups->GrpCnt);
                // инициализируем переменные
                for (size_t i = 0; i < Groups->VarCnt; i++) {
                    Inst->Config.Groups->Vars[i] = __config_loader_init_struct_var;
                }
                switch (buildtype) {
                case 0:
                    config_loader_var_groups_build0(Inst);
                    break;
                case 1:
                    config_loader_var_groups_build1(Inst);
                    break;
                /*
                case 2:
                    config_loader_var_groups_build2(Inst);
                    break;
                */
                }
            }
            else { Inst->Errors.error = errno; Inst->perror(Inst, "void* malloc(Config.Groups)"); }
        }
        return (Inst->Errors.error ? -1 : 0);
    }
    /** Считаем количество переменных в файле формата:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    size_t config_loader_count_simple(const unsigned char *Buff, const unsigned char Sep) {
        size_t n = 0;
        for (size_t i = 0; Buff[i]; i++) {
            if (Sep != Buff[i]) {}
            else { ++n; }
        }
        return n + 1;
    }
    /** Считаем количество переменных в файле формата:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_loader_count_multi1(size_t *Grps, size_t *Vars, const unsigned char *Buff) {
        unsigned char *in = (unsigned char *)Buff;
        size_t *grps = Grps;
        size_t *vars = Vars;
        size_t pi = 1;
        unsigned char gprstart[] = { '\n', in[0], 0x00 };
        for (pi = 1; 0 != in[pi]; pi++) {
            if ('\n' < in[pi]) {}
            else if (('\n' == in[pi]) && ('[' != in[pi + 1])) { ++pi; ++(*vars); }
            else if (('\n' == in[pi]) && ('[' == in[pi + 1])) { ++pi; ++(*grps); }
        }
        ++(*grps);
        return 0;
    }
    // считаем количество переменных в неименнованных гркппах
    int config_loader_count_multi2(size_t *Grps, size_t *Vars, const unsigned char *Buff) {
        unsigned char *in = (unsigned char *)Buff;
        *Grps = 1;
        *Vars = 0;
        size_t pi = 0;
        const char *Start = (char *)Buff;
        for (pi = 0; 0 != in[pi]; pi++) {
            if ('\n' < in[pi]) {}
            else if ('\n' == in[pi]) { ++(*Grps); }
            else if ('\t' == in[pi]) { ++(*Vars); }
        }
        *Vars -= *Grps;
        return 0;
    }
    /** Создаёт структуру из конфига вида:
     * Var1=Value
     * Var2=Value
     * Var3=Value
     * ... */
    int config_loader_var_groups_build0(struct __config_loader *Inst) {
        // Будем снова копировать =)
        unsigned char *in = Inst->Buffers.In;
        unsigned char *wrk = Inst->Buffers.In;
        size_t pos;
        // создаём указатель на массив переменных
        config_variable_t *v = Inst->Config.Groups->Vars;
        // указатель на кол-во переменных
        size_t *vcnt = &(Inst->Config.VarCnt);
        // поехали!!!
        for (size_t i = 0; i < *vcnt; i++) {
            // инициализируем указатель на имя
            v[i].Var = (char*)(wrk + pos);
            // копируем имя
            while ((0 != in[pos]) && ('=' != in[pos])) { wrk[pos] = in[pos]; ++pos; }
            // затираем равно
            wrk[pos++] = 0;
            // иниуиализируем указатель на значение
            v[i].Val = (char*)(wrk + pos);
            // копируем значение
            while ((0 != in[pos]) && ('\n' != in[pos])) { wrk[pos] = in[pos]; ++pos; }
            // затираем перенос каретки
            wrk[pos++] = 0;
        }
        return 0;
    }
    size_t config_loader_construct_var(config_variable_t *Inst, size_t *pos, const unsigned char *In, unsigned char *Wrk, const char *Qts) {
        char Q = 0, *t = 0;
        while ((0 != In[*pos]) && ('!' > In[*pos])) { Wrk[(*pos)++] = 0; }
        Inst->Var = (char*)(Wrk + (*pos));
        while ((0 != In[*pos]) && ('=' != In[*pos])) { Wrk[*pos] = In[*pos]; ++(*pos); }
        Wrk[(*pos)++] = 0;
        if (nullptr != (t = strchr((char*)Qts, (char)In[*pos]))) { Q = *t; Wrk[(*pos)++] = 0; }
        Inst->Val = (char*)(Wrk + *pos);
        while ((0 != In[*pos]) && ('\n' != In[*pos])) { Wrk[*pos] = In[*pos]; ++(*pos); }
        if (0 != Q) Wrk[(*pos) - 1] = 0;
        Wrk[*pos] = 0;
        return *pos;
    }
    /** Создаёт структуру из конфига вида:
     * [Group]
     * Var1=Value
     * Var2=Value
     * ... */
    int config_loader_var_groups_build1(struct __config_loader *Inst) {
        unsigned char *in = Inst->Buffers.In;
        unsigned char *wrk = Inst->Buffers.Wrk;
        size_t pi = 0;

        struct __config_loader_var_group *Gps = Inst->Config.Groups;
        config_variable_t *Var = Gps->Vars;

        memset(wrk, 0, Inst->Buffers.iSz);

        for (pi = 0; pi < Inst->Buffers.iSz; Gps++) {
            if ('[' == in[pi]) {
                ++pi;
                Gps->Name = (char*)(wrk + pi);
                while (in[pi] != ']') { wrk[pi] = in[pi]; ++pi; }
                in[pi++] = 0;
                Gps->Vars = Var;
                while ((pi < Inst->Buffers.iSz) && ('[' != in[pi])) {
                    config_loader_construct_var(Var, &pi, in, wrk, Inst->Charset.Quotes);
                    ++Var; ++(Gps->count); ++pi;
                }
            }
            else {
                Inst->Errors.error = EILSEQ;
                Inst->perror(Inst, "var_groups_build1()");
                return -1;
            }
        }
        return 0;
    }
    
#ifdef __test_config_loader_c__
    int tostring(struct __config_loader_var_groups *Grps) {
        for (size_t g = 0; g < Grps->GrpCnt; g++) {
            printf("['%s'] Vars: %lu\n", Grps->Groups[g].Name, Grps->Groups[g].count);
            for (size_t v = 0; v < Grps->Groups[g].count; v++) {
                printf("\t'%s' = '%s'\n", Grps->Groups[g].Vars[v].Var, Grps->Groups[g].Vars[v].Val);
            }
        }
        return 0;
    }
int main (const int argc, const char *argv[]) {
    struct __config_loader *CL = config_loader_construct(nullptr);
    if (CL) {
        if (!CL->load(CL, "log.cfg")) {
            CL->build(CL);
            tostring(&(CL->Config));
        }
    }
    CL->destruct(CL);
    return 0;
}
#endif // __test_config_loader_c__