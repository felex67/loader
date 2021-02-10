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
    ConfigCleaner* new_config_cleaner(ConfigCleaner *Ptr) {
        ConfigCleaner *Inst = Ptr;
        if (!Inst) Inst = (ConfigCleaner*)malloc(sizeof(ConfigCleaner));
        if (Inst) {
            *(size_t*)&Inst->__private_flags = (Ptr ? 0 : CONFCLNR_FLAGS_DYNINST);

            *((void**)&Inst->destruct) = (void*)config_cleaner_destruct;
            *((void**)&Inst->reset) = (void*)config_cleaner_reset;
            *((void**)&Inst->resetBuff) = (void*)config_cleaner_reset_buffers;
            *((int**)&Inst->set_string_comments) = (int*)config_cleaner_set_scomments;
            *((int**)&Inst->load) = (int*)config_cleaner_process_file;
            *((int**)&Inst->release) = (int*)config_cleaner_release;

            config_cleaner_buffers_construct((struct __config_cleaner_buffers*)&Inst->__private_buffs);
            config_cleaner_charset_construct((struct __config_cleaner_charset*)&Inst->__private_char);
            
            errno = EXIT_SUCCESS;
        }
        return Inst;
    }
/* Деструктор */
    void config_cleaner_destruct(struct __config_cleaner *Inst) {
        if (Inst) {
            Inst->__private_buffs.destruct(((struct __config_cleaner_buffers*)&Inst->__private_buffs));
            if (Inst->__private_flags & CONFCLNR_FLAGS_DYNINST) free(Inst);
        }
    }
/* Методы */

    /* void config_cleaner::reset(): освобождает буфферы */
    void config_cleaner_reset(ConfigCleaner *Inst) {
        if (Inst) {
            Inst->resetBuff(Inst);
            config_cleaner_charset_construct((struct __config_cleaner_charset*)&Inst->__private_char);
        }
        return;
    }
    /* int load(): Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые */
    int config_cleaner_process_file(struct __config_cleaner *Inst, const char* FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        size_t readed = 0;
        int result = -1;
        if (Inst && FileName) {
            //проверяем файл на наличие
            if (-1 != (result = stat(FileName, &statbuf))) {
                if (-1 != (result = Inst->__private_buffs.init((struct __config_cleaner_buffers*)&Inst->__private_buffs, statbuf.st_size + 1))) {
                    if (-1 != (result = open(FileName, O_RDONLY))) {
                        if (statbuf.st_size == (readed = read(result, Inst->__private_buffs.In.bytes, statbuf.st_size))) {
                            if (-1 != (result = config_cleaner_clean_file(Inst))) {
                                result = config_cleaner_delete_emty_strings(Inst);
                            }
                        }
                        else {
                            result = -1;
                        }
                    }    
                }
            }
        }
        return result;
    }
    /* int set_string_comments(): устанавливает пользовательский набор строковых комментариев */
    int config_cleaner_set_scomments(struct __config_cleaner *I, const char *Keys) {
        struct __config_cleaner_charset *CS = (struct __config_cleaner_charset*)&(I->__private_char);
        if (nullptr != Keys) {
            strncpy((char*)CS->LineComm, Keys, sizeof(CS->LineComm) - 1);
            ((char*)CS->LineComm)[sizeof(CS->LineComm) - 1] = 0;
            config_cleaner_charset_sort(CS);
            return 0;
        }
        errno == EINVAL;
        return -1;
    }
    // отчищает файл от комментариев
    int config_cleaner_clean_file(struct __config_cleaner *I) {
        if (!I) { errno = EINVAL; return -1; }
        // pos & size
        size_t pi = 0, pw = 0;
        const size_t iSz = (I->__private_buffs.In.size) - 1;
        // buffers
        unsigned char *in = I->__private_buffs.In.bytes, *wrk = I->__private_buffs.Wrk.bytes;
        // chars
        const char *Qts = I->__private_char.Quotes;
        const char *Com = I->__private_char.LineComm;
        const char *NSp = I->__private_char.NotSpace;
        // max chars
        char *gQ = (char*)&(I->__private_char.gQ);
        char *gC = (char*)&(I->__private_char.gC);
        char *tchar = nullptr;
        unsigned char Q = 0;
        unsigned char mchar = (unsigned char)(*gQ < *gC ? *gC : *gQ);
        
        for (pi = 0, pw = 0; pi < iSz; pi++) {
            // помимо тех что больше максимального искомого символа исключаем алфавит и цифры
            if ((in[pi] > mchar) || ('A' <= in[pi] && 'Z' >= in[pi]) || (('0' <= in[pi]) && ('9' >= in[pi]))) {
                wrk[(pw)++] = in[pi];
            }
            // это ковычка
            else if (nullptr != (tchar = strchr(*((char**)&Qts), in[pi]))) {
                do {
                    wrk[(pw)++] = in[(pi)++];
                } while ((0 != in[pi]) && (('"' != in[pi]) || ('\\' == in[(pi) - 1])));
                wrk[(pw)++] = in[pi];
            }
            // это строковый коммент
            else if (nullptr != (tchar = strchr(*((char**)&Com), in[pi]))) {
                while ('\n' != in[pi]) { ++(pi); }
                wrk[(pw)++] = in[pi];
            }
            // возможно Си коммент 
            else if (in[pi] == '/') {
                if ('/' == in[(pi) + 1]) {
                    while ('\n' != in[pi]) { ++(pi); }
                    wrk[(pw)++] = in[pi];
                }
                else if ('*' == in[(pi) + 1]) {
                    do { (pi++); } while (('*' != in[pi]) && ('/' != in[(pi) + 1]));
                    ++(pi);
                }
                else {
                    wrk[(pw)++] = in[pi];
                }
            }
            // необходимый служебный символ: '\t' || '\n'
            else if ((' ' < in[(pi)]) || (nullptr != (tchar = strchr(*((char**)&NSp), in[pi])))) { wrk[(pw)++] = in[pi]; }
        }
        I->__private_buffs.In.bytes[pw] = 0;
        I->__private_buffs.Wrk.resize(((bytebuffer_t*)&I->__private_buffs.Wrk), pw);
        I->__private_buffs.swap((struct __config_cleaner_buffers*)&I->__private_buffs);
        return 0;
    }
    // удаляет пустые строки
    int config_cleaner_delete_emty_strings(struct __config_cleaner *Inst) {
        unsigned char *in = nullptr, *wrk = nullptr;
        size_t pi = 0, pw = 0, isz = 0, len = 0;
        int result = -1;
        if (Inst) {
            in = Inst->__private_buffs.In.bytes;
            wrk = Inst->__private_buffs.Wrk.bytes;
            isz = Inst->__private_buffs.In.size;
            for ( ; pi < isz; pi++) {
                if('\n' != in[pi]) { wrk[pw++] = in[pi]; ++len; }
                else if (len > 0){ wrk[pw++] = in[pi]; len = 0; }
            }
            wrk[pw] = 0;
            if ('\n' == wrk[pw - 1]) { wrk[--(pw)] = 0; }
            Inst->__private_buffs.Wrk.resize((bytebuffer_t*)&Inst->__private_buffs.Wrk, pw);
            Inst->__private_buffs.swap((struct __config_cleaner_buffers*)&Inst->__private_buffs);
            errno = result = EXIT_SUCCESS;
        }
        else { errno = EINVAL; }
        return result;
    }
    //
    int config_cleaner_release(ConfigCleaner *Inst, void **Dest, size_t *DestSize) {
        int result = -1;
        if (Inst && Dest && DestSize) {
            result = Inst->__private_buffs.In.release((bytebuffer_t*)&Inst->__private_buffs.In, Dest, DestSize);
        }
        else { errno = EINVAL; }
        return result;
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
    int config_cleaner_charset_construct(struct __config_cleaner_charset *CS) {
        int result = -1;
        if (CS) {
            strncpy((char*)CS->NotSpace, __config_cleaner_default_nonspace, sizeof(CS->NotSpace) - 1);
            strncpy((char*)CS->LineComm, __config_cleaner_default_lcomm, sizeof(CS->LineComm) - 1);
            strncpy((char*)CS->Quotes, __config_cleaner_default_quotes, sizeof(CS->Quotes) - 1);
            *(char*)&(CS->NotSpace[sizeof(CS->NotSpace) - 1]) = 0;
            *(char*)&(CS->LineComm[sizeof(CS->LineComm) - 1]) = 0;
            *(char*)&(CS->Quotes[sizeof(CS->Quotes) - 1]) = 0;
            result = config_cleaner_charset_sort(CS);
        }
        else { errno = EINVAL; }
        return 0;
    }
    // сортирует символы и выставляет максимумы
    int config_cleaner_charset_sort(struct __config_cleaner_charset *CS) {
        int result = -1;
        if (CS) {
            config_cleaner_strsort_za((char*)CS->NotSpace);
            config_cleaner_strsort_za((char*)CS->LineComm);
            config_cleaner_strsort_za((char*)CS->Quotes);
            *(char*)&(CS->gQ) = CS->Quotes[0];
            *(char*)&(CS->gC) = CS->LineComm[0];
            errno = result = EXIT_SUCCESS;
        }
        else { errno = EINVAL; }
        return result;
    }

/********************************* class __config_cleaner_buffers **********************************/
    int config_cleaner_buffers_construct(struct __config_cleaner_buffers *Buffs) {
        int result = -1;
        if (Buffs) {
            *((void**)&Buffs->destruct) = (void*)config_cleaner_buffers_destruct;
            *((int**)&Buffs->init) = (int*)config_cleaner_buffers_init;
            *((void**)&Buffs->reset) = (void*)config_cleaner_buffers_reset;
            *((int**)&Buffs->swap) = (int*)config_cleaner_buffers_swap;
            
            new_byte_buffer(&Buffs->In);
            new_byte_buffer(&Buffs->Wrk);
            errno = result = EXIT_SUCCESS;
        }
        else { errno = EINVAL; }
        return result;
    }
    void config_cleaner_buffers_destruct(struct __config_cleaner_buffers *Buffs) {
        if (Buffs) {
            Buffs->In.destruct(&Buffs->In);
            Buffs->Wrk.destruct(&Buffs->Wrk);
        }
    }
    // инициализирует буфферы, выделяет память 'Size' байт
    int config_cleaner_buffers_init(struct __config_cleaner_buffers *Buffs, const size_t Size) {
        int result = -1;
        if (Buffs && Size) {
            if (-1 != (result = Buffs->In.init(&Buffs->In, Size))) {
                if (-1 != (result = Buffs->Wrk.clone(&Buffs->Wrk, &Buffs->In))) {
                    errno = result = EXIT_SUCCESS;
                }
            }
            if (result) {
                // сохраним чтоб знать что стряслось =)
                result = errno;
                Buffs->In.destruct(&Buffs->In);
                Buffs->In.destruct(&Buffs->Wrk);
                errno = result;
            }
        }
        else { errno = EINVAL; }
        return result;
    }
    //
    void config_cleaner_buffers_reset(struct __config_cleaner_buffers *Buffs) {
        if (Buffs) {
            Buffs->In.reset(&Buffs->In);
            Buffs->Wrk.reset(&Buffs->Wrk);
        }
    }
    int config_cleaner_buffers_swap(struct __config_cleaner_buffers *Buffs) {
        int result = -1;
        if (Buffs) {
            result = Buffs->In.swap(&Buffs->In, &Buffs->Wrk);
        }
        else { errno = EINVAL; }
        return result;
    }

    /* void __config_cleaner::resetBuff() */
    void config_cleaner_reset_buffers(ConfigCleaner *Inst) {
        if (Inst) {
            Inst->__private_buffs.reset(((struct __config_cleaner_buffers*)&Inst->__private_buffs));
        }
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
