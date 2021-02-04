#pragma once

#include "loader.h"

class ConfigLoader : protected __config_loader {
public:
    inline ConfigLoader() {
        wLen = iBuffSz = pi = pw = 0;
        iBuff = wBuff = nullptr;
        int error = 0;
    }
    inline ~ConfigLoader() {
        clear();
    }
    
    inline int init(const char *FileName) {
        return loader_init((__config_loader *)this, FileName);
    }
    inline void clear() {
        loader_clear((__config_loader *)this);
    }
    inline const char* getBuffer() { return (char *)iBuff; }
    inline operator bool () { return ((nullptr != iBuff) && (nullptr != wBuff)); }
};