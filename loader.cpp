#include "loader.hpp"

    __config_loader::__config_loader(const char* FileName) : eBuff() {
        load_config(FileName);
    }
    __config_loader::~__config_loader() {
        if (nullptr != iBuff)
            free(iBuff);
        if (nullptr != wBuff)
            free(wBuff);
    }
    __config_loader::operator bool () { return nullptr != iBuff; }
    int __config_loader::init_buffer(const char * FileName) {
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
    int __config_loader::load_config(const char *FileName) {
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
    int __config_loader::trimm_comments() {
        for (pos_w = 0, pos_i = 0; 0 != iBuff[pos_i]; pos_i++) {
            if ('/' < iBuff[pos_i]) { wBuff[pos_w] = iBuff[pos_i]; ++pos_w; }
            else {
                switch (iBuff[pos_i]) {
                case '/':
                    if (iBuff[pos_i + 1] == '*') {
                        get_endof_c_comment();
                        break;
                    }
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
    int __config_loader::get_endof_c_comment() {
        for (pos_i+= 2; 0 != iBuff[pos_i]; pos_i++ ) {
            if ('*' == iBuff[pos_i] && '/' == iBuff[pos_i + 1]) { pos_i++; break; }
        }
        return 0;
    }
    //вычисляет конец строчного коментария
    int __config_loader::get_endof_s_comment() {
        while ((0 != iBuff[pos_i]) && ('\n' != iBuff[pos_i])) { ++pos_i; }
        --pos_i;
        return 0;
    }
    int __config_loader::trimm_spaces() {
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
    int __config_loader::trimm_emty_strings() {
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
    void __config_loader::swap_buffers() {
        unsigned char *t = wBuff;
        wBuff = iBuff;
        iBuff = t;
        iBuffSz = pos_i;
        pos_i = pos_w = 0;
    }
    void __config_loader::perror(const char * What) {
        sprintf((char *)eBuff, "ERROR [%i] '%s'", errno, What);
        perror((char *)eBuff);
    }
    void __config_loader::perror(const char * What, int Error) {
        sprintf((char *)eBuff, "ERROR [%i] '%s'", Error, What);
        perror((char *)eBuff);
    }
