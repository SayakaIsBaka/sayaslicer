#pragma once

#include <iostream>
#include <string>

int FromBaseToDec(const char* s, int b, const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") {
    int res = 0;
    int len = strlen(s);
    for (int i = len - 1; i >= 0; i--) {
        const char* tmp = strchr(digits, s[i]);
        if (!tmp)
            return -1;
        int d = tmp - digits;
        if (d >= b)
            return -1;
        res += d * pow(b, len - (i + 1));
    }
    return res;
}

const std::string VerifyBaseAndConvert(const char* buf, int base) {
    if (strlen(buf) != 2)
        return buf;
    int res = FromBaseToDec(buf, base);
    if (res == -1)
        return buf;
    return std::to_string(res);
}

std::string FromDecToBase(unsigned int n, int b, const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")
{
    return (n / b ? FromDecToBase(n / b, b, digits) : "") + std::string(1, digits[n % b]);
}

int ToBaseString(char* buf, int buf_size, const void* p_data, int base) {
    auto val = *(const ImU32*)p_data;
    auto s = FromDecToBase(val, base);
    auto size = s.size();
    if (size == 1) {
        s.insert(0, 1, '0');
        size++;
    }
    memset(buf, 0, buf_size);
    memcpy(buf, s.c_str(), size >= buf_size ? buf_size : size);
    return size;
}