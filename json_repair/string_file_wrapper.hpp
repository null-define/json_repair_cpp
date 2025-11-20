#ifndef STRING_FILE_WRAPPER_HPP
#define STRING_FILE_WRAPPER_HPP

#include <fstream>
#include <string>
#include <unordered_map>

class StringFileWrapper {
private:
    std::fstream& fd;
    mutable size_t length;
    std::unordered_map< size_t, std::string > buffers;
    size_t buffer_length;

public:
    StringFileWrapper(std::fstream& file_descriptor, size_t chunk_length);

    std::string get_buffer(size_t index);
    std::string operator[](size_t index);
    std::string get_range(size_t start, size_t stop);
    size_t size() const;
    void write_at(size_t index, const std::string& value);
};

#endif