#include "elf.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <cstring>

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        return 69;
    }
    auto size = std::filesystem::file_size(argv[1]);
    std::uint8_t* buf = new std::uint8_t[size];
    std::ifstream file{argv[1]};
    file.read(static_cast<char*>((void*)buf), size);
    assert(file.gcount() == size);

    libelf::Elf my_file{buf, size};
    auto& hdr = my_file.header();
    auto* secs = my_file.sections();

    for (auto i = 0; i < hdr.e_shnum; i++) {
        std::cout << my_file.string(secs[i].sh_name) << "\t\t\t\t\t"; 
        std::cout << secs[i].sh_size << "\t\t\t\t\t";
        std::cout << "\n";
    }

    delete[] buf;
}