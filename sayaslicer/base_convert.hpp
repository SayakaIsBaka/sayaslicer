#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cmath>

const std::string VerifyBaseAndConvert(const char* buf, int base);
int ToBaseString(char* buf, int buf_size, const void* p_data, int base);
int FromBaseToDec(const char* s, int b, const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");