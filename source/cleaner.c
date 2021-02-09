#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define __config_cleaner_c__ 1

#include "headers/cleaner.h"

#ifndef nullptr
    #define nullptr ((0))
#endif // nullptr

/************************************** class __config_cleaner **************************************/
/* Конструктор */
    ConfigCleaner __config_cleaner_init_struct = {
        /* методы */
        .destruct = config_cleaner_destruct,
        .reset = config_cleaner_reset,
        .load = config_cleaner_process_file,
        .set_string_comments = config_cleaner_set_scomments,
        .perror = config_cleaner_perror
    };
    ConfigCleaner* config_cleaner_construct(void *Ptr) {
        ConfigCleaner *Inst = (ConfigCleaner*)Ptr;
        if (nullptr == Inst) { Inst = (ConfigCleaner *)malloc(sizeof(ConfigCleaner)); }
        if (nullptr != Inst) {
            Inst->Buffers = __config_cleaner_init_struct_buffers;
            Inst->Errors = __config_cleaner_init_struct_errors;
            Inst->Errors.unFl.s_flags.DynInst = ((nullptr == Ptr) ? 1 : 0);

            *((char***)&(Inst->Result)) = (char**)&(Inst->Buffers.In);
            *((size_t**)&(Inst->Size)) = &(Inst->Buffers.iSz);
            config_cleaner_construct_charset(&Inst->Charset);
        }
        return Inst;
    }
/* Деструктор */
    void config_cleaner_destruct(struct __config_cleaner *Inst) {
        Inst->reset(Inst);
        if (Inst->Errors.unFl.s_flags.DynInst) free(Inst);
    }

/* Методы */

    /* void config_cleaner::clear(): освобождает буфферы */
    void config_cleaner_reset(ConfigCleaner *Inst) {
        if (Inst->Errors.unFl.s_flags.BuffInit) {
            Inst->Buffers.destruct(Inst);
            Inst->Buffers = __config_cleaner_init_struct_buffers;
            *((size_t*)&(Inst->Size)) = 0;
            *((char**)&(Inst->Result)) = nullptr;
            Inst->Errors.unFl.ui32 = 0;
        }
        config_cleaner_construct_charset(Inst);
        return;
    }
    /* int load(): Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые */
    int config_cleaner_process_file(struct __config_cleaner *Inst, const char* FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        int *error = &(Inst->Errors.error);
        struct __config_cleaner_buffers *Buff = &(Inst->Buffers);
        //проверяем файл на наличие
        if (-1 == (*error = stat(FileName, &statbuf))) {
            config_cleaner_perror(Inst, FileName);
            return -1;
        }
        Buff->iSz = statbuf.st_size;
        //зфагружаем в буффер
        if (-1 == (*error = config_cleaner_init_buffers(Inst, FileName))) {
            config_cleaner_perror(Inst, "config_cleaner_init_buffer()");
            return -1;
        }
        else { Inst->Errors.unFl.s_flags.BuffInit = 1; }
        // вычищаем файл
        config_cleaner_clean_file(Inst);
        config_cleaner_delete_emty_strings(&(Inst->Buffers));
        if (!Inst->Errors.error) {
            Inst->Errors.unFl.s_flags.FileClear = 1;
        }

        return 0;
    }
    /* int isInit(): 1 - если успешно загружен, 0 - в случае ошибки */
    int config_cleaner_isInit(struct __config_cleaner *Inst) {
        return (Inst->Errors.unFl.s_flags.BuffInit);
    }
    /* int set_string_comments(): устанавливает пользовательский набор строковых комментариев */
    int config_cleaner_set_scomments(struct __config_cleaner *I, const char *Keys) {
        struct __config_cleaner_charset *CS = &(I->Charset);
        if (nullptr != Keys) {
            strncpy((char*)CS->LineComm, Keys, sizeof(CS->LineComm) - 1);
            ((char*)CS->LineComm)[sizeof(CS->LineComm) - 1] = 0;
            config_cleaner_sort_charset(CS);
            return 0;
        }
        errno == EINVAL;
        return -1;
    }
    // отчищает файл от комментариев
    int config_cleaner_clean_file(struct __config_cleaner *I) {
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
        config_cleaner_swap_buffers(&(I->Buffers));
        return 0;
    }
    // удаляет пустые строки
    int config_cleaner_delete_emty_strings(struct __config_cleaner_buffers *I) {
        size_t len = 0;
        for (I->Pw = I->Pi = 0; 0 != I->In[I->Pi]; (I->Pi)++) {
            if('\n' != I->In[I->Pi]) { I->Wrk[(I->Pw)++] = I->In[I->Pi]; ++len; }
            else if (len > 0){ I->Wrk[(I->Pw)++] = I->In[I->Pi]; len = 0; }
        }
        I->In[I->Pw] = 0;
        if ('\n' == I->Wrk[I->Pw - 1]) { I->Wrk[--(I->Pw)] = 0; }
        config_cleaner_swap_buffers(I);
        return 0;
    }
    // меняет буфферы местами
    void config_cleaner_swap_buffers(struct __config_cleaner_buffers *b) {
        unsigned char *t = b->In;
        b->In = b->Wrk;
        b->Wrk = t;
        b->iSz = b->Pw;
        b->In[b->Pw] = 0;
        b->Pi = b->Pw = 0;
    }
/* end of class __config_cleaner */

/********************************* struct __config_cleaner_charset *********************************/
/* значения по умолчанию */
    const char __config_cleaner_default_quotes[] = "`'\"";
    const char __config_cleaner_default_lcomm[] = ";#";
    const char __config_cleaner_default_nonspace[] = "\n\t";
    const char __config_cleaner_default_group_begin[] = "[";
    const char __config_cleaner_default_group_end[] = "[";
/* Конструктор */
    int config_cleaner_construct_charset(struct __config_cleaner_charset *CS) {
        strncpy((char*)CS->NotSpace, __config_cleaner_default_nonspace, sizeof(CS->NotSpace) - 1);
        strncpy((char*)CS->LineComm, __config_cleaner_default_lcomm, sizeof(CS->LineComm) - 1);
        strncpy((char*)CS->Quotes, __config_cleaner_default_quotes, sizeof(CS->Quotes) - 1);
        *(char*)&(CS->NotSpace[sizeof(CS->NotSpace) - 1]) = 0;
        *(char*)&(CS->LineComm[sizeof(CS->LineComm) - 1]) = 0;
        *(char*)&(CS->Quotes[sizeof(CS->Quotes) - 1]) = 0;
        config_cleaner_sort_charset(CS);
        return 0;
    }
    // сортирует символы и выставляет максимумы
    int config_cleaner_sort_charset(struct __config_cleaner_charset *CS) {
        config_cleaner_strsort_za((char*)CS->NotSpace);
        config_cleaner_strsort_za((char*)CS->LineComm);
        config_cleaner_strsort_za((char*)CS->Quotes);
        *(char*)&(CS->gQ) = CS->Quotes[0];
        *(char*)&(CS->gC) = CS->LineComm[0];
        return 0;
    }

/********************************* struct __config_cleaner_errors **********************************/

    struct __config_cleaner_errors __config_cleaner_init_struct_errors = {
        .Buff = { 0 },
        .error = 0,
        .unFl = { 0 }
    };
    // выводит ошибки в терминал
    void config_cleaner_perror(struct __config_cleaner *Inst, const char * What) {
        errno = Inst->Errors.error;
        sprintf((char *)Inst->Errors.Buff, "ERROR [%i] '%s'", Inst->Errors.error, What);
        perror((char *)Inst->Errors.Buff);
    }

/********************************* class __config_cleaner_buffers **********************************/

    struct __config_cleaner_buffers __config_cleaner_init_struct_buffers = {
        .destruct = config_cleaner_destruct_buffers,
        .reset = config_cleaner_reset_buffers,

        .In = 0,
        .Wrk = 0,
        .iSz = 0,
        .Pi = 0,
        .Pw = 0
    };
    // инициализирует буфферы и считывает в них файл
    int config_cleaner_init_buffers(ConfigCleaner *Inst, const char *FileName) {
        struct __config_cleaner_buffers *B = &(Inst->Buffers);
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
    /* void __config_cleaner_buffers::destruct() */
    void config_cleaner_destruct_buffers(ConfigCleaner *Inst) {
        if (Inst->Errors.unFl.s_flags.BuffInit) {
            free(Inst->Buffers.In);
            free(Inst->Buffers.Wrk);
        }
        return;
    }
    /* void __config_cleaner_buffers::reset() */
    void config_cleaner_reset_buffers(ConfigCleaner *Inst) {
        struct __config_cleaner_buffers *Buff = &(Inst->Buffers);
        
        if (Buff->In) free(Buff->In);
        if (Buff->Wrk) free(Buff->In);

        Inst->Errors.unFl.s_flags.BuffInit = 0;
        Buff->iSz = Buff->Pi = Buff->Pw = 0;
        Buff->In = Buff->Wrk = nullptr;

        return;
    }
/* end of class __config_cleaner_buffers */

/*****************************************************************************************************/
/* Функции */
    // меняет символы местами
    int config_cleaner_swap_c(char *c1, char *c2) {
        char t = *c1;
        *c1 = *c2;
        *c2 = t;
        return 0;
    }
    // сортирует строку по возрастанию
    int config_cleaner_strsort_az(char *a) {
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
    int config_cleaner_strsort_za(char *a) {
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
    int config_cleaner_strinverse(char *a) {
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
