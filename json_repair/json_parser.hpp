#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include "json_context.hpp"
#include "object_comparer.hpp"
#include "string_file_wrapper.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

struct JSONReturnType {
    using MapType = std::map< std::string, JSONReturnType >;
    using VectorType = std::vector< JSONReturnType >;
    using StringType = std::string;
    using DoubleType = double;
    using IntType = int;
    using BoolType = bool;
    using NullType = std::nullptr_t;

    using Data =
        std::variant< MapType, VectorType, StringType, DoubleType, IntType, BoolType, NullType >;

protected:
    Data data;

public:
    JSONReturnType() : data(NullType()) {}
    JSONReturnType(Data data) : data(std::move(data)) {}
    JSONReturnType(const JSONReturnType& other) = default;
    JSONReturnType(JSONReturnType&& other) = default;

    JSONReturnType(const VectorType& vec) : data(vec) {}

    JSONReturnType(VectorType&& vec) : data(std::move(vec)) {}

    JSONReturnType(const StringType& in_data) : data(in_data) {}

    JSONReturnType(StringType&& in_data) : data(std::move(in_data)) {}

    JSONReturnType(const DoubleType& in_data) : data(in_data) {}

    JSONReturnType(const MapType& in_data) : data(in_data) {}

    JSONReturnType(MapType&& in_data) : data(std::move(in_data)) {}

    // Assignment operators with type-specific behavior
    JSONReturnType& operator=(const JSONReturnType& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }

    JSONReturnType& operator=(JSONReturnType&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }

    // Assignment operators for each variant type
    JSONReturnType& operator=(const MapType& map) {
        data = map;
        return *this;
    }

    JSONReturnType& operator=(MapType&& map) {
        data = std::move(map);
        return *this;
    }

    JSONReturnType& operator=(const VectorType& vec) {
        data = vec;
        return *this;
    }

    JSONReturnType& operator=(VectorType&& vec) {
        data = std::move(vec);
        return *this;
    }

    JSONReturnType& operator=(const StringType& str) {
        data = str;
        return *this;
    }

    JSONReturnType& operator=(StringType&& str) {
        data = std::move(str);
        return *this;
    }

    JSONReturnType& operator=(DoubleType d) {
        data = d;
        return *this;
    }

    JSONReturnType& operator=(IntType i) {
        data = static_cast< DoubleType >(i); // Convert int to double to avoid ambiguity
        return *this;
    }

    JSONReturnType& operator=(BoolType b) {
        data = b;
        return *this;
    }

    JSONReturnType& operator=(NullType n) {
        data = n;
        return *this;
    }

    std::string dump(int indent = -1) const {
        if (std::holds_alternative< StringType >(data)) {
            // Properly escape string for JSON
            std::string str = std::get< StringType >(data);
            return "\"" + str + "\"";
        } else if (std::holds_alternative< DoubleType >(data)) {
            return std::to_string(std::get< DoubleType >(data));
        } else if (std::holds_alternative< IntType >(data)) {
            return std::to_string(std::get< IntType >(data));
        } else if (std::holds_alternative< BoolType >(data)) {
            return std::get< BoolType >(data) ? "true" : "false";
        } else if (std::holds_alternative< NullType >(data)) {
            return "null";
        } else if (std::holds_alternative< MapType >(data)) {
            std::string result = "{";
            const auto& map = std::get< MapType >(data);
            bool first = true;
            for (const auto& [key, value] : map) {
                if (!first)
                    result += ",";
                if (indent >= 0)
                    result += "\n" + std::string(indent + 2, ' ');
                result += "\"" + key + "\":" + (indent >= 0 ? " " : "") +
                          value.dump(indent >= 0 ? indent + 2 : -1);
                first = false;
            }
            if (indent >= 0 && !map.empty())
                result += "\n" + std::string(indent, ' ');
            result += "}";
            return result;
        } else if (std::holds_alternative< VectorType >(data)) {
            std::string result = "[";
            const auto& vec = std::get< VectorType >(data);
            bool first = true;
            for (const auto& item : vec) {
                if (!first)
                    result += ",";
                if (indent >= 0)
                    result += "\n" + std::string(indent + 2, ' ');
                result += item.dump(indent >= 0 ? indent + 2 : -1);
                first = false;
            }
            if (indent >= 0 && !vec.empty())
                result += "\n" + std::string(indent, ' ');
            result += "]";
            return result;
        } else {
            throw std::runtime_error("Unknown JSON type");
        }
    }

    bool empty() const { return std::holds_alternative< NullType >(data); }

    ~JSONReturnType() = default;

    template < typename T > bool is() const { return std::holds_alternative< T >(data); }

    template < typename T > T& get() { return std::get< T >(data); }

    template < typename T > const T& get() const { return std::get< T >(data); }

    // Comparison operators
    bool operator==(const JSONReturnType& other) const { return data == other.data; }

    bool operator!=(const JSONReturnType& other) const { return data != other.data; }

    // Comparison operators for each variant type
    bool operator!=(const MapType& map) const {
        return !std::holds_alternative< MapType >(data) || std::get< MapType >(data) != map;
    }

    bool operator!=(const VectorType& vec) const {
        return !std::holds_alternative< VectorType >(data) || std::get< VectorType >(data) != vec;
    }

    bool operator!=(const StringType& str) const {
        return !std::holds_alternative< StringType >(data) || std::get< StringType >(data) != str;
    }

    bool operator!=(DoubleType d) const {
        return !std::holds_alternative< DoubleType >(data) || std::get< DoubleType >(data) != d;
    }

    bool operator!=(IntType i) const {
        return !std::holds_alternative< DoubleType >(data) ||
               std::get< DoubleType >(data) != static_cast< DoubleType >(i);
    }

    bool operator!=(BoolType b) const {
        return !std::holds_alternative< BoolType >(data) || std::get< BoolType >(data) != b;
    }

    bool operator!=(NullType) const { return !std::holds_alternative< NullType >(data); }

    // Comparison operators for each variant type
    bool operator==(const MapType& map) const {
        return std::holds_alternative< MapType >(data) && std::get< MapType >(data) == map;
    }

    bool operator==(const VectorType& vec) const {
        return std::holds_alternative< VectorType >(data) && std::get< VectorType >(data) == vec;
    }

    bool operator==(const StringType& str) const {
        return std::holds_alternative< StringType >(data) && std::get< StringType >(data) == str;
    }

    bool operator==(DoubleType d) const {
        return std::holds_alternative< DoubleType >(data) && std::get< DoubleType >(data) == d;
    }

    bool operator==(IntType i) const {
        return std::holds_alternative< DoubleType >(data) &&
               std::get< DoubleType >(data) == static_cast< DoubleType >(i);
    }

    bool operator==(BoolType b) const {
        return std::holds_alternative< BoolType >(data) && std::get< BoolType >(data) == b;
    }

    bool operator==(NullType) const { return std::holds_alternative< NullType >(data); }

    // Subscript operators for accessing map and vector elements
    JSONReturnType& operator[](const std::string& key) {
        if (!std::holds_alternative< MapType >(data)) {
            data = MapType{};
        }
        return std::get< MapType >(data)[key];
    }

    const JSONReturnType& operator[](const std::string& key) const {
        return std::get< MapType >(data).at(key);
    }

    JSONReturnType& operator[](size_t index) {
        if (!std::holds_alternative< VectorType >(data)) {
            data = VectorType{};
        }
        if (index >= std::get< VectorType >(data).size()) {
            std::get< VectorType >(data).resize(index + 1);
        }
        return std::get< VectorType >(data)[index];
    }

    const JSONReturnType& operator[](size_t index) const {
        return std::get< VectorType >(data).at(index);
    }
};

class JSONParser {
public:
    // Split the parse methods into separate files because this one was like 3000 lines
    JSONReturnType parse_comment();
    JSONReturnType parse_object();
    JSONReturnType::VectorType parse_array();
    JSONReturnType parse_number();
    JSONReturnType::StringType parse_string();

    JSONParser(const std::string& json_str,
               bool logging = false,
               size_t json_fd_chunk_length = 0,
               bool stream_stable = false);

    JSONParser(StringFileWrapper& json_fd_wrapper,
               bool logging = false,
               size_t json_fd_chunk_length = 0,
               bool stream_stable = false);

    JSONReturnType parse();
    std::pair< JSONReturnType, std::vector< std::map< std::string, std::string > > >
    parse_with_logs();

    JSONReturnType parse_json();

    char get_char_at(int count = 0);

    void skip_whitespaces();
    size_t scroll_whitespaces(size_t idx = 0);
    size_t skip_to_character(char character, size_t idx = 0);
    size_t skip_to_character(const std::vector< char >& characters, size_t idx = 0);

    size_t index;
    JsonContext context;
    std::variant< std::string, StringFileWrapper > json_str_variant;
    bool logging;
    std::vector< std::map< std::string, std::string > > logger;
    bool stream_stable;
    std::function< void(const std::string&) > log;

private:
    void _log(const std::string& text);

    // Helper to get current character based on the variant type
    char get_char_at_impl(size_t pos);
    size_t get_length() const;
};

#endif