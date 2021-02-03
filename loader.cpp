#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

struct Entry {
    char *var;
    char *val;
    Entry(char* var, char* val): var(var), val(val) {}
    ~Entry() { delete[] var; delete[] val; }
};
size_t get_closingQuote(char,const char *);
size_t trimm_comments(char *);
size_t trimm_line(char *);
size_t get_endof_c_comment(char *);
size_t trimm_spaces(char *);
size_t trimm_nonquoted_spaces(char *);
size_t find_nextQuote(const char, const char *);
size_t trimm_emty_strings(char *Start);
int process_loaded_file(char *, char *);
const char QuotePairs[][2] = { { '"', '"' }, { '\'', '\'' } };
const char Braces[][2] = { { '(', ')'}, { '{' , '}' }, { '[', ']' } };
const char *Comments[] = { "//", "#", "/*"};

enum BraceSide { BrOpen, BrClose };

size_t get_closingQuote(char ClosingQuote, const char* Start) {
    for (size_t i = 0; Start[i] != ClosingQuote; i++) {
        if (Start[i]) {
            if (Start[i] != ClosingQuote);
            else {
                if (Start[i - 1] != '\\') {
                    return i;
                }
            }
        }
        else { break; }
    }
    return 0;
}
size_t trimm_spaces(char *Start) {
    char *tgt = Start, *src = Start;
    while (*src != 0) {
        if ((' ' < *src) || ('\n' == *src)) {
            if (tgt != src) { *tgt = *src; }
            ++tgt;
        }
        ++src;
    }
    *tgt = 0;
    return (tgt - Start);
}
/*
    ЭТУ ФУНКЦИЮ НУЖНО ПЕРЕДЕЛАТЬ!!!
    РАБОТАЕТ, НО МОЖНО И БЫСТРЕЙ
    варианты ускорения
    1. найти первый срез и продолжать без проверок идентичности указателей
    2. копировать в буффер, потом мащинными словами скопировать обратно
    3. подавать 2 буффера
 */ 
size_t trimm_nonquoted_spaces(char *Start) {
    unsigned char *tgt = (unsigned char *)Start, *src = (unsigned char*)Start;
    unsigned char Q = 0;
    //для непонятливых =D
    bool same = (tgt == src);
    //после можно смело проверять на обратную косую
    if ((*src == '"') || (*src == '\'') || (*src == '`')) { Q = *(src++); }
    //покуда строка не кончилась
    while (*src != 0) {
        // Это точно НЕ ковычка
        if ('\'' < *src) {
            if (!same) { *tgt = *src; }
            ++tgt;
        }
        //может быть
        else {
            //точно - ковычка самая частая(") потом реже (') самая редкая (`)
            if ((*src == '"') || (*src == '\'') || (*src == '`')) {
                //ещё и без обратной косой
                if ('\\' != src[-1]) {
                    //если первая - всё просто =)
                    if (!Q) { Q = *src; }
                    //да т если вторая,- тоже
                    else if (Q == *src) { Q = 0; }
                }
                if (!same) { *tgt = *src; }
                ++tgt;
            }
            //точно НЕ ковычка
            else {
                //больше пробела или перенос каретки? 
                if ((' ' < *src) || ('\n' == *src)) {
                    if (!same) { *tgt = *src; }
                    ++tgt;
                }
                else {
                    /** точно что-то что нам НЕ нужно =)
                     * значит будет нолик, если это не пробел,
                     * или мы не в кавычках! */
                    if (((' ' == *src) /* || ('\t' == *src) */) && (0 != Q))  {
                        // это пробел и мы в кавычках
                        if (!same) *tgt = *src;
                        ++tgt;
                    }
                    else {
                        same = false;
                    }
                }
            }
            if (!same) { *src = 0; }
        }
        ++src;
    }
    *tgt = 0;
    return ((char *)tgt - Start);
}
size_t find_nextQuote(const char Q, const char *Start) {
    size_t n;
    for (n = 1; 0 != Start[n]; n++) {
        if (Start[n] != Q || '\\' != Start[n - 1]);
        else { break; }
    }
    return n;
}
size_t trimm_emty_strings(char *Start) {
    size_t t = 0, s = 0;
    if ('\n' == *Start) { s = 1; }
    else { while ((0 != Start[t]) && ('\n' != Start[t])) { ++t; } }
    if (0 != Start[t]) {
        for (s = t + 1 ; 0 < Start[s]; s++) {
            if (Start[s]);
        }
    }
    Start[t] = 0;
    return t;
}

struct Loader {
    Loader() : eBuff() {}
    ~Loader() {
        if (nullptr != iBuff)
            free(iBuff);
        if (nullptr != wBuff)
            free(wBuff);
    }
//private:
    /**
     * Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые,
     * разбивает файл на группы(если они есть: [Имя группы])
    */
    int init_buffer(const char * FileName) {
        //FILE *in;
        int in;
        if (!iBuffSz) return -1;
        if (nullptr != (iBuff = (unsigned char *)malloc(iBuffSz + 1))) {
            if (nullptr != (wBuff = (unsigned char *)malloc(iBuffSz + 1))) {
                if (-1 != (in = open(FileName, O_RDONLY))) {
                    wLen = read(in, iBuff, iBuffSz);
                    error = errno;
                    close(in);
                    if (wLen == iBuffSz) { iBuff[iBuffSz] = 0; }
                    else { perror("init_buffer()->fread()"); }
                }
                else { perror("init_buffer()->fopen()"); }
            }
            else { perror("init_buffer()->malloc(wBuffer)"); }
        }
        else { perror("init_buffer()->malloc(iBuffer)"); }
        return error;
    }
    //Загружает файл и создаёт 2 буффера
    int load_config(const char *FileName) {
        //get fileinfo
        struct stat statbuf;
        unsigned char* temp;
        //проверяем файл на наличие
        if (-1 == (error = stat(FileName, &statbuf))) {
            perror(FileName);
            return -1;
        }
        iBuffSz = statbuf.st_size;
        //зфагружаем в буффер
        if (-1 == (error = init_buffer(FileName))) {
            perror("init_buffer()");
            return -1;
        }
        //удаляем комменты
        trimm_comments();
        trimm_spaces();
        trimm_emty_strings();
        return 0;
    }
    //отчищает файл от комментариев
    int trimm_comments() {
        for (pos_w = 0, pos_i = 0; 0 != iBuff[pos_i]; pos_i++) {
            if (';' < iBuff[pos_i]) { wBuff[pos_w] = iBuff[pos_i]; ++pos_w; }
            else {
                switch (iBuff[pos_i]) {
                case '/':
                    if (iBuff[pos_i + 1] == '*') {
                        get_endof_c_comment();
                        break;
                    }
                case ';':
                case '#':
                    get_endof_s_comment();
                    break;
                default:
                    wBuff[pos_w++] = iBuff[pos_i];
                }
            }
        }
        wBuff[pos_w] = 0;
        swap_buffers();
        return 0;
    }
    //вычисляет конец "Си" комментария
    int get_endof_c_comment() {
        for (pos_i+= 2; 0 != iBuff[pos_i]; pos_i++ ) {
            if ('*' == iBuff[pos_i] && '/' == iBuff[pos_i + 1]) { pos_i++; break; }
        }
        return 0;
    }
    //вычисляет конец строчного коментария
    int get_endof_s_comment() {
        while ((0 != iBuff[pos_i]) && ('\n' != iBuff[pos_i])) { ++pos_i; }
        --pos_i;
        return 0;
    }
    int trimm_spaces() {
        unsigned char Q = 0;
        if (('"' == iBuff[0])  || ('\'' == iBuff[0]) || ('`' == iBuff[0])) {
            Q = iBuff[0];
            wBuff[0] = iBuff[0];
        }
        for (pos_w = pos_i = (!Q ? 0 : 1); 0 != iBuff[pos_i]; pos_i++) {
            if (('`' != iBuff[pos_i]) && ('\'' < iBuff[pos_i])) { wBuff[pos_w++] = iBuff[pos_i]; }
            else {
                switch (iBuff[pos_i]) {
                case ' ':
                    if (!Q) {}
                    else { wBuff[pos_w++] = iBuff[pos_i]; }
                    break;
                case '"':
                case '\'':
                case '`':
                    if ('\\' != iBuff[pos_i - 1]) { Q = (Q ? 0 : iBuff[pos_i]); }
                case '\t':
                case '\n':
                    wBuff[pos_w++] = iBuff[pos_i];
                }
            }
        }
        wBuff[pos_w] = 0;
        swap_buffers();
        return 0;
    }
    int trimm_emty_strings() {
        size_t len = 0;
        for (pos_w = pos_i = 0; 0 != iBuff[pos_i]; pos_i++) {
            if('\n' != iBuff[pos_i]) { wBuff[pos_w++] = iBuff[pos_i]; ++len; }
            else {
                if (len > 0) { wBuff[pos_w++] = iBuff[pos_i]; len = 0; }
            }
        }
        wBuff[pos_w] = 0;
        if ('\n' == wBuff[pos_w - 1]) { wBuff[--pos_w] = 0; }
        swap_buffers();
        return 0;
    }
    void swap_buffers() {
        unsigned char *t = iBuff;
        iBuff = wBuff;
        wBuff = t;
        iBuffSz = pos_w;
        pos_i = pos_w = 0;
    }

    void perror(const char * What) {
        sprintf((char *)eBuff, "ERROR [%i] '%s'", errno, What);
        perror((char *)eBuff);
    }
    void perror(const char * What, int Error) {
        sprintf((char *)eBuff, "ERROR [%i] '%s'", Error, What);
        perror((char *)eBuff);
    }
//private:
    char eBuff[2048];
    std::vector<char *> Strings;

    size_t wLen;
    size_t iBuffSz;
    unsigned char *iBuff;
    size_t pos_i;
    unsigned char *wBuff;
    size_t pos_w;
    
    int error;
};
char cfg[] = "lin2db.cf";
int main (int argc, char **argv) {
    char *temp;
    Loader l;
    if (-1 == l.load_config("lin2db.cfg")) {
        return -1;
    }
    int out = open("out.txt", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG |  S_IRWXO);
    write(out, l.iBuff, l.iBuffSz);
    close(out);
    return 0;
}