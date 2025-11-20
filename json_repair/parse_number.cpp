#include "parse_number.hpp"
#include "constants.hpp"
#include <cctype>
#include <sstream>

JSONReturnType parse_number(JSONParser& parser) {
    std::string number_str = "";
    char current_char = parser.get_char_at();
    bool is_array = (parser.context.getCurrent() == ContextValues::ARRAY);
    
    while (current_char && 
           NUMBER_CHARS.find(current_char) != NUMBER_CHARS.end() && 
           (!is_array || current_char != ',')) {
        number_str += current_char;
        parser.index += 1;
        current_char = parser.get_char_at();
    }
    
    if (!number_str.empty() && 
        (number_str.back() == '-' || number_str.back() == 'e' || 
         number_str.back() == 'E' || number_str.back() == '/' || 
         number_str.back() == ',')) {
        number_str.pop_back();
        parser.index -= 1;
    } else if (current_char && std::isalpha(current_char)) {
        parser.index -= number_str.length();
        return parser.parse_string();
    }
    
    if (number_str.find(',') != std::string::npos) {
        return number_str;
    }
    
    if (number_str.find('.') != std::string::npos || 
        number_str.find('e') != std::string::npos || 
        number_str.find('E') != std::string::npos) {
        try {
            double value = std::stod(number_str);
            return value;
        } catch (const std::exception&) {
            return number_str;
        }
    } else {
        try {
            int value = std::stoi(number_str);
            return value;
        } catch (const std::exception&) {
            return number_str;
        }
    }
}