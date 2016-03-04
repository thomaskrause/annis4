#pragma once

#include "abstractedgeoperator.h"

namespace annis
{

class Dominance : public AbstractEdgeOperator
{
public:
  Dominance(const DB& db, std::string ns, std::string name,
                   unsigned int minDistance = 1, unsigned int maxDistance = 1);

  Dominance(const DB& db, std::string ns, std::string name,
                   const Annotation& edgeAnno);

  virtual std::string operatorString() override
  {
    return ">";
  }

  
  virtual ~Dominance();
private:

};
} // end namespace annis
