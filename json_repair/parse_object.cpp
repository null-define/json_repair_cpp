#include "parse_object.hpp"
#include "constants.hpp"
#include "object_comparer.hpp"
#include <cctype>

JSONReturnType parse_object(JSONParser& parser) {
    std::map<std::string, JSONReturnType> obj;
    size_t start_index = parser.index;
    
    while (parser.get_char_at() != '}' && parser.get_char_at() != '\0') {
        parser.skip_whitespaces();

        if (parser.get_char_at() == ':') {
            parser.log("While parsing an object we found a : before a key, ignoring");
            parser.index += 1;
        }

        parser.context.set(ContextValues::OBJECT_KEY);

        size_t rollback_index = parser.index;

        std::string key = "";
        while (parser.get_char_at() != '\0') {
            rollback_index = parser.index;
            if (parser.get_char_at() == '[' && key.empty()) {
                // Complex array merging logic skipped for brevity
            }
            
            key = parser.parse_string();
            if (key.empty()) {
                parser.skip_whitespaces();
            }
            if (!key.empty() || (key.empty() && (parser.get_char_at() == ':' || parser.get_char_at() == '}'))) {
                break;
            }
        }
        
        auto context_it = std::find(parser.context.getContext().begin(), parser.context.getContext().end(), ContextValues::ARRAY);
        if (context_it != parser.context.getContext().end() && obj.find(key) != obj.end()) {
            parser.log("While parsing an object we found a duplicate key, closing the object here and rolling back the index");
            parser.index = rollback_index - 1;
            break;
        }

        parser.skip_whitespaces();

        if (parser.get_char_at() == '}' || parser.get_char_at() == '\0') {
            continue;
        }

        parser.skip_whitespaces();

        if (parser.get_char_at() != ':') {
            parser.log("While parsing an object we missed a : after a key");
        }

        parser.index += 1;
        parser.context.reset();
        parser.context.set(ContextValues::OBJECT_VALUE);
        parser.skip_whitespaces();
        
        JSONReturnType value = std::string("");
        if (parser.get_char_at() == ',' || parser.get_char_at() == '}') {
            parser.log("While parsing an object value we found a stray , ignoring it");
        } else {
            value = parser.parse_json();
        }

        parser.context.reset();
        obj[key] = value;

        if (parser.get_char_at() == ',' || parser.get_char_at() == '\'' || parser.get_char_at() == '"') {
            parser.index += 1;
        }

        parser.skip_whitespaces();
    }

    parser.index += 1;

    if (obj.empty() && parser.index - start_index > 2) {
        parser.log("Parsed object is empty, we will try to parse this as an array instead");
        parser.index = start_index;
        return parser.parse_array();
    }

    if (!parser.context.isEmpty()) {
        return obj;
    }

    parser.skip_whitespaces();
    if (parser.get_char_at() != ',') {
        return obj;
    }
    parser.index += 1;
    parser.skip_whitespaces();
    if (std::find(STRING_DELIMITERS.begin(), STRING_DELIMITERS.end(), std::string(1, parser.get_char_at())) == STRING_DELIMITERS.end()) {
        return obj;
    }
    parser.log("Found a comma and string delimiter after object closing brace, checking for additional key-value pairs");
    
    auto additional_obj = parse_object(parser).get<JSONReturnType::MapType>();
    for (const auto& [k, v] : additional_obj) {
        obj[k] = v;
    }

    return obj;
}