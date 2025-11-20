#include "parse_comment.hpp"
#include "constants.hpp"
#include <cctype>
#include <algorithm>

JSONReturnType parse_comment(JSONParser& parser) {
    char current_char = parser.get_char_at();
    std::vector<char> termination_characters = {'\n', '\r'};
    
    if (std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::ARRAY) != parser.context.getContext().end()) {
        termination_characters.push_back(']');
    }
    if (std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::OBJECT_VALUE) != parser.context.getContext().end()) {
        termination_characters.push_back('}');
    }
    if (std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::OBJECT_KEY) != parser.context.getContext().end()) {
        termination_characters.push_back(':');
    }
    
    if (current_char == '#') {
        std::string comment = "";
        while (current_char && 
               std::find(termination_characters.begin(), termination_characters.end(), current_char) == termination_characters.end()) {
            comment += current_char;
            parser.index += 1;
            current_char = parser.get_char_at();
        }
        parser.log("Found line comment: " + comment + ", ignoring");
    }
    else if (current_char == '/') {
        char next_char = parser.get_char_at(1);
        if (next_char == '/') {
            std::string comment = "//";
            parser.index += 2;
            current_char = parser.get_char_at();
            while (current_char && 
                   std::find(termination_characters.begin(), termination_characters.end(), current_char) == termination_characters.end()) {
                comment += current_char;
                parser.index += 1;
                current_char = parser.get_char_at();
            }
            parser.log("Found line comment: " + comment + ", ignoring");
        }
        else if (next_char == '*') {
            std::string comment = "/*";
            parser.index += 2;
            while (true) {
                current_char = parser.get_char_at();
                if (!current_char) {
                    parser.log("Reached end-of-string while parsing block comment; unclosed block comment.");
                    break;
                }
                comment += current_char;
                parser.index += 1;
                if (comment.length() >= 2 && comment.substr(comment.length() - 2) == "*/") {
                    break;
                }
            }
            parser.log("Found block comment: " + comment + ", ignoring");
        }
        else {
            parser.index += 1;
        }
    }
    
    if (parser.context.isEmpty()) {
        return parser.parse_json();
    } else {
        return std::string("");
    }
}