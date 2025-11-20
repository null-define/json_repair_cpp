#include "json_repair/json_parser.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>

std::string test_basic_parsing(std::string input) {
    // Test simple string
    {
        JSONParser parser(input);
        auto result = parser.parse().dump(4);
        return result;
    }
}

int main(int argc, char const *argv[])
{
    if(argc < 2)  {
        std::cout << "Usage: " << argv[0] << " <json_path>" << std::endl;
        return 1;
    }
    auto file_path = std::string(argv[1]);
    std::ifstream file(file_path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto result = test_basic_parsing(buffer.str());
    std::cout << result << std::endl;
    return 0;
}
