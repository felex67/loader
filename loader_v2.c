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
    #define nullptr ((void*)0)
#endif // nullptr

    const char loader_default_quotes[] = "`'\"";
    const char loader_default_lcomm[] = ";#";
    const char loader_NonSpaceChars[] = "\n\t";
    const char loader_default_group[] = "[]";

    struct __loader_errors __loader_init_error = {
        .Buff = { 0 },
        .error = 0,
        .unFl = { 0 }
    };
    struct __loader_buffers __loader_init_buffers = {
        .In = nullptr,
        .Wrk = nullptr,
        .iSz = 0,
        .Pi = 0,
        .Pw = 0
    };
    struct __loader_charset __loader_init_charset = {
        .NotSpace = { 0 },
        .LineComm = { 0 },
        .Quotes = { 0 },
        .gQ = 0,
        .gC = 0
    };
/* конструктор */
    struct __config_loader* loader_construct(void *Ptr) {
        struct __config_loader *Inst;
        if (nullptr == Ptr) { Inst = (struct __config_loader *)malloc(sizeof(struct __config_loader)); }
        if (nullptr != Inst) {
            Inst->Buffers = __loader_init_buffers;
            Inst->Errors = __loader_init_error;
            Inst->Errors.unFl.s_flags.DynInst = ((nullptr == Ptr) ? 1 : 0);
            Inst->load = loader_process_file;
            Inst->clear = loader_cleanup_buffers;
            Inst->isInit = loader_isInit;
            Inst->set_string_comments = loader_set_scomments;
            Inst->destruct = loader_cleanup_instance;
            loader_init_charset(&Inst->Charset);
        }
        return Inst;
    }
/* деструктор */
    void loader_cleanup_instance(struct __config_loader *Ptr) {
        if (Ptr->Errors.unFl.s_flags.Init) loader_clear_buffers(Ptr);
        if (Ptr->Errors.unFl.s_flags.DynInst) free(Ptr);
    }
    void loader_cleanup_buffers(struct __config_loader *Inst) {
        if (nullptr != Inst->Buffers.Wrk) free(Inst->Buffers.Wrk);
        if (nullptr != Inst->Buffers.In) free(Inst->Buffers.In);
        Inst->Buffers.iSz = Inst->Buffers.Pi = Inst->Buffers.Pw = 0;
        Inst->Errors.unFl.s_flags.Init = 0;
    }
    void loader_cleanup_groups(struct Config_Groups *Gps) {
        for (int i = 0; i < Gps->count; i++) {
            Gps->Groups[i]->destruct(Gps->Groups[i]);
        }
        free(Gps);
    }
    void loader_cleanup_group(cfggrp_t *Grp) {
        free(Grp->Vars);
    }
//realisation
    /**
     * Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые
    */
    int loader_process_file(struct __config_loader *I, const char* FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        int *error = I->Errors.error;
        struct __loader_buffers *Buff = &(I->Buffers);
        //проверяем файл на наличие
        if (-1 == (*error = stat(FileName, &statbuf))) {
            loader_perror(I, FileName, errno);
            return -1;
        }
        Buff->iSz = statbuf.st_size;
        //зфагружаем в буффер
        if (-1 == (*error = loader_init_buffers(I, FileName))) {
            loader_perror(I, "loader_init_buffer()", *error);
            return -1;
        }
        else { I->Errors.unFl.s_flags.Init = 1; }
        //удаляем комменты
        loader_trimm_comments(I);
        loader_trimm_spaces(I);
        loader_trimm_emty_strings(I);
        return 0;
    }
    // очищает буферы
    // 1 - если успешно загружен, 0 - в случае ошибки
    int loader_isInit(struct __config_loader *Inst) {
        return (Inst->Errors.unFl.s_flags.Init);
    }
    // инициализирует буфферы и считывает в них файл
    int loader_init_buffers(struct __config_loader *CL, const char *FileName) {
        struct __loader_buffers *B = &CL->Buffers;
        int In;
        if (!B->iSz) return -1;
        if (B->In = (unsigned char *)malloc(B->iSz + 1)) {
            if (B->Wrk = (unsigned char *)malloc(B->iSz + 1)) {
                if (-1 != (In = open(FileName, O_RDONLY))) {
                    size_t readed = read(In, B->In, B->iSz);
                    CL->Errors.error = errno;
                    close(In);
                    if (readed == B->iSz) {
                        CL->Errors.unFl.s_flags.Init = 1;
                        CL->Errors.error = 0;
                        B->In[B->iSz] = 0;
                    }
                    else { loader_perror(CL, "init_buffer()->fread()", CL->Errors.error); }
                }
                else { CL->Errors.error = errno; loader_perror(CL, "init_buffer()->fopen()", errno); }
            }
            else { CL->Errors.error = errno; loader_perror(CL, "init_buffer()->malloc(wBuffer)", errno); }
        }
        else { CL->Errors.error = errno; loader_perror(CL, "init_buffer()->malloc(iBuffer)", errno); }
        if (CL->Errors.error) {
            free(B->In);
            free(B->Wrk);
            B->Pi = B->Pw = 0;
            B->iSz = 0;
        }
        return CL->Errors.error;
    }
    // инициализирует набор символов
    int loader_init_charset(const struct __loader_charset *CS) {
        strncpy((char*)CS->NotSpace, loader_NonSpaceChars, sizeof(CS->NotSpace) - 1);
        strncpy((char*)CS->LineComm, loader_default_lcomm, sizeof(CS->LineComm) - 1);
        strncpy((char*)CS->Quotes, loader_default_quotes, sizeof(CS->Quotes) - 1);
        *(char*)CS->NotSpace[sizeof(CS->NotSpace) - 1] = 0;
        *(char*)CS->LineComm[sizeof(CS->LineComm) - 1] = 0;
        *(char*)CS->Quotes[sizeof(CS->Quotes) - 1] = 0;
    }
    // сортирует символы и выставляет максимумы
    int loader_sort_charset(const struct __loader_charset *CS) {
        loader_strsort_za((char*)CS->NotSpace);
        loader_strsort_za((char*)CS->LineComm);
        loader_strsort_za((char*)CS->Quotes);
        *(char*)CS->gQ = CS->Quotes[0];
        *(char*)CS->gC = CS->LineComm[0];
    }
    // отчищает файл от комментариев
    /*
    int loader_trimm_comments(struct __config_loader *I) {
        for (I->pw = 0, I->pi = 0; 0 != I->iBuff[I->pi]; I->pi++) {
            if (';' < I->iBuff[I->pi]) { I->wBuff[(I->pw)++] = I->iBuff[I->pi]; }
            else {
                switch (I->iBuff[I->pi]) {
                case ';':
                case '#':
                    loader_get_endof_s_comment(I);
                    break;
                case '/':
                    if (I->iBuff[I->pi + 1] == '*') {
                        loader_get_endof_c_comment(I);
                        break;
                    }
                default:
                    I->wBuff[(I->pw)++] = I->iBuff[I->pi];
                }
            }
        }
        I->wBuff[I->pw] = 0;
        loader_swap_buffers(I);
        return 0;
    }
    
    */
    // вычисляет конец "Си" комментария
    int loader_endof_c_comment(struct __loader_buffers *B) {
        for (B->Pi += 2; 0 != (B->In[B->Pi]); (B->Pi)++ ) {
            if ('*' == B->In[B->Pi] && '/' == B->In[B->Pi + 1]) { (B->Pi)++; break; }
        }
        return 0;
    }
    // вычисляет конец строчного коментария
    int loader_endof_s_comment(struct __loader_buffers *I) {
        while ((0 != I->In[I->Pi]) && ('\n' != I->In[I->Pi])) { ++(I->Pi); }
        --(I->Pi);
        return 0;
    }
    /*
    // удаляет пробелы
    int loader_trimm_spaces(struct __config_loader *Inst) {
        unsigned char Q = 0;
        if (('"' == I->iBuff[0])  || ('\'' == I->iBuff[0]) || ('`' == I->iBuff[0])) {
            Q = I->iBuff[0];
            I->wBuff[0] = I->iBuff[0];
        }
        for (I->pw = I->pi = (!Q ? 0 : 1); 0 != I->iBuff[I->pi]; I->pi++) {
            if (('`' != I->iBuff[I->pi]) && ('\'' < I->iBuff[I->pi])) { I->wBuff[I->pw++] = I->iBuff[I->pi]; }
            else {
                switch (I->iBuff[I->pi]) {
                case ' ':
                    if (!Q) {}
                    else { I->wBuff[(I->pw)++] = I->iBuff[I->pi]; }
                    break;
                case '"':
                case '\'':
                case '`':
                    if ('\\' != I->iBuff[I->pi - 1]) { Q = (Q ? 0 : I->iBuff[I->pi]); }
                case '\t':
                case '\n':
                    I->wBuff[(I->pw)++] = I->iBuff[I->pi];
                    break;
                default:
                    if (' ' < I->iBuff[I->pi]) {
                        I->wBuff[(I->pw)++] = I->iBuff[I->pi];
                    }
                }
            }
        }
        I->wBuff[I->pw] = 0;
        loader_swap_buffers(I);
        return 0;
    } */
    int loader_set_scomments(struct __config_loader *I, const char *Keys) {
        struct __loader_charset *CS = &(I->Charset);
        if (nullptr != Keys) {
            strncpy((char*)CS->LineComm, Keys, sizeof(CS->LineComm) - 1);
            ((char*)CS->LineComm)[sizeof(CS->LineComm) - 1] = 0;
            loader_sort_charset(CS);
            return 0;
        }
        errno == EINVAL;
        return -1;
    }
    // удаляет пустые строки
    int loader_trimm_emty_strings(struct __loader_buffers *I) {
        size_t len = 0;
        for (I->Pw = I->Pi = 0; 0 != I->In[I->Pi]; (I->Pi)++) {
            if('\n' != I->In[I->Pi]) { I->Wrk[(I->Pw)++] = I->In[I->Pi]; ++len; }
            else {
                if (len > 1) { I->In[(I->Pw)++] = I->In[I->Pi]; len = 0; }
            }
        }
        I->In[I->Pw] = 0;
        if ('\n' == I->In[I->Pw - 1]) { I->In[--(I->Pw)] = 0; }
        loader_swap_buffers(I);
        return 0;
    }
    // меняет буфферы местами
    void loader_swap_buffers(struct __loader_buffers *b) {
        unsigned char *t = b->Wrk;
        b->Wrk = b->In;
        b->In = t;
        b->iSz = b->Pw;
        b->In[b->Pw] = 0;
        b->Pi = b->Pw = 0;
    }
    // меняет символы местами
    int loader_swap_c(char *c1, char *c2) {
        char t = *c1;
        *c1 = *c2;
        *c2 = t;
        return 0;
    }
    // сортирует по возрастанию
    int loader_strsort_az(char *a) {
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
    // сортирует по убыванию
    int loader_strsort_za(char *a) {
        int i = 0, fwd = 1, swapped = 0;
        if (0 != a) {
            for ( i = 0, fwd = 1; (0 != a[i]); ) {
                if ((0 != a[i + 1]) && swapped) { fwd = 0; }
                if (a[i] < a[i + 1]) {
                    swap_chars(&(a[i]), &(a[i + 1]));
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
    int loader_strinverse(char *a) {
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
    // выводит ошибки в терминал
    void loader_perror(struct __config_loader *I, const char * What, int Error) {
        sprintf((char *)I->Errors.Buff, "ERROR [%i] '%s'", Error, What);
        perror((char *)I->Errors.Buff);
    }
    
#undef nullptr
