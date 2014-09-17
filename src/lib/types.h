#ifndef TUPEL_H
#define TUPEL_H

#include <cstdint>
#include <string>

namespace annis
{
  const std::string annis_ns = "annis4_internal";

  struct Edge
  {
    std::uint32_t source;
    std::uint32_t target;
  };

  enum class ComponentType {COVERAGE, DOMINANCE, POINTING, ORDERING,
                            ComponentType_MAX};
  static std::string ComponentTypeToString(const ComponentType& type)
  {
    switch(type)
    {
    case ComponentType::COVERAGE:
      return "COVERAGE";
      break;
    case ComponentType::DOMINANCE:
      return "DOMINANCE";
      break;
    case ComponentType::POINTING:
      return "POINTING";
      break;
    case ComponentType::ORDERING:
      return "ORDERING";
      break;
    default:
      return "UNKNOWN";
    }
  }

  static ComponentType ComponentTypeFromString(const std::string& typeAsString)
  {
    for(unsigned int t = (unsigned int)ComponentType::COVERAGE; t < (unsigned int) ComponentType::ComponentType_MAX; t++)
    {
      if(ComponentTypeToString((ComponentType) t) == typeAsString)
      {
        return (ComponentType) t;
      }
    }
    return ComponentType::ComponentType_MAX;
  }

  const size_t MAX_COMPONENT_NAME_SIZE = 255;

  struct Component
  {
    ComponentType type;
    char ns[MAX_COMPONENT_NAME_SIZE];
    char name[MAX_COMPONENT_NAME_SIZE];
  };

  struct Annotation
  {
    std::uint32_t name;
    std::uint32_t ns;
    std::uint32_t val;
  };

  typedef std::pair<std::uint32_t, Annotation> Match;

  static Edge constructEdge(std::uint32_t source, std::uint32_t target)
  {
    Edge result;
    result.source = source;
    result.target = target;
    return result;
  }
}

#endif // TUPEL_H
