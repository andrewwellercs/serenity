#pragma once

#include <AK/AKString.h>
#include <AK/Vector.h>

struct KSym {
    u32 address;
    const char* name;
};

const KSym* ksymbolicate(u32 address);
void load_ksyms();
void init_ksyms();

extern bool ksyms_ready;
extern u32 ksym_lowest_address;
extern u32 ksym_highest_address;

void dump_backtrace();
