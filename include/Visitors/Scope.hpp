#ifndef CYGNI_VISITORS_SCOPE_HPP
#define CYGNI_VISITORS_SCOPE_HPP

#include <unordered_map>
#include "Visitors/ScopeException.hpp"

namespace Cygni {
namespace Visitors {

template <typename TValue>
class Scope
{
private:
	Scope<TValue>* parent;
	std::unordered_map<std::u32string, TValue> values;

public:
	Scope() : parent(nullptr) {};
	Scope(Scope<TValue>* parent) : parent(parent) {}

	void Declare(const std::u32string& name, const TValue& value);

	bool Exists(const std::u32string& name) const;

    TValue& Get(const std::u32string& name);

    const TValue& Get(const std::u32string& name) const;
};


template <typename TValue>
void Scope<TValue>::Declare(const std::u32string& name, const TValue& value)
{
    values[name] = value;
}

template <typename TValue>
bool Scope<TValue>::Exists(const std::u32string& name) const
{
    if (values.find(name) != values.end())
    {
        return true;
    }
    else
    {
        if (parent != nullptr)
        {
            return parent->Exists(name);
        }
        else
        {
            return false;
        }
    }
}

template <typename TValue>
TValue& Scope<TValue>::Get(const std::u32string& name)
{
    if (values.find(name) != values.end())
    {
        return values[name];
    }
    else
    {
        if (parent != nullptr)
        {
            return parent->Get(name);
        }
        else
        {
            throw ScopeException(__FILE__, __LINE__, "Undefined symbol.", nullptr, name);
        }
    }
}

template <typename TValue>
const TValue& Scope<TValue>::Get(const std::u32string& name) const
{
    if (values.find(name) != values.end())
    {
        return values[name];
    }
    else
    {
        if (parent != nullptr)
        {
            return parent->Get(name);
        }
        else
        {
            throw ScopeException(__FILE__, __LINE__, "Undefined symbol.", nullptr, name);
        }
    }
}

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_SCOPE_HPP */