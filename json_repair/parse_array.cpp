#include "parse_array.hpp"
#include "constants.hpp"
#include "object_comparer.hpp"
#include <cctype>

std::vector<JSONReturnType> parse_array(JSONParser& parser) {
    std::vector<JSONReturnType> arr;
    parser.context.set(ContextValues::ARRAY);
    char current_char = parser.get_char_at();
    while (current_char && current_char != ']' && current_char != '}') {
        parser.skip_whitespaces();
        JSONReturnType value = std::string("");
        if (std::find(STRING_DELIMITERS.begin(), STRING_DELIMITERS.end(), std::string(1, current_char)) != STRING_DELIMITERS.end()) {
            size_t i = 1;
            i = parser.skip_to_character(current_char, i);
            i = parser.scroll_whitespaces(i + 1);
            if (parser.get_char_at(i) == ':') {
                value = parser.parse_object();
            } else {
                value = parser.parse_string();
            }
        } else {
            value = parser.parse_json();
        }

        if (ObjectComparer::is_strictly_empty(value)) {
            parser.index += 1;
        } else if (value == "..." && parser.get_char_at(-1) == '.') {
            parser.log("While parsing an array, found a stray '...'; ignoring it");
        } else {
            arr.push_back(value);
        }

        current_char = parser.get_char_at();
        while (current_char && current_char != ']' && (std::isspace(current_char) || current_char == ',')) {
            parser.index += 1;
            current_char = parser.get_char_at();
        }
    }

    if (current_char != ']') {
        parser.log("While parsing an array we missed the closing ], ignoring it");
    }

    parser.index += 1;
    parser.context.reset();
    return arr;
}