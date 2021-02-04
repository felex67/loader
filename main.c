#include "loader.hpp"

int main (int argc, char **argv) {
    const char *Config = "log.cfg";
    struct __config_loader loader;
    loader_init(&loader, Config);
    //printf("%s\n", loader.iBuff);
    int fout = open("out_c.cfg", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    write(fout, loader.iBuff, loader.iBuffSz);
    close(fout);
    loader_clear(&loader);

    return 0;
}