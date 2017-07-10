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

#include "overlap.h"
#include <annis/wrapper.h>                    // for ListWrapper
#include <google/btree_set.h>                 // for btree_set
#include <utility>                            // for pair, move
#include <vector>                             // for vector
#include "annis/graphstorage/graphstorage.h"  // for ReadableGraphStorage
#include "annis/iterators.h"                  // for EdgeIterator, AnnoIt
#include "annis/operators/operator.h"         // for Operator
#include "annis/util/helper.h"                // for TokenHelper


using namespace annis;

Overlap::Overlap(const DB &db, DB::GetGSFuncT getGraphStorageFunc)
  : tokHelper(getGraphStorageFunc, db), anyNodeAnno(Init::initAnnotation(db.getNodeTypeStringID(), 0, db.getNamespaceStringID()))
{
  gsOrder = getGraphStorageFunc(ComponentType::ORDERING, annis_ns, "");
  gsCoverage = getGraphStorageFunc(ComponentType::COVERAGE, annis_ns, "");
  gsInverseCoverage = getGraphStorageFunc(ComponentType::INVERSE_COVERAGE, annis_ns, "");
}

std::unique_ptr<AnnoIt> Overlap::retrieveMatches(const annis::Match &lhs)
{
  std::unique_ptr<ListWrapper> w = std::unique_ptr<ListWrapper>(new ListWrapper());

  btree::btree_set<nodeid_t> uniqueResultSet;

  // get covered token of lhs
  if(tokHelper.isToken(lhs.node))
  {
    // get all nodes that are covering this token
    std::vector<nodeid_t> overlapCandidates = gsInverseCoverage->getOutgoingEdges(lhs.node);
    for(const auto& c : overlapCandidates)
    {
      uniqueResultSet.insert(c);
    }
     // also add the token itself
    uniqueResultSet.insert(lhs.node);
  }
  else
  {

    std::unique_ptr<EdgeIterator> coveredByLeftIt
        = gsCoverage->findConnected(lhs.node);
    for(auto leftToken = coveredByLeftIt->next();
        leftToken.first; leftToken = coveredByLeftIt->next())
    {

      // get all nodes that are covering the token
      std::vector<nodeid_t> overlapCandidates = gsInverseCoverage->getOutgoingEdges(leftToken.second);
      for(const auto& c : overlapCandidates)
      {
        uniqueResultSet.insert(c);
      }
       // also add the token itself
      uniqueResultSet.insert(leftToken.second);
    }
  }

  // add all unique matches to result
  for(const auto& m : uniqueResultSet)
  {
    w->addMatch(Init::initMatch(anyNodeAnno, m));
  }

  return std::move(w);
}

bool Overlap::filter(const Match &lhs, const Match &rhs)
{
  nodeid_t lhsLeftToken = tokHelper.leftTokenForNode(lhs.node);
  nodeid_t lhsRightToken = tokHelper.rightTokenForNode(lhs.node);
  nodeid_t rhsLeftToken = tokHelper.leftTokenForNode(rhs.node);
  nodeid_t rhsRightToken = tokHelper.rightTokenForNode(rhs.node);

  if(gsOrder->distance(Init::initEdge(lhsLeftToken, rhsRightToken)) >= 0
     && gsOrder->distance(Init::initEdge(rhsLeftToken, lhsRightToken)) >= 0)
  {
    return true;
  }
  return false;
}

double Overlap::selectivity() 
{
  if(gsOrder == nullptr || gsCoverage == nullptr)
  {
    return Operator::selectivity();
  }

  auto statsCov = gsCoverage->getStatistics();
  auto statsOrder = gsOrder->getStatistics();
  auto statsInvCov = gsInverseCoverage->getStatistics();


  double numOfToken = statsOrder.nodes;

  if(statsCov.nodes == 0)
  {
    // only token in this corpus
    return 1.0 / numOfToken;
  }
  else
  {
    double covered_token_per_node = statsCov.fanOut99Percentile;
    // for each covered token get the number of inverse covered non-token nodes
    double aligned_non_token = covered_token_per_node * statsInvCov.fanOut99Percentile;

    double sum_included = covered_token_per_node + aligned_non_token;
    return sum_included / (double) statsCov.nodes;
  }

}


Overlap::~Overlap()
{

}
