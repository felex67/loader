#include "loader.hpp"
#include <fstream>

int main (int argc, char **argv) {
    ConfigLoader Config;
    Config.init("log.cfg");
    std::ofstream fout;
    fout.open("out_cpp.cfg", std::ios::binary | std::ios::trunc);
    fout << Config.getBuffer();
    fout.close();
    return 0;
}