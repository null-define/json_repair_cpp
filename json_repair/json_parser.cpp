#include "json_parser.hpp"

#include "constants.hpp"
#include "json_context.hpp"
#include "object_comparer.hpp"
#include "parse_array.hpp"
#include "parse_comment.hpp"
#include "parse_number.hpp"
#include "parse_object.hpp"
#include "parse_string.hpp"
#include "string_file_wrapper.hpp"

#include <cctype>
#include <functional>
#include <variant>

JSONParser::JSONParser(const std::string& json_str,
                       bool logging_param,
                       size_t json_fd_chunk_length,
                       bool stream_stable_param)
    : json_str_variant(json_str),
      index(0),
      logging(logging_param),
      stream_stable(stream_stable_param) {
    if (logging) {
        logger.clear();
        log = [this](const std::string& text) { this->_log(text); };
    } else {
        log = [](const std::string&) {};
    }
}

JSONParser::JSONParser(StringFileWrapper& json_fd_wrapper,
                       bool logging_param,
                       size_t json_fd_chunk_length,
                       bool stream_stable_param)
    : json_str_variant(json_fd_wrapper),
      index(0),
      logging(logging_param),
      stream_stable(stream_stable_param) {
    if (logging) {
        logger.clear();
        log = [this](const std::string& text) { this->_log(text); };
    } else {
        log = [](const std::string&) {};
    }
}

JSONReturnType JSONParser::parse() {
    auto result = parse_json();
    if (index < get_length()) {
        log("The parser returned early, checking if there's more json elements");
        std::vector< JSONReturnType > json_array = {result};
        while (index < get_length()) {
            context.reset();
            auto j = parse_json();
            if (j != std::string("")) {
                if (ObjectComparer::is_same_object(json_array.back(), j)) {
                    json_array.pop_back();
                }
                json_array.push_back(j);
            } else {
                index += 1;
            }
        }
        if (json_array.size() == 1) {
            log("There were no more elements, returning the element without the array");
            return json_array[0];
        }
        // Convert vector to JSONReturnType
        std::vector< JSONReturnType > temp_vec;
        for (const auto& item : json_array) {
            temp_vec.push_back(item);
        }
        return temp_vec;
    }

    return result;
}

std::pair< JSONReturnType, std::vector< std::map< std::string, std::string > > >
JSONParser::parse_with_logs() {
    auto result = parse_json();
    if (index < get_length()) {
        log("The parser returned early, checking if there's more json elements");
        std::vector< JSONReturnType > json_array = {result};
        while (index < get_length()) {
            context.reset();
            auto j = parse_json();
            if (j != "") {
                if (ObjectComparer::is_same_object(json_array.back(), j)) {
                    json_array.pop_back();
                }
                json_array.push_back(j);
            } else {
                index += 1;
            }
        }
        if (json_array.size() == 1) {
            log("There were no more elements, returning the element without the array");
            result = json_array[0];
        }
    }
    return std::make_pair(result, logger);
}

JSONReturnType JSONParser::parse_json() {
    while (true) {
        char current_char = get_char_at();
        auto const curr_string = std::string{current_char};
        if (current_char == '\0') {
            return std::string("");
        } else if (current_char == '{') {
            index += 1;
            return parse_object();
        } else if (current_char == '[') {
            index += 1;
            return parse_array();
        } else if (!context.isEmpty() &&
                   (std::find(STRING_DELIMITERS.begin(), STRING_DELIMITERS.end(), curr_string) !=
                        STRING_DELIMITERS.end() ||
                    std::isalpha(current_char))) {
            return parse_string();
        } else if (!context.isEmpty() &&
                   (std::isdigit(current_char) || current_char == '-' || current_char == '.')) {
            return parse_number();
        } else if (current_char == '#' || current_char == '/') {
            return parse_comment();
        } else {
            index += 1;
        }
    }
}

char JSONParser::get_char_at(int count) {
    size_t pos = index + count;
    if (pos >= get_length()) {
        return '\0';
    }
    return get_char_at_impl(pos);
}

void JSONParser::skip_whitespaces() {
    try {
        char current_char = get_char_at_impl(index);
        while (std::isspace(current_char)) {
            index += 1;
            current_char = get_char_at_impl(index);
        }
    } catch (...) {
        // Handle index out of bounds
    }
}

size_t JSONParser::scroll_whitespaces(size_t idx) {
    try {
        char current_char = get_char_at_impl(index + idx);
        while (std::isspace(current_char)) {
            idx += 1;
            current_char = get_char_at_impl(index + idx);
        }
    } catch (...) {
        // Handle index out of bounds
    }
    return idx;
}

size_t JSONParser::skip_to_character(char character, size_t idx) {
    std::vector< char > chars = {character};
    return skip_to_character(chars, idx);
}

size_t JSONParser::skip_to_character(const std::vector< char >& characters, size_t idx) {
    std::set< char > targets(characters.begin(), characters.end());
    size_t i = index + idx;
    size_t n = get_length();
    size_t backslashes = 0;

    while (i < n) {
        char ch = get_char_at_impl(i);

        if (ch == '\\') {
            backslashes += 1;
            i += 1;
            continue;
        }

        if (targets.find(ch) != targets.end() && (backslashes % 2 == 0)) {
            return i - index;
        }

        backslashes = 0;
        i += 1;
    }

    return n - index;
}

void JSONParser::_log(const std::string& text) {
    size_t window = 10;
    size_t start = (index > window) ? index - window : 0;
    size_t end = std::min(index + window, get_length());

    std::string context_str;
    for (size_t i = start; i < end; ++i) {
        context_str += get_char_at_impl(i);
    }

    std::map< std::string, std::string > log_entry;
    log_entry["text"] = text;
    log_entry["context"] = context_str;
    logger.push_back(log_entry);
}

char JSONParser::get_char_at_impl(size_t pos) {
    if (std::holds_alternative< std::string >(json_str_variant)) {
        const std::string& str = std::get< std::string >(json_str_variant);
        if (pos < str.length()) {
            return str[pos];
        }
        return '\0';
    } else {
        StringFileWrapper& wrapper = std::get< StringFileWrapper >(json_str_variant);
        if (pos < wrapper.size()) {
            std::string char_str = wrapper[pos];
            return char_str.empty() ? '\0' : char_str[0];
        }
        return '\0';
    }
}

size_t JSONParser::get_length() const {
    if (std::holds_alternative< std::string >(json_str_variant)) {
        return std::get< std::string >(json_str_variant).length();
    } else {
        return std::get< StringFileWrapper >(json_str_variant).size();
    }
}

std::vector< JSONReturnType > JSONParser::parse_array() {
    return ::parse_array(*this);
}

JSONReturnType JSONParser::parse_comment() {
    return ::parse_comment(*this);
}

JSONReturnType JSONParser::parse_number() {
    return ::parse_number(*this);
}

JSONReturnType JSONParser::parse_object() {
    return ::parse_object(*this);
}

JSONReturnType::StringType JSONParser::parse_string() {
    return ::parse_string(*this);
}