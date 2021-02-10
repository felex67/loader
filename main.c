#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/parser.h"


int main (const int argc, const char *argv[]) {
    ConfigParser *Parser = new_config_parser(0);
    if (Parser) {
        if (-1 != Parser->init(Parser, "log.cfg")) {
            parser_group_t *map = *((parser_group_t**)&Parser->__private_map);
            for (size_t G = 0; G < Parser->__private_gc; G++) {
                printf("[%s] Vars: %lu\n", map[G].Name, map[G].VarCnt);
                for (size_t V = 0; V < map[G].VarCnt; V++) {
                    printf("'%s'='%s'\n", map[G].Vars[V].Var, map[G].Vars[V].Val);
                }
            }
        }
        Parser->reset(Parser);
        if (-1 != Parser->init(Parser, "test.cfg")) {
            parser_group_t *map = *((parser_group_t**)&Parser->__private_map);
            printf("\n");
            for (size_t G = 0; G < Parser->__private_gc; G++) {
                printf("[%s] Vars: %lu\n", map[G].Name, map[G].VarCnt);
                for (size_t V = 0; V < map[G].VarCnt; V++) {
                    printf("'%s'='%s'\n", map[G].Vars[V].Var, map[G].Vars[V].Val);
                }
            }
        }
        Parser->destruct(Parser);
    }
    return 0;
}
