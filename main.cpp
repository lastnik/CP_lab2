#include <iostream>
#include <cstring>
#include "cipher.h"
#include "block.h"
#include "Logger.h"
#include "Buffer.h"
constexpr char const* file = "config.json";
struct config
{
std::string IV;
std::string Key;
std::string File;
std::string LogLevel;
std::string mode;
};
std::string find(std::string const& str, std::string const& key)
{
    auto k = str.find(key);
    if(k == std::string::npos)
    {
        throw error::ExeptionBase<error::ErrorList::InputError>("Lose mandatory field " + key);
    }
    k = str.find(':', k + 1);
    if(k == std::string::npos)
    {
        throw error::ExeptionBase<error::ErrorList::InputError>("Lose mandatory value of field " + key);
    }
    auto start = str.find("\"",k + 1);
    auto end   = str.find( "\"", start + 1);
    if(end == str.size())
        end  = str.find( "\n", start + 1);

    return str.substr(start + 1, end - start - 1);
};
config parse(std::string const& str)
{
    config cfg;
    cfg.IV = find(str,"InitVect");
    cfg.Key = find(str,"Key");
    cfg.File = find(str,"File");
    cfg.LogLevel = find(str,"LogLevel");
    cfg.mode    = find(str,"Mode");
    return cfg;
}
int main() {
    Logger::setLevel(Log::Level::debug);
    bool opened = true;
    try {
        Logger::start();
    }
    catch (error::Exeption &exp) {
        std::cout << exp.what() << std::endl;
        return -1;
    }
    CodeBlock IV, k1, k2;
    uint64_t Iteration = 0;
    Kuznyechik::Key key;
    bool mode = false; // encrypt
    std::fstream in(file, std::ios_base::in);
    std::ifstream stream;
    std::ofstream result;
    std::string str;
    stream.exceptions(std::ifstream::eofbit);
    Buffer buf(stream);
    if (!in.is_open()) {
        Logger::print<Log::Level::fatal>((std::string("Can't open file: ") + file).c_str());
        opened = false;
    } else {
        std::stringstream buf;
        buf << in.rdbuf();
        std::string json = buf.str();
        try {
            auto cfg = parse(json);
            Logger::setLevel(cfg.LogLevel);
            if (cfg.mode == "encrypt") mode = false;
            if (cfg.mode == "decrypt") mode = true;
            stream.open(cfg.File, std::ios_base::binary);
            if (!stream.is_open()) {
                Logger::print<Log::Level::fatal>((std::string("Can't open file: ") + cfg.File).c_str());
                opened = false;
            }
            k2.setByString(cfg.Key.substr(32, 32));
            k1.setByString(cfg.Key.substr(0, 32));
            Logger::print<Log::Level::debug>("Key = %s", (k1.toString() + k2.toString()).c_str());
            std::memcpy(&key[0], &k1.get()[0], 16);
            std::memcpy(&key[16], &k2.get()[0], 16);
            IV.setByString(cfg.IV);
            Logger::print<Log::Level::debug>("IV = %s", IV.toString().c_str());
            str = cfg.File;
        }
        catch (error::Exeption &exp) {
            Logger::print<Log::Level::fatal>(exp.what().c_str());
            std::cout << exp.what() << std::endl;
            opened = false;
        }
    }
    try{
        if (opened) {
            if (mode) {
                //decrypt
                result.open(str.substr(0, str.size() - 4), std::ios_base::binary);
                char *buffer = new char[16];
                CodeBlock a;
                bool end = false;
                while (!end) {
                    a.setZero();
                    int32_t size = buf.getBlock(a);
                    Logger::print<Log::Level::info>("size = %d", size);
                    if(size == 0)
                    {
                        end = true;
                        break;
                    }
                    CodeBlock b = IV;
                    Kuznyechik::Cipher::encrypt(b, key);
                    Kuznyechik::Cipher::X(a, b);

                    Logger::print<Log::Level::info>("plop = %s %x", a.toString().c_str(), a.get(15));
                    bool cbc = true;
                    if (a.get()[15] <= 16 && a.get()[15] > 0) {
                        for (size_t i = 16 - a.get()[15]; i < 16; i++) {
                            Logger::print<Log::Level::info>("%x", a.get(i));
                            if (a.get(15) != a.get(i)) {
                                cbc = false;
                                break;
                            }
                        }
                    }else
                    {
                        cbc = false;
                    }
                    if (cbc) {
                        size = 16 - a.get()[15];
                        end = true;
                    } else
                    {
                        size = 16;
                    }
                    a.get((uint8_t *) buffer, 0, size);
                    result.write(buffer, size);
                    Iteration++;
                    for (size_t i = 0; i < 8; i++)
                        IV.get()[i] = ((uint8_t *) &Iteration)[i];
                }
            } else {
                //encrypt
                char *buffer = new char[16];
                result.open(str + ".kuz", std::ios_base::binary);
                CodeBlock a;
                a.setZero();
                bool end = false;
                while (!end) {
                    //Logger::print<Log::Level::debug>("Iter = %d", Iteration);
                    a.setZero();
                    auto size = buf.getBlock(a);
                    //Logger::print<Log::Level::debug>("size = %d", size);
                    if (size != 16) {
                        end = true;
                        for (size_t i = size; i < 16; i++) {
                            a.get()[i] = 16 - size;
                        }
                    }
                    CodeBlock b = IV;
                    Kuznyechik::Cipher::encrypt(b, key);
                    Kuznyechik::Cipher::X(a, b);
                    a.get((uint8_t *) buffer, 0, 16);
                    result.write(buffer, 16);
                    Iteration++;
                    for (size_t i = 0; i < 8; i++)
                        IV.get()[i] = ((uint8_t *) &Iteration)[i];
                }
            }
        }
    }catch (error::Exeption &exp)
    {
        Logger::print<Log::Level::fatal>(exp.what().c_str());
        std::cout << exp.what() << std::endl;
        opened = false;
    }
    stream.close();
    Logger::stop();
}