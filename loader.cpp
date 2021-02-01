#include <sys/types.h>
#include <cstring>

struct Entry {
    char *var;
    char *val;
    Entry(char* var, char* val): var(var), val(val) {}
    ~Entry() { delete[] var; delete[] val; }
};

const char QuotePairs[][2] = { { '"', '"' }, { '\'', '\'' } };
const char Braces[][2] = { { '(', ')'}, { '{' , '}' }, { '[', ']' } };
const char *Comments[] = { "//", "#", "/*"};
const short CStyle = {'//'};

size_t get_closingQuote(char Quote, const char* Start) {
    for (size_t i = 0; Start[i] != Quote; i++) {
        if (Start[i]) {
            if (Start[i] != Quote);
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
size_t trimm_comments(char *Start, size_t MaxLen) {
    size_t len = 0;
    char *tgt = Start, *src = Start;
    if ((*Start == '/') && (Start[1] == '/')) {
        src += trimm_line(Start);
    }

}
size_t trimm_line(char* Start) {
    char *temp = Start;
    while (*temp && (*temp != '\n')) {
        *temp++ = 0;
    }
    if (*temp) {
        *temp = 0;
    }
    return temp - Start + 1;
}
int main (int argc, char **argv) {
    
    return 1;
}