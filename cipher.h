#pragma once

#include <array>

class CodeBlock;

namespace Kuznyechik
{
using Key = std::array<uint8_t, 32>;
class Cipher
{
    static constexpr uint16_t mask = 0x1C3; // x^8 + x^7 + x^6 + x + 1;
    static uint16_t mul(uint8_t, uint8_t);
private:
    static void R(CodeBlock&);
    static void revertR(CodeBlock&);
    static void S(CodeBlock&);
    static void L(CodeBlock&);
    static void revertS(CodeBlock&);
    static void revertL(CodeBlock&);
public:
    static void X(CodeBlock&, CodeBlock&);
    static void encrypt(CodeBlock&, Key const&);
    static void decrypt(CodeBlock&, Key const&); //TODO for crt mode decrypt isn't needed, but ... :)
};

}


