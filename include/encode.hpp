#pragma once

#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>

class Encode
{
public:
    std::string static gethash(const std::string &src)
    {
        std::string res;
        CryptoPP::SHA256 hash;
        CryptoPP::StringSource
            ss(src,
               true,
               new CryptoPP::HashFilter(
                   hash,
                   new CryptoPP::HexEncoder(
                       new CryptoPP::StringSink(res))));
        return res;
    }

    static bool verify(const std::string &src, const std::string &hash)
    {
        return gethash(src) == hash;
    }
};