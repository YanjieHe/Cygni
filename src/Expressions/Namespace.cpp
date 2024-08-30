#include "Expressions/Namespace.hpp"
#include <Utility/UTF32Functions.hpp>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace Cygni
{
namespace Expressions
{

std::vector<std::u32string> Namespace::GetFullQualifiedName()
{
    std::vector<std::u32string> path;
    const Namespace *current = this;
    while (current != nullptr)
    {
        path.push_back(current->Name());
        current = current->Parent();
    }
    std::reverse(path.begin(), path.end());

    return path;
}

void NamespaceFactory::Insert(Namespace *predecessor, const std::vector<std::u32string> &path)
{

    for (const std::u32string &name : path)
    {
        if (predecessor->Children().ContainsKey(name))
        {
            predecessor = predecessor->Children().GetItemByKey(name);
        }
        else
        {
            Namespace *ns = Create(predecessor, name);
            predecessor->Children().AddItem(name, ns);
            predecessor = ns;
        }
    }
}

Namespace *NamespaceFactory::Search(Namespace *predecessor, const std::vector<std::u32string> &path)
{
    for (const std::u32string &name : path)
    {
        if (predecessor->Children().ContainsKey(name))
        {
            predecessor = predecessor->Children().GetItemByKey(name);
        }
        else
        {

            return nullptr;
        }
    }

    return predecessor;
}

VariableDeclarationExpression *NamespaceFactory::SearchGlobalVariable(Namespace *current,
                                                                      const std::vector<std::u32string> &path)
{
    VariableDeclarationExpression *varDecl = SearchGlobalVariableRecursively(current, path, 0);

    if (varDecl == nullptr)
    {
        return SearchGlobalVariableRecursively(GetRoot(), path, 0);
    }
    else
    {
        return varDecl;
    }
}

LambdaExpression *NamespaceFactory::SearchFunction(Namespace *current, const std::vector<std::u32string> &path)
{
    LambdaExpression *funcDecl = SearchFunctionRecursively(current, path, 0);

    if (funcDecl == nullptr)
    {
        return SearchFunctionRecursively(GetRoot(), path, 0);
    }
    else
    {
        return funcDecl;
    }
}

StructureExpression *NamespaceFactory::SearchStructure(Namespace *current, const std::vector<std::u32string> &path)
{
    StructureExpression *structureDefinition = SearchStructureRecursively(current, path, 0);

    if (structureDefinition == nullptr)
    {
        return SearchStructureRecursively(GetRoot(), path, 0);
    }
    else
    {
        return structureDefinition;
    }
}

VariableDeclarationExpression *NamespaceFactory::SearchGlobalVariableRecursively(
    Namespace *current, const std::vector<std::u32string> &path, int i)
{
    int n = static_cast<int>(path.size());
    const std::u32string &name = path.at(static_cast<size_t>(i));
    if (i == n)
    {
        /* Searched to the end. */
        return nullptr;
    }
    else if (i == n - 1)
    {
        if (current->GlobalVariables().ContainsKey(name))
        {

            return current->GlobalVariables().GetItemByKey(name);
        }
        else
        {

            return nullptr;
        }
    }
    else
    {
        if (current->Children().ContainsKey(name))
        {

            return SearchGlobalVariableRecursively(current->Children().GetItemByKey(name), path, i + 1);
        }
        else
        {

            return nullptr;
        }
    }
}

LambdaExpression *NamespaceFactory::SearchFunctionRecursively(Namespace *current,
                                                              const std::vector<std::u32string> &path, int i)
{
    int n = static_cast<int>(path.size());
    const std::u32string &name = path.at(static_cast<size_t>(i));
    if (i == n)
    {
        /* Searched to the end. */
        return nullptr;
    }
    else if (i == n - 1)
    {
        if (current->Functions().ContainsKey(name))
        {

            return current->Functions().GetItemByKey(name);
        }
        else
        {

            return nullptr;
        }
    }
    else
    {
        if (current->Children().ContainsKey(name))
        {

            return SearchFunctionRecursively(current->Children().GetItemByKey(name), path, i + 1);
        }
        else
        {

            return nullptr;
        }
    }
}

StructureExpression *NamespaceFactory::SearchStructureRecursively(Namespace *current,
                                                                  const std::vector<std::u32string> &path, int i)
{
    int n = static_cast<int>(path.size());
    const std::u32string &name = path.at(static_cast<size_t>(i));
    if (i == n)
    {
        /* Searched to the end. */
        return nullptr;
    }
    else if (i == n - 1)
    {
        if (current->Structures().ContainsKey(name))
        {

            return current->Structures().GetItemByKey(name);
        }
        else
        {

            return nullptr;
        }
    }
    else
    {
        if (current->Children().ContainsKey(name))
        {

            return SearchStructureRecursively(current->Children().GetItemByKey(name), path, i + 1);
        }
        else
        {

            return nullptr;
        }
    }
}

}; /* namespace Expressions */
}; /* namespace Cygni */