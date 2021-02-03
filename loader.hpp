#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

struct __config_loader {
public:
    __config_loader(const char* FileName);
    ~__config_loader();
    operator bool ();
private:
    /**
     * Загружает и отчищает файл от коментариев,
     * разбивает на строки и удаляет пустые
    */
    int init_buffer(const char * FileName);
    //Загружает файл и создаёт 2 буффера
    int load_config(const char *FileName);
    //отчищает файл от комментариев
    int trimm_comments();
    //вычисляет конец "Си" комментария
    int get_endof_c_comment();
    //вычисляет конец строчного коментария
    int get_endof_s_comment();
    int trimm_spaces();
    int trimm_emty_strings();
    void swap_buffers();
protected:
    void perror(const char * What);
    void perror(const char * What, int Error);
protected:
    char eBuff[2048];
    size_t wLen;
    size_t iBuffSz;
    unsigned char *iBuff;
    size_t pos_i;
    unsigned char *wBuff;
    size_t pos_w;
    int error;
};