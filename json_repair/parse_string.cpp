#include "parse_string.hpp"
#include "parse_array.hpp"
#include "parse_comment.hpp"
#include "constants.hpp"
#include "json_context.hpp"
#include <cctype>
#include <algorithm>

std::string parse_string(JSONParser& parser) {
    auto _append_literal_char = [&parser](std::string acc, char current_char) -> std::pair<std::string, char> {
        acc += current_char;
        parser.index += 1;
        char new_char = parser.get_char_at();
        return {acc, new_char};
    };

    bool missing_quotes = false;
    bool doubled_quotes = false;
    char lstring_delimiter = '"';
    char rstring_delimiter = '"';

    char current_char = parser.get_char_at();
    if (current_char == '#' || current_char == '/') {
        // return parser.parse_comment();
        return "";
    }
    
    while (current_char && 
           std::find(STRING_DELIMITERS.begin(), STRING_DELIMITERS.end(), std::string(1, current_char)) == STRING_DELIMITERS.end() && 
           !std::isalnum(current_char)) {
        parser.index += 1;
        current_char = parser.get_char_at();
    }

    if (!current_char) {
        return "";
    }

    if (current_char == '\'') {
        lstring_delimiter = '\'';
        rstring_delimiter = '\'';
    }  else if (std::isalnum(current_char)) {
        if ((current_char == 't' || current_char == 'T' || current_char == 'f' || 
             current_char == 'F' || current_char == 'n' || current_char == 'N') && 
            parser.context.getCurrent() != ContextValues::OBJECT_KEY) {
            // Simplified boolean parsing
            if (std::tolower(current_char) == 't') {
                // Check if it's "true"
                if (parser.get_char_at(1) == 'r' && parser.get_char_at(2) == 'u' && parser.get_char_at(3) == 'e') {
                    parser.index += 4;
                    return "true";
                }
            } else if (std::tolower(current_char) == 'f') {
                // Check if it's "false"
                if (parser.get_char_at(1) == 'a' && parser.get_char_at(2) == 'l' && parser.get_char_at(3) == 's' && parser.get_char_at(4) == 'e') {
                    parser.index += 5;
                    return "false";
                }
            } else if (std::tolower(current_char) == 'n') {
                // Check if it's "null"
                if (parser.get_char_at(1) == 'u' && parser.get_char_at(2) == 'l' && parser.get_char_at(3) == 'l') {
                    parser.index += 4;
                    return "null";
                }
            }
        }
        parser.log("While parsing a string, we found a literal instead of a quote");
        missing_quotes = true;
    }

    if (!missing_quotes) {
        parser.index += 1;
    }
    
    if (parser.get_char_at() == '`') {
        // Simplified JSON block parsing
        parser.log("While parsing a string, we found code fences but they did not enclose valid JSON, continuing parsing the string");
    }
    
    if (parser.get_char_at() == lstring_delimiter) {
        if ((parser.context.getCurrent() == ContextValues::OBJECT_KEY && parser.get_char_at(1) == ':') || 
            (parser.context.getCurrent() == ContextValues::OBJECT_VALUE && 
             (parser.get_char_at(1) == ',' || parser.get_char_at(1) == '}'))) {
            parser.index += 1;
            return "";
        } else if (parser.get_char_at(1) == lstring_delimiter) {
            parser.log("While parsing a string, we found a doubled quote and then a quote again, ignoring it");
            return "";
        }
        size_t i = parser.skip_to_character(rstring_delimiter, 1);
        char next_c = parser.get_char_at(i);
        if (parser.get_char_at(i + 1) == rstring_delimiter) {
            parser.log("While parsing a string, we found a valid starting doubled quote");
            doubled_quotes = true;
            parser.index += 1;
        } else {
            i = parser.scroll_whitespaces(1);
            next_c = parser.get_char_at(i);
            if (std::find(STRING_DELIMITERS.begin(), STRING_DELIMITERS.end(), std::string(1, next_c)) != STRING_DELIMITERS.end() || 
                next_c == '{' || next_c == '[') {
                parser.log("While parsing a string, we found a doubled quote but also another quote afterwards, ignoring it");
                parser.index += 1;
                return "";
            } else if (!(next_c == ',' || next_c == '}' || next_c == ']')) {
                parser.log("While parsing a string, we found a doubled quote but it was a mistake, removing one quote");
                parser.index += 1;
            }
        }
    }

    std::string string_acc = "";

    current_char = parser.get_char_at();
    bool unmatched_delimiter = false;
    
    while (current_char && current_char != rstring_delimiter) {
        if (missing_quotes) {
            if (parser.context.getCurrent() == ContextValues::OBJECT_KEY && 
                (current_char == ':' || std::isspace(current_char))) {
                parser.log("While parsing a string missing the left delimiter in object key context, we found a :, stopping here");
                break;
            } else if (parser.context.getCurrent() == ContextValues::ARRAY && 
                      (current_char == ']' || current_char == ',')) {
                parser.log("While parsing a string missing the left delimiter in array context, we found a ] or ,, stopping here");
                break;
            }
        }
        
        string_acc += current_char;
        parser.index += 1;
        current_char = parser.get_char_at();
        
        if (parser.stream_stable && !current_char && !string_acc.empty() && string_acc.back() == '\\') {
            string_acc.pop_back();
        }
        
        if (current_char && !string_acc.empty() && string_acc.back() == '\\') {
            parser.log("Found a stray escape sequence, normalizing it");
            if (current_char == rstring_delimiter || current_char == 't' || 
                current_char == 'n' || current_char == 'r' || current_char == 'b' || 
                current_char == '\\') {
                string_acc.pop_back();
                char escape_char = current_char;
                switch (current_char) {
                    case 't': escape_char = '\t'; break;
                    case 'n': escape_char = '\n'; break;
                    case 'r': escape_char = '\r'; break;
                    case 'b': escape_char = '\b'; break;
                    default: break;
                }
                string_acc += escape_char;
                parser.index += 1;
                current_char = parser.get_char_at();
                
                while (current_char && !string_acc.empty() && string_acc.back() == '\\' && 
                       (current_char == rstring_delimiter || current_char == '\\')) {
                    string_acc.pop_back();
                    string_acc += current_char;
                    parser.index += 1;
                    current_char = parser.get_char_at();
                }
                continue;
            }
        }
        
        if (current_char == rstring_delimiter && !string_acc.empty() && string_acc.back() != '\\') {
            if (doubled_quotes && parser.get_char_at(1) == rstring_delimiter) {
                parser.log("While parsing a string, we found a doubled quote, ignoring it");
                parser.index += 1;
            } else {
                // Check if eventually there is a rstring delimiter, otherwise we bail
                size_t i = 1;
                char next_c = parser.get_char_at(i);
                bool check_comma_in_object_value = true;
                while (next_c && next_c != rstring_delimiter && next_c != lstring_delimiter) {
                    if (check_comma_in_object_value && std::isalpha(next_c)) {
                        check_comma_in_object_value = false;
                    }
                    if ((std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::OBJECT_KEY) != parser.context.getContext().end() && (next_c == ':' || next_c == '}')) ||
                        (std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::OBJECT_VALUE) != parser.context.getContext().end() && next_c == '}') ||
                        (std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::ARRAY) != parser.context.getContext().end() && (next_c == ']' || next_c == ',')) ||
                        (check_comma_in_object_value && 
                         parser.context.getCurrent() == ContextValues::OBJECT_VALUE && 
                         next_c == ',')) {
                        break;
                    }
                    i += 1;
                    next_c = parser.get_char_at(i);
                }
            }
        }
    }
    
    if (current_char && missing_quotes && parser.context.getCurrent() == ContextValues::OBJECT_KEY && std::isspace(current_char)) {
        parser.log("While parsing a string, handling an extreme corner case in which the LLM added a comment instead of valid string, invalidate the string and return an empty value");
        parser.skip_whitespaces();
        if (parser.get_char_at() != ':' && parser.get_char_at() != ',') {
            return "";
        }
    }

    if (current_char != rstring_delimiter) {
        if (!parser.stream_stable) {
            parser.log("While parsing a string, we missed the closing quote, ignoring");
            while (!string_acc.empty() && std::isspace(string_acc.back())) {
                string_acc.pop_back();
            }
        }
    } else {
        parser.index += 1;
    }

    if (!parser.stream_stable && (missing_quotes || (!string_acc.empty() && string_acc.back() == '\n'))) {
        while (!string_acc.empty() && std::isspace(string_acc.back())) {
            string_acc.pop_back();
        }
    }

    return string_acc;
}