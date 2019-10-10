#pragma once

#include <istream>
class CodeBlock;
class Buffer
{
public:
    explicit Buffer(std::ifstream& file):stream(file){};
    size_t getBlock(CodeBlock& block);
private:
    std::ifstream& stream;
    char buffer[16];
};

