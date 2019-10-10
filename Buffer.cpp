#include "Logger.h"
#include "Buffer.h"
#include "block.h"
#include <cstring>
size_t Buffer::getBlock(CodeBlock &block)
{
    auto old = stream.tellg();
    size_t size;
    try
    {
        size = stream.readsome(buffer, 16);
        Logger::print<Log::Level::info>("%d", size);
        memcpy(&block.get()[0], buffer, size);
    }catch(std::ifstream::failure e)
    {
        Logger::print<Log::Level::debug>("%s", e.what());
    }
    return size;
}
