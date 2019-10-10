#pragma once

#include <array>
#include <cstdint>

using Block = std::array<uint8_t, 16>;
class CodeBlock
{
public:
    CodeBlock() = default;
    CodeBlock(CodeBlock const&) = default;
    explicit CodeBlock(Block const&);
    void setByString(std::string);
    uint8_t operator[](size_t);
    void get(uint8_t*, size_t , size_t); // 8 bits mini blocks
    uint8_t get(size_t); //one 8 bits mini block
    Block& get(); // return reference
    std::string toString() const;
    void setZero()
    {
        block.fill(0);
    }
private:
    Block block; //128 bit block of coding
};

