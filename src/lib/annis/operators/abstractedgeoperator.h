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

#include <boost/optional/optional.hpp>
#include <annis/operators/operator.h>  // for Operator
#include <stdint.h>                    // for int64_t
#include <functional>                  // for function
#include <list>                        // for list
#include <memory>                      // for shared_ptr, unique_ptr
#include <string>                      // for string
#include <vector>                      // for vector
#include <annis/types.h>               // for ComponentType, Match (ptr only)
#include <annis/db.h>

namespace annis { class AnnoIt; }
namespace annis { class NodeByEdgeAnnoSearch; }
namespace annis { class ReadableGraphStorage; }
namespace annis { class StringStorage; }
namespace annis { class EstimatedSearch; }


namespace annis
{

class AbstractEdgeOperator : public Operator
{

public:
  AbstractEdgeOperator(ComponentType componentType, std::string ns, std::string name,
                       DB::GetGSFuncT getGraphStorageFunc,
                       const DB& db,
      unsigned int minDistance = 1, unsigned int maxDistance = 1);

  AbstractEdgeOperator(ComponentType componentType, std::string name,
                       DB::GetAllGSFuncT getAllGraphStorageFunc,
                       const DB& db,
      unsigned int minDistance = 1, unsigned int maxDistance = 1);


  AbstractEdgeOperator(
      ComponentType componentType, std::string ns, std::string name,
      DB::GetGSFuncT getGraphStorageFunc,
      const DB& db,
      const Annotation& edgeAnno = Init::initAnnotation());

  AbstractEdgeOperator(
      ComponentType componentType, std::string name,
      DB::GetAllGSFuncT getAllGraphStorageFunc,
      const DB& db,
      const Annotation& edgeAnno = Init::initAnnotation());

  virtual std::unique_ptr<AnnoIt> retrieveMatches(const Match& lhs) override;
  virtual bool filter(const Match& lhs, const Match& rhs) override;

  virtual bool valid() const override {return !gs.empty();}
  
  virtual std::string operatorString() = 0;

  virtual std::string description() override;

  virtual double selectivity() override;

  virtual double edgeAnnoSelectivity() override;

  virtual std::int64_t guessMaxCountEdgeAnnos();
  
  virtual std::shared_ptr<EstimatedSearch> createAnnoSearch(std::function<std::list<Annotation> (nodeid_t)> nodeAnnoMatchGenerator,
      bool maximalOneNodeAnno, bool returnsNothing,
      int64_t wrappedNodeCountEstimate, std::string debugDescription) const;

  virtual ~AbstractEdgeOperator();
private:
  ComponentType componentType;
  boost::optional<DB::GetGSFuncT> getGraphStorageFunc;
  boost::optional<DB::GetAllGSFuncT> getAllGraphStorageFunc;

  const DB& db;
  const StringStorage& strings;
  std::string ns;
  std::string name;
  unsigned int minDistance;
  unsigned int maxDistance;
  Annotation anyAnno;
  const Annotation edgeAnno;

  std::vector<std::shared_ptr<const ReadableGraphStorage>> gs;

  void initGraphStorage();
  bool checkEdgeAnnotation(std::shared_ptr<const ReadableGraphStorage> gs, nodeid_t source, nodeid_t target);
};

} // end namespace annis
