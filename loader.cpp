#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <stdio.h>

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

const char QuotePairs[][2] = { { '"', '"' }, { '\'', '\'' } };
const char Braces[][2] = { { '(', ')'}, { '{' , '}' }, { '[', ']' } };
const char *Comments[] = { "//", "#", "/*"};
const short CStyle = {'//'};
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
size_t trimm_comments(char *Start) {
    char *tgt, *src;
    for (tgt = src = Start; (*src != 0); src++) {
        if (0 != *src) {
            if (';' < (*src)) { (*tgt) = (*src); ++tgt; }
            else {
                switch (*src) {
                case '/':
                    if (src[1] == '*') {
                        src += get_endof_c_comment(src);
                        break;
                    }
                case ';':
                case '#':
                    src += trimm_line(src);
                    break;
                default:
                    *tgt = *src;
                    ++tgt;
                }
            }
        }
        else { break; }
    }
    *tgt = 0;
    return (tgt - Start);
}
size_t trimm_line(char* Start) {
    char *temp = Start;
    while ((*temp) && ((*temp) != '\n')) {
        (*temp) = 0;
        ++temp;
    }
    return (temp - Start - 1);
}
size_t get_endof_c_comment(char *Start) {
    int n;
    //find last
    for (n = 2; 0 != Start[n]; n++ ) {
        if ('/' != Start[n] || '*' != Start[n - 1]) {}
        else { ++n; break; }
    }
    //erase part
    for (int i = 0; i < n; i++) { Start[i] = 0; }
    return n;
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
            if (Start[s])
        }
    }
    Start[t] = 0;
    return t;
}

int main (int argc, char **argv) {
    struct stat statbuff;
    char *buff;
    size_t total;
    FILE *in = fopen("lin2db.cfg", "r");
    if (-1 != fstat(in->_fileno, &statbuff)) {
        size_t size = statbuff.st_size;
        buff = new char[size + 1];
        fread(buff, size, 1, in);
        fclose(in);
        buff[size] = 0;
        printf("File: '%s'\n%s\n", "lin2db.cfg", buff);
        size = trimm_comments(buff);
        printf("comments removed, listing:\n%s\n", buff);
        size = trimm_nonquoted_spaces(buff);
        size = trimm_emty_strings(buff);
        printf("Spaces removed, listing:\n%s\n", buff);
        //size = strlen(buff);
        in = fopen("out.txt", "w");
        fwrite(buff, size, 1, in);
        delete[] buff;
    }
    fcloseall();
    return 0;
}