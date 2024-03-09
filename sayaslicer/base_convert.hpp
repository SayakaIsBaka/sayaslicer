#pragma once

#include <iostream>
#include <string>

const std::string VerifyBaseAndConvert(const char* buf, int base);
int ToBaseString(char* buf, int buf_size, const void* p_data, int base);