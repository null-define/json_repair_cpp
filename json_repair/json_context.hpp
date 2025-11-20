#ifndef JSON_CONTEXT_HPP
#define JSON_CONTEXT_HPP

#include <vector>
#include <optional>

enum class ContextValues {
    OBJECT_KEY,
    OBJECT_VALUE,
    ARRAY
};

class JsonContext {
private:
    std::vector<ContextValues> context;
    std::optional<ContextValues> current;
    bool empty;

public:
    JsonContext();

    void set(ContextValues value);
    void reset();

    std::optional<ContextValues> getCurrent() const;
    bool isEmpty() const;
    const std::vector<ContextValues>& getContext() const;
};

#endif