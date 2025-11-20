#include "string_file_wrapper.hpp"
#include <algorithm>

StringFileWrapper::StringFileWrapper(std::fstream& file_descriptor, size_t chunk_length) 
    : fd(file_descriptor), length(0) {
    if (!chunk_length || chunk_length < 2) {
        chunk_length = 1000000; // 1MB default
    }
    this->buffer_length = chunk_length;
}

std::string StringFileWrapper::get_buffer(size_t index) {
    auto it = buffers.find(index);
    if (it == buffers.end()) {
        fd.seekg(index * buffer_length);
        std::string buffer;
        buffer.resize(buffer_length);
        fd.read(&buffer[0], buffer_length);
        size_t bytes_read = fd.gcount();
        buffer.resize(bytes_read);
        buffers[index] = buffer;
        
        size_t max_buffers = std::max(static_cast<size_t>(2), static_cast<size_t>(2000000 / buffer_length));
        if (buffers.size() > max_buffers) {
            auto oldest_it = buffers.begin();
            if (oldest_it->first != index) {
                buffers.erase(oldest_it);
            }
        }
    }
    return buffers[index];
}

std::string StringFileWrapper::operator[](size_t index)  {
    size_t buffer_index = index / buffer_length;
    std::string buffer = get_buffer(buffer_index);
    return std::string(1, buffer[index % buffer_length]);
}

std::string StringFileWrapper::get_range(size_t start, size_t stop)  {
    size_t buffer_index = start / buffer_length;
    size_t buffer_end = stop / buffer_length;
    
    if (buffer_index == buffer_end) {
        std::string buffer = get_buffer(buffer_index);
        return buffer.substr(start % buffer_length, stop - start);
    } else {
        std::string start_slice = get_buffer(buffer_index).substr(start % buffer_length);
        std::string end_slice = get_buffer(buffer_end).substr(0, stop % buffer_length);
        std::string result = start_slice;
        
        for (size_t i = buffer_index + 1; i < buffer_end; ++i) {
            result += get_buffer(i);
        }
        
        result += end_slice;
        return result;
    }
}

size_t StringFileWrapper::size() const {
    if (length == 0) {
        std::streampos current_position = fd.tellg();
        fd.seekg(0, std::ios::end);
        length = fd.tellg();
        fd.seekg(current_position);
    }
    return length;
}

void StringFileWrapper::write_at(size_t index, const std::string& value) {
    std::streampos current_position = fd.tellg();
    fd.seekp(index);
    fd.write(value.c_str(), value.length());
    fd.seekg(current_position);
}