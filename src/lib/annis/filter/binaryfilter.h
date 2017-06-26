/*
   Copyright 2017 Thomas Krause <thomaskrause@posteo.de>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <annis/iterators.h>  // for Iterator
#include <stddef.h>           // for size_t
#include <memory>             // for shared_ptr
#include <vector>             // for vector

namespace annis
{

class Operator;
struct Match;

class BinaryFilter : public Iterator
{
public:

  BinaryFilter(std::shared_ptr<Operator> op, std::shared_ptr<Iterator> inner,
    size_t lhsIdx, size_t rhsIdx);

  virtual bool next(std::vector<Match>& tuple) override;
  virtual void reset() override;

  virtual ~BinaryFilter();

private:
  std::shared_ptr<Operator> op;
  std::shared_ptr<Iterator> inner;
  size_t lhsIdx; 
  size_t rhsIdx;
};

} // end namespace annis
