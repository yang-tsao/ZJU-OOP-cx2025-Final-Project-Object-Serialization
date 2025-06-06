#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace base64
{
    void base64_encode(std::string &out, const std::vector<uint8_t> &buf);
    void base64_encode(std::string &out, const uint8_t *buf, size_t bufLen);
    void base64_encode(std::string &out, std::string const &buf);

    void base64_decode(std::vector<uint8_t> &out, std::string const &encoded_string);

    // Use this if you know the output should be a valid std::string
    void base64_decode(std::string &out, std::string const &encoded_string);
} // namespace base64
