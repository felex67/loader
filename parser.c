#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define __config_parser_c__ 1

#include "parser.h"

#ifndef nullptr
    #define nullptr (0)
#endif // nullptr

    /** Конструктор интерпритатора */
    ConfigParser* config_parser_construct(ConfigParser *Ptr) {
        ConfigParser *Inst = Inst;
        if (nullptr == Inst) { Inst = (ConfigParser*)malloc(sizeof(ConfigParser)); }
        if (nullptr != Inst) {
            config_parser_preinit_instance(Inst);
            if (nullptr == Ptr) (*(struct __config_parser_flags*)&Inst->__private_flags).DynInst = 1;
        }
        return Inst;
    }
    /** Инициализатор интерпритатора */
    int config_parser_preinit_instance(ConfigParser *Inst) {
        *((void**)&Inst->destruct) = config_parser_destruct;
        *((void**)&Inst->reset) = config_parser_reset;
        *((int**)&Inst->init) = config_parser_init;
        *((int**)&Inst->set_counting_method) = config_parser_set_counting_method;
        *((int**)&Inst->set_building_method) = config_parser_set_building_method;

        *((int**)&Inst->get_var) = config_parser_get_var;
        *((int**)&Inst->parse_i) = config_parser_parse_i;
        *((int**)&Inst->parse_ui) = config_parser_parse_ui;
        *((int**)&Inst->parse_l) = config_parser_parse_l;
        *((int**)&Inst->parse_ul) = config_parser_parse_ul;
        *((int**)&Inst->parse_f) = config_parser_parse_f;
        *((int**)&Inst->parse_d) = config_parser_parse_f;
        *((char**)&Inst->parse_str) = config_parser_parse_str;

        *((int**)&Inst->__private_init_map) = config_parser_init_map;
        *((int**)&Inst->__private_count) = config_parser_count_map;
        *((int**)&Inst->__private_build) = config_parser_build_map;
        *((int**)&Inst->__private_counter) = nullptr;
        *((int**)&Inst->__private_builder) = nullptr;

        (*(size_t*)&(Inst->__private_gc)) = (*(size_t*)&(Inst->__private_vc)) = 0;
        (*(void**)&(Inst->__private_src)) = 0;
        (*(void**)&(Inst->__private_map)) = 0;
        (*(u_int32_t*)&(Inst->__private_flags)) = 0;
        (*(int*)&(Inst->Error)) = 0;

        return 0;
    }
    /** Деструктор интерпритатора */
    void config_parser_destruct(ConfigParser *Inst) {
        if (Inst->__private_flags.SrcInit) free(Inst->__private_src);
        if (Inst->__private_flags.MapInit) free(Inst->__private_map);
        if (Inst->__private_flags.DynInst) free(Inst);
    }
    /** Устанавливает флаги */

void config_parser_set_flag(ConfigParser* Inst, enum __config_parser_flags_e Flags) {
    u_int32_t *flags = (u_int32_t*)&(Inst->__private_flags);
    if (CP_FLAG_BTPUSR & Flags) (*((struct __config_parser_flags*)&(flags))).BldType = 0;
    if (CP_FLAG_CTPUSR & Flags) (*((struct __config_parser_flags*)&(flags))).CntType = 0;
    (*(flags)) |= Flags;
}
void config_parser_unset_flag(ConfigParser* Inst, enum __config_parser_flags_e Flags) {
    u_int32_t *flags = (u_int32_t*)&(Inst->__private_flags);
    if (CP_FLAG_BTPUSR & Flags) (*((struct __config_parser_flags*)flags)).BldType = 0;
    if (CP_FLAG_CTPUSR & Flags) (*((struct __config_parser_flags*)flags)).CntType = 0;
    (*(flags)) ^= (0xfffffff0 & Flags);
}
    /** Выполняет поиск переменой в массиве.
     * Если переменная с таким именем не найдена, возвращается нулевой указатель: ((config_variable_t*)0) */
    parser_variable_t* config_parser_get_var(const struct ConfigParser *Inst, const char *Grp, const char *Var) { return ((parser_variable_t*)0); }
    /** Сканирует значение как целое со знаком 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_i(void *Val, char *Var) { return -1; }
    /** Сканирует значение как целое без знака 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ui(void *Val, char *Var) { return -1; }
    /** Сканирует значение как целое со знаком 8 байт и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_l(void *Val, char *Var) { return -1; }
    /** Сканирует значение как целое без знака 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_ul(void *Val, char *Var) { return -1; }
    /** Сканирует значение как вещественное 4 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_f(void *Val, char *Var) { return -1; }
    /** Сканирует значение как вещественное 8 байта и записывает в Val
     * в случае неудачи возвращает -1, а поле остаётся нетронутым */
    int config_parser_parse_d(void *Val, char *Var) { return -1; }
    // Возвращает указатель на первый символ после ковычки
    const char *config_parser_parse_str(void *Val, char *Var) { return nullptr; }
