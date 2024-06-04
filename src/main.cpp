#include <iostream>
#include <fstream>
#include <cstdint>

#include "psx.hpp"

uint8_t *LoadBIOSFile(const char *file_name)
{
    std::ifstream file(file_name, std::ios::binary);
    if (file.fail())
        return nullptr;

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto *buffer = new uint8_t[size];
    file.read(reinterpret_cast<char *>(buffer), size);

    return buffer;
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: psx [bios rom]" << std::endl;
        return 1;
    }

    uint8_t *bios = LoadBIOSFile(argv[1]);
    if (!bios)
    {
        std::cerr << "Invalid BIOS rom" << std::endl;
        return 1;
    }

    PSX psx(bios);
    psx.Run();

    delete[] bios;

    return 0;
}