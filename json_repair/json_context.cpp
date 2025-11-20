#include "json_context.hpp"

JsonContext::JsonContext() : empty(true) {}

void JsonContext::set(ContextValues value) {
    context.push_back(value);
    current = value;
    empty = false;
}

void JsonContext::reset() {
    if (!context.empty()) {
        context.pop_back();
        if (!context.empty()) {
            current = context.back();
        } else {
            current = std::nullopt;
            empty = true;
        }
    } else {
        current = std::nullopt;
        empty = true;
    }
}

std::optional<ContextValues> JsonContext::getCurrent() const {
    return current;
}

bool JsonContext::isEmpty() const {
    return empty;
}

const std::vector<ContextValues>& JsonContext::getContext() const {
    return context;
}