#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <vector>
#include <set>
#include <string>

const std::vector<std::string> STRING_DELIMITERS = {
    "\"",  // regular double quote
    "'",   // regular single quote
    "“",   // left double quotation mark
    "”"    // right double quotation mark
};

const std::set<char> NUMBER_CHARS = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '-', '.', 'e', 'E', '/', ','
};

#endif