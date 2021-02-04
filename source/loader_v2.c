#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define __config_loader_c__ 1

#include "headers/loader_v2.h"
    
    struct __config_loader ConfigLoader = {
        .eBuff = {0},
        .wLen = 0,
        .iBuffSz = 0,
        .iBuff = 0,
        .pi = 0,
        .wBuff = 0,
        .pw = 0,
        .error = 0,
        .NotSpace = { '\n', '\t', 0},
        .LineComm = { ';', '#', 0 },
        .Quotes = { '`', '\'', '"', 0 }
    };
    
//realisation
    /**
     * Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые
    */
    int loader_init(struct __config_loader *I, const char* FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        //проверяем файл на наличие
        if (-1 == (I->error = stat(FileName, &statbuf))) {
            perror(FileName);
            return -1;
        }
        I->iBuffSz = statbuf.st_size;
        //зфагружаем в буффер
        if (-1 == (I->error = loader_init_buffers(I, FileName))) {
            perror("loader_init_buffer()");
            return -1;
        }
        //удаляем комменты
        loader_trimm_comments(I);
        loader_trimm_spaces(I);
        loader_trimm_emty_strings(I);
        return 0;
    }
    // очищает буферы
    void loader_clear(struct __config_loader *Inst) {
        if (Inst->iBuff) free(Inst->iBuff);
        if (Inst->wBuff) free(Inst->wBuff);
    }
    // 1 - если успешно загружен, 0 - в случае ошибки
    int loader_isInit(struct __config_loader *Inst) {
        return (Inst->iBuff && Inst->wBuff);
    }
    // инициализирует буфферы и считывает в них файл
    int loader_init_buffers(struct __config_loader *I, const char * FileName) {
        int in;
        if (!I->iBuffSz) return -1;
        if (I->iBuff = (unsigned char *)malloc(I->iBuffSz + 1)) {
            if (I->wBuff = (unsigned char *)malloc(I->iBuffSz + 1)) {
                if (-1 != (in = open(FileName, O_RDONLY))) {
                    I->wLen = read(in, I->iBuff, I->iBuffSz);
                    I->error = errno;
                    close(in);
                    if (I->wLen == I->iBuffSz) { I->iBuff[I->iBuffSz] = 0; }
                    else { perror("init_buffer()->fread()"); }
                }
                else { perror("init_buffer()->fopen()"); }
            }
            else { perror("init_buffer()->malloc(wBuffer)"); }
        }
        else { perror("init_buffer()->malloc(iBuffer)"); }
        if (I->error) {
            free(I->wBuff);
            free(I->iBuff);
            I->iBuff = I->wBuff = 0;
        }
        return I->error;
    }
    // отчищает файл от комментариев
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
    // вычисляет конец "Си" комментария
    int loader_get_endof_c_comment(struct __config_loader *I) {
        for (I->pi += 2; 0 != I->iBuff[I->pi]; (I->pi)++ ) {
            if ('*' == I->iBuff[I->pi] && '/' == I->iBuff[I->pi + 1]) { I->pi++; break; }
        }
        return 0;
    }
    // вычисляет конец строчного коментария
    int loader_get_endof_s_comment(struct __config_loader *I) {
        while ((0 != I->iBuff[I->pi]) && ('\n' != I->iBuff[I->pi])) { ++(I->pi); }
        --(I->pi);
        return 0;
    }
    // удаляет пробелы
    int loader_trimm_spaces(struct __config_loader *I) {
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
    }
    // удаляет пустые строки
    int loader_trimm_emty_strings(struct __config_loader *I) {
        size_t len = 0;
        for (I->pw = I->pi = 0; 0 != I->iBuff[I->pi]; (I->pi)++) {
            if('\n' != I->iBuff[I->pi]) { I->wBuff[(I->pw)++] = I->iBuff[I->pi]; ++len; }
            else {
                if (len > 1) { I->wBuff[(I->pw)++] = I->iBuff[I->pi]; len = 0; }
            }
        }
        I->wBuff[I->pw] = 0;
        if ('\n' == I->wBuff[I->pw - 1]) { I->wBuff[--(I->pw)] = 0; }
        loader_swap_buffers(I);
        return 0;
    }
    // меняет буфферы местами
    void loader_swap_buffers(struct __config_loader *I) {
        unsigned char *t = I->wBuff;
        I->wBuff = I->iBuff;
        I->iBuff = t;
        I->iBuffSz = I->pw;
        I->iBuff[I->pi] = 0;
        I->pi = I->pw = 0;
    }
    // меняет символы местами
    int swap_c(char *c1, char *c2) {
        char t = *c1;
        *c1 = *c2;
        *c2 = t;
        return 0;
    }
    // сортирует по возрастанию
    int strsort_az(char *a) {
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
    int strsort_za(char *a) {
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
    int str_inverse(char *a) {
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
        sprintf((char *)I->eBuff, "ERROR [%i] '%s'", Error, What);
        perror((char *)I->eBuff);
    }
    int loader_process2(struct __config_loader *I) {
        unsigned char *In = I->iBuff, *Out = I->wBuff;
        size_t Pi = 0, Po = 0, Sz = I->iBuffSz;
        char Q = 0;
        for (size_t i = 0; i < Sz; i++) {

        }
        return 0;
    }