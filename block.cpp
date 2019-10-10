#include "Exeptions.h"
#include "block.h"
#include "Logger.h"
#include <cstring>

namespace details
{

inline uint16_t convert(char const& a)
{
    if(a >= '0' && a <= '9')
    {
        return a - '0';
    }else if(a >= 'A' && a <= 'F')
    {
        return a - 'A' + 10;
    } else if(a >= 'a' && a <= 'f')
    {
        return a - 'a' + 10;
    } else
    {
        throw error::ExeptionBase<error::ErrorList::InputError>(std::string("Can't convert value: ") + a);
    }
}
enum  BitDepth : size_t
{
low = 0,
high = 4
};
template<BitDepth bit>
inline char convert(uint8_t const& a)
{
    uint8_t r = (a >> bit) & 0b1111;
    if(r < 10)
    {
        return '0' + r;
    }else
    {
        return 'A' + (r - 10);
    }
}

}

CodeBlock::CodeBlock(Block const & a) : block(a){}

uint8_t CodeBlock::operator[](size_t i)
{
    return (block[i >> 3] & (1 << i & 0x7)) >> (i & 0x7);
}

void CodeBlock::get(uint8_t* it, size_t first, size_t length)
{
    Logger::print<Log::Level::info>("%d %d", first, length);
    if(first + length > 16 || first > 16) throw error::ExeptionBase<error::ErrorList::FatalTrace>("Out of range");
    std::memcpy(it, &block[first], length);
}

uint8_t CodeBlock::get(size_t first)
{
    if(first > 16) throw error::ExeptionBase<error::ErrorList::FatalTrace>("Out of range");
    return block[first];
}


std::string CodeBlock::toString() const
{
    std::string str;
    str.resize(block.size() * 2);
    for(size_t i = 0; i < block.size(); i++)
    {
        str[2 * i] = details::convert<details::BitDepth::high>(block[block.size() - 1 - i]);
        str[2 * i + 1] = details::convert<details::BitDepth::low>(block[block.size() - 1 - i]);
    }
    return str;
}

void CodeBlock::setByString(std::string str)
{
    //if(str.size() != block.size() * 2) throw error::ExeptionBase<error::ErrorList::InputError>("Too few values for setting block");
    auto iterator = block.size() - 1;
    size_t bitSize = 0;
    block.fill(0);
    for(auto i : str)
    {
        try
        {
            block[iterator - (bitSize >> 3)] += ((bitSize % 8 == 0 ? (details::convert(i) << 4) : details::convert(i)));
        }
        catch(error::Exeption& exp)
        {
            block.fill(0);
            Logger::print<Log::Level::fatal>(exp.what().c_str());
            throw error::ExeptionBase<error::ErrorList::FatalTrace>("Fail function BigInt::setByString()");
        }
        bitSize += 4;
    }
}

Block& CodeBlock::get()
{
    return block;
}
