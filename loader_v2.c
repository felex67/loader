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

/*********************************** class Config ***********************************/
/* Объявление конструкторов */
    struct __config_loader_var __config_loader_init_struct_var = {
        .Var = nullptr,
        .Val = nullptr
    };
    struct __config_loader_var_group __config_loader_init_struct_group = {
        .Name = nullptr,
        .count = 0,
        .Vars = nullptr,
    };
    struct __config_loader_var_groups __config_loader_init_struct_groups = {
        .count = 0,
        .Groups = nullptr,
        .destruct = config_loader_destruct_groups
    };
    /* деструктор */
    void config_loader_destruct_groups(struct __config_loader_var_groups *Gps) {
        free(Gps);
    }

/************************************** class __config_loader **************************************/
/* Конструктор */
    struct __config_loader* config_loader_construct(void *Ptr) {
        struct __config_loader *Inst;
        if (nullptr == Ptr) { Inst = (struct __config_loader *)malloc(sizeof(struct __config_loader)); }
        if (nullptr != Inst) {
            Inst->Buffers = __config_loader_init_struct_buffers;
            Inst->Errors = __config_loader_init_struct_errors;
            Inst->Errors.unFl.s_flags.DynInst = ((nullptr == Ptr) ? 1 : 0);
            Inst->Config = __config_loader_init_struct_groups;
            //
            Inst->load = config_loader_process_file;
            Inst->clear = config_loader_destruct_buffers;
            Inst->isInit = config_loader_isInit;
            Inst->set_string_comments = config_loader_set_scomments;
            Inst->destruct = config_loader_destruct;
            //
            config_loader_construct_charset(&Inst->Charset);
        }
        return Inst;
    }
/* Деструктор */
    void config_loader_destruct(struct __config_loader *Ptr) {
        if (Ptr->Errors.unFl.s_flags.Init) Ptr->Buffers.destruct(Ptr);
        if (Ptr->Errors.unFl.s_flags.DynInst) free(Ptr);
    }
/* Методы */
    /* int load(): Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые */
    int config_loader_process_file(struct __config_loader *I, const char* FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        int *error = &(I->Errors.error);
        struct __config_loader_buffers *Buff = &(I->Buffers);
        //проверяем файл на наличие
        if (-1 == (*error = stat(FileName, &statbuf))) {
            config_loader_perror(I, FileName, errno);
            return -1;
        }
        Buff->iSz = statbuf.st_size;
        //зфагружаем в буффер
        if (-1 == (*error = config_loader_init_buffers(I, FileName))) {
            config_loader_perror(I, "config_loader_init_buffer()", *error);
            return -1;
        }
        else { I->Errors.unFl.s_flags.Init = 1; }
        // вычищаем файл
        config_loader_clean_file(I);
        config_loader_delete_emty_strings(&(I->Buffers));
        return 0;
    }
    /* int isInit(): 1 - если успешно загружен, 0 - в случае ошибки */
    int config_loader_isInit(struct __config_loader *Inst) {
        return (Inst->Errors.unFl.s_flags.Init);
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
        if (!I->Errors.unFl.s_flags.Init) return -1;
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
    const char __config_loader_default_group[] = "[]";
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

/********************************* struct __config_loader_errors **********************************/

    struct __config_loader_errors __config_loader_init_struct_errors = {
        .Buff = { 0 },
        .error = 0,
        .unFl = { 0 }
    };
    // выводит ошибки в терминал
    void config_loader_perror(struct __config_loader *I, const char * What, int Error) {
        sprintf((char *)I->Errors.Buff, "ERROR [%i] '%s'", Error, What);
        perror((char *)I->Errors.Buff);
    }

/********************************* class __config_loader_buffers **********************************/

    struct __config_loader_buffers __config_loader_init_struct_buffers = {
        .In = 0,
        .Wrk = 0,
        .iSz = 0,
        .Pi = 0,
        .Pw = 0,
        .destruct = config_loader_destruct_buffers
    };
    // инициализирует буфферы и считывает в них файл
    int config_loader_init_buffers(struct __config_loader *CL, const char *FileName) {
        struct __config_loader_buffers *B = &CL->Buffers;
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
                    else { config_loader_perror(CL, "init_buffer()->fread()", CL->Errors.error); }
                }
                else { CL->Errors.error = errno; config_loader_perror(CL, "init_buffer()->fopen()", errno); }
            }
            else { CL->Errors.error = errno; config_loader_perror(CL, "init_buffer()->malloc(wBuffer)", errno); }
        }
        else { CL->Errors.error = errno; config_loader_perror(CL, "init_buffer()->malloc(iBuffer)", errno); }
        if (CL->Errors.error) {
            free(B->In);
            free(B->Wrk);
            B->Pi = B->Pw = 0;
            B->iSz = 0;
        }
        return CL->Errors.error;
    }
    /* void config_loader::clear(): освобождает буфферы */
    /* void __config_loader_buffers::destruct */
    void config_loader_destruct_buffers(struct __config_loader *Inst) {
        if (nullptr != Inst->Buffers.Wrk) free(Inst->Buffers.Wrk);
        if (nullptr != Inst->Buffers.In) free(Inst->Buffers.In);
        Inst->Buffers.iSz = Inst->Buffers.Pi = Inst->Buffers.Pw = 0;
        Inst->Errors.unFl.s_flags.Init = 0;
    }
/* end of class __config_loader_buffers */

/*****************************************************************************************************/
/* Функции */
    // сортирует символы и выставляет максимумы
    int config_loader_sort_charset(struct __config_loader_charset *CS) {
        config_loader_strsort_za((char*)CS->NotSpace);
        config_loader_strsort_za((char*)CS->LineComm);
        config_loader_strsort_za((char*)CS->Quotes);
        *(char*)&(CS->gQ) = CS->Quotes[0];
        *(char*)&(CS->gC) = CS->LineComm[0];
        return 0;
    }
    // меняет символы местами
    int config_loader_swap_c(char *c1, char *c2) {
        char t = *c1;
        *c1 = *c2;
        *c2 = t;
        return 0;
    }
    // сортирует по возрастанию
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
    // сортирует по убыванию
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
    

int main (const int argc, const char *argv[]) {
    struct __config_loader *CL = config_loader_construct(nullptr);
    if (!CL->load(CL, "log.cfg")) {
        int fout = open("out.cfg", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
        write(fout, CL->Buffers.In, CL->Buffers.iSz);
        close(fout);
        printf("Cleared:\n%s\n", (char*)CL->Buffers.In);
    }
    CL->destruct(CL);
    return 0;
}
