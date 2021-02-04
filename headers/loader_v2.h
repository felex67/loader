#ifndef __config_loader_h__
#define __config_loader_h__ 1

#include <sys/types.h>



    struct __config_loader {
        char eBuff[2048];
        size_t wLen;
        size_t iBuffSz;
        unsigned char *iBuff;
        size_t pi;
        unsigned char *wBuff;
        size_t pw;
        int error;

        /** если равны нулю к моменту вызова loader_init(__config_loader *)
         *  то будут установлены умолчания */
        char *NotSpace;
        char *LineComm;
        char *Quotes;
    };
    
    extern struct __config_loader ConfigLoader;
    
    //прототипы
    /**
     * Загружает и отчищает файл от коментариев, разбивает на строки, удаляет пустые
    */
    int loader_init(struct __config_loader *I, const char* FileName);
    // устанавливает типы строчных комментариев
    int loader_set_string_comments(struct __config_loader *I, const char* Keys);
    // очищает буферы
    void loader_clear(struct __config_loader *I);
    // 1 - если успешно загружен, 0 - в случае ошибки
    int loader_isInit(struct __config_loader *I);
    
    #ifdef __config_loader_c__

        char loader_NonSpaceChars[] = "\t\n";
        char loader_Default_SComm[]  = ";#";
        char loader_Default_Quotes[] = "`'\"";
        
        // инициализирует буфферы и считывает в них файл
        int loader_init_buffers(struct __config_loader *I, const char * FileName);
        // отчищает файл от комментариев
        int loader_trimm_comments(struct __config_loader *I);
        // вычисляет конец "Си" комментария
        int loader_get_endof_c_comment(struct __config_loader *I);
        // вычисляет конец строчного коментария
        int loader_get_endof_s_comment(struct __config_loader *I);
        // удаляет пробелы
        int loader_trimm_spaces(struct __config_loader *I);
        // удаляет пустые строки
        int loader_trimm_emty_strings(struct __config_loader *I);
        // меняет буфферы местами
        void loader_swap_buffers(struct __config_loader *I);
        // выводит ошибки в терминал
        void loader_perror(struct __config_loader *I, const char * What, int Error);
        // меняет символы местами
        int swap_c(char *c1, char *c2);
        // сортирует строку по возрастанию
        int strsort_az(char *a);
        // сортирует строку по убыванию
        int strsort_za(char *a);
        // разворачивает строку
        int str_inverse(char *a);
        

    #endif // __config_loader_cpp__
    
#endif // __config_loader_h__