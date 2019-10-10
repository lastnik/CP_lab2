#include "cipher.h"
#include "block.h"
#include <cstring>
#include "Logger.h"
namespace Kuznyechik
{
namespace
{
std::array<uint8_t, 256> substitutionTable =
{
        252, 238, 221,  17, 207, 110,  49,  22,  251,  196,  250,  218,  35,  197,  4,  77,  233,  119,  240,  219,
        147, 46,  153, 186,  23, 54, 241, 187, 20, 205, 95,  193, 249, 24,  101,90, 226, 92, 239, 33,  129, 28, 60,
        66,  139,   1, 142,  79, 5,  132, 2,  174, 227,  106,  143,  160, 6, 11, 237, 152,  127, 212, 211,31,235, 52, 44,
        81,  234, 200,  72, 171, 242, 42,  104,  162,  253,  58,  206,  204,  181,  112,  14, 86, 8,  12,  118,  18,  191,
        114, 19,   71, 156, 183,  93,  135, 21,  161,  150, 41,  16,  123,  154,  199, 243,  145,  120,  111,  157,
        158, 178, 177,  50, 117,  25, 61, 255, 53,  138, 126,  109,  84,  198,  128,  195,  189,  13,  87, 223,
        245, 36,  169,  62, 168,  67, 201, 215,  121, 214, 246,  124,  34, 185,  3, 224,  15, 236, 222,  122,  148,
        176, 188, 220, 232,  40,  80, 78, 51,  10, 74,  167,  151, 96, 115, 30, 0, 98, 68,  26,  184,  56,  130,  100,
        159, 38,   65, 173,  69, 70,  146,  39,  94, 85,  47,  140,  163,  165,  125,  105, 213,  149, 59,  7, 88,  179,
        64,  134, 172,  29, 247,  48,  55,  107,  228,  136,  217,  231,  137,  225,  27,  131,  73, 76, 63, 248, 254,
        141, 83,  170, 144, 202, 216,  133, 97, 32,  113,  103,  164, 45, 43, 9, 91,203,  155, 37, 208, 190, 229,  108,
        82, 89,  166,  116, 210, 230, 244, 180,  192, 209,  102,  175,  194, 57, 75, 99,  182
};

std::array<uint16_t, 16> linearConst = {1, 148, 32, 133, 16, 194, 192, 1, 251, 1, 192, 194, 16, 133, 32, 148};
}

void Cipher::R(CodeBlock& block)
{
    auto& b = block.get();
    uint16_t last = 0;
    for(size_t i = 0; i < 16; i++)
    {
        uint16_t res =  mul(b[i], linearConst[i]); // best way is calculated table and style like subst;
        while(res > 0xFF)
        {
            uint16_t bitMask = 0x8000;
            uint8_t val = 7;
            while((res & bitMask) == 0)
            {
                bitMask >>= 1;
                val--;
            }
            res = res ^ (mask << val);
        }
        last ^= res;
        //if(last > 0xFF) last ^= mask;
        if(i != 15)
            b[i] = b[i + 1];
        else
            b[i] = last;
    }
}

void Cipher::revertR(CodeBlock & b)
{

}

void Cipher::X(CodeBlock& block, CodeBlock& key)
{
    auto& b  = block.get();
    auto& k = key.get();
    for(size_t i = 0; i < b.size(); i++)
    {
        b[i] = b[i] ^ k[i];
    }
}

void Cipher::S(CodeBlock& block)
{
    auto& b = block.get();
    for(auto& i : b)
    {
        i = substitutionTable[i];
    }
}

void Cipher::L(CodeBlock& block)
{
    for(size_t iterator = 0; iterator < 16; iterator++)
    {
        R(block);
    }
}

void Cipher::revertS(CodeBlock &)
{

}

void Cipher::revertL(CodeBlock &)
{

}

void Cipher::encrypt(CodeBlock& block, Key const& key)
{
    CodeBlock K1,K2,C, old;
    auto& k1 = K1.get(); auto& k2 = K2.get(); auto& c = C.get();
    c.fill(0);
    std::memcpy(&k2[0], &key[0], 16);
    std::memcpy(&k1[0], &key[16], 16);
    size_t numKey = 1;
    Logger::print<Log::Level::info>("input block  = %s", block.toString().c_str());
    for(size_t i = 0; i < 9; i++)
    {
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        Logger::print<Log::Level::debug>("|                              ROUND                                   |");
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        if(i % 2)
            X(block, K2);
        else
            X(block, K1);
        Logger::print<Log::Level::debug>("result after X %d: block = %s", i, block.toString().c_str());
        S(block);
        Logger::print<Log::Level::debug>("result after S %d: block = %s", i, block.toString().c_str());
        L(block);
        Logger::print<Log::Level::debug>("result after L %d: block = %s", i, block.toString().c_str());
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        Logger::print<Log::Level::debug>("|                            END ROUND                                 |");
        Logger::print<Log::Level::debug>("________________________________________________________________________");

        if(i % 2 == 0 || i == 9) continue;
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        Logger::print<Log::Level::debug>("|                      ROUND_OF_KEY_GENERATION                         |");
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        for(size_t j = (numKey - 1) * 8 + 1; j < (numKey) * 8 + 1; j++)
        {
            c.fill(0);
            c[0] = j;
            L(C);
            old = K2;
            K2  = K1;
            X(K1, C);
            S(K1);
            L(K1);
            X(K1, old);
        }
        Logger::print<Log::Level::debug>("K1 =%s K2 = %s", K1.toString().c_str(), K2.toString().c_str());
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        Logger::print<Log::Level::debug>("|                        END_OF_KEY_GENERATION                         |");
        Logger::print<Log::Level::debug>("________________________________________________________________________");
        numKey++;
    }
    X(block, K2);
    Logger::print<Log::Level::info>("output block = %s", block.toString().c_str());
}

void Cipher::decrypt(CodeBlock&, Key const&) {

}

uint16_t Cipher::mul(uint8_t a, uint8_t b)
{
    uint16_t res = 0;
    for(size_t i = 0; i < 8; i++)
    {
        if(b & (1 << i))
        {
            res ^= (uint16_t (a) << i);
        }
    }
    return res;
}

}
