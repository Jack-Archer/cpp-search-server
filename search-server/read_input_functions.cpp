#include "read_input_functions.h"
#include <iostream>

std::string ReadLine()
{
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    std::cin >> result;
    ReadLine();
    return result;
}

std::vector<int> ReadNumbers()
{
    std::string s;
    std::getline(std::cin, s);
    int num;
    std::vector<int> numbers;

    while(std::cin>>s)
    {
        std::cin>>num;
        numbers.push_back(num);
    }

    return numbers;
}