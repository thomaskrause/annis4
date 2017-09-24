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

#include "dfs.h"

#include <iostream>                           // for operator<<, cerr, ostream
#include "annis/graphstorage/graphstorage.h"  // for ReadableGraphStorage


using namespace annis;

DFS::DFS(const ReadableGraphStorage &gs,
                                                     std::uint32_t startNode,
                                                     unsigned int minDistance,
                                                     unsigned int maxDistance)
  : startNode(startNode), gs(gs), minDistance(minDistance), maxDistance(maxDistance)
{
  // add the initial value to the stack
  traversalStack.push({startNode, 0});
}

DFSIteratorResult DFS::nextDFS()
{
  DFSIteratorResult result;
  result.found = false;

  while(!result.found && !traversalStack.empty())
  {
    std::pair<uint32_t, unsigned int> stackEntry = traversalStack.top();
    result.node = stackEntry.first;
    result.distance = stackEntry.second;


    // we are entering a new node
    if(beforeEnterNode(result.node, result.distance))
    {
      result.found = enterNode(result.node, result.distance);
    }
    else
    {
      traversalStack.pop();
    }
  }
  return result;
}

bool DFS::enterNode(nodeid_t node, unsigned int distance)
{
  bool found = false;

  traversalStack.pop();

  if(distance >= minDistance && distance <= maxDistance)
  {
    // get the next node
    found = true;
  }

  // add the remaining child nodes
  if(distance < maxDistance)
  {
    // add the outgoing edges to the stack
    auto outgoing = gs.getOutgoingEdges(node);
    for(const auto& outNodeID : outgoing)
    {

      traversalStack.push(std::pair<nodeid_t, unsigned int>(outNodeID, distance+1));
    }
  }
  return found;
}


boost::optional<nodeid_t> DFS::next()
{
  DFSIteratorResult result = nextDFS();
  return result.found ? result.node : boost::optional<nodeid_t>();
}


void DFS::reset()
{
  // clear the stack
  while(!traversalStack.empty())
  {
    traversalStack.pop();
  }

  traversalStack.push({startNode, 0});
}


CycleSafeDFS::CycleSafeDFS(const ReadableGraphStorage &gs, std::uint32_t startNode, unsigned int minDistance, unsigned int maxDistance, bool outputCycleErrors)
  : DFS(gs, startNode, minDistance, maxDistance), lastDistance(0),
    outputCycleErrors(outputCycleErrors),
    cycleDetected(false)
{
  nodesInCurrentPath.insert(startNode);
  distanceToNode.insert({0, startNode});
}


void CycleSafeDFS::reset()
{
  nodesInCurrentPath.clear();
  distanceToNode.clear();

  DFS::reset();

  nodesInCurrentPath.insert(startNode);
  distanceToNode.insert({0, startNode});
}

bool CycleSafeDFS::enterNode(nodeid_t node, unsigned int distance)
{
  nodesInCurrentPath.insert(node);
  distanceToNode.insert({distance, node});

  lastDistance = distance;

  return DFS::enterNode(node, distance);
}

bool CycleSafeDFS::beforeEnterNode(nodeid_t node, unsigned int distance)
{
  if(lastDistance >= distance)
  {
    // A subgraph was completed.
    // Remove all nodes from the path set that are below the parent node:
    for(auto it=distanceToNode.find(distance); it != distanceToNode.end(); it = distanceToNode.erase(it))
    {
      nodesInCurrentPath.erase(it->second);
    }
  }

  if(nodesInCurrentPath.find(node) == nodesInCurrentPath.end())
  {
    return true;
  }
  else
  {
    // we detected a cycle!
    if(outputCycleErrors)
    {
      std::cerr << "------------------------------" << std::endl;
      std::cerr << "ERROR: cycle detected when visting node " << node << std::endl;
      std::cerr << "distanceToNode: ";
      for(auto itPath = distanceToNode.begin(); itPath != distanceToNode.end(); itPath++)
      {
        std::cerr << itPath->first << "->" << itPath->second << " ";
      }
      std::cerr << std::endl;
      std::cerr << "nodesInCurrentPath: ";
      for(auto itPath = nodesInCurrentPath.begin(); itPath != nodesInCurrentPath.end(); itPath++)
      {
        std::cerr << *itPath << " ";
      }
      std::cerr << std::endl;
      std::cerr << "------------------------------" << std::endl;
    }

    lastDistance = distance;
    cycleDetected = true;

    return false;
  }
}

CycleSafeDFS::~CycleSafeDFS()
{

}


UniqueDFS::UniqueDFS(const ReadableGraphStorage &gs, std::uint32_t startNode, unsigned int minDistance, unsigned int maxDistance)
  : DFS(gs, startNode, minDistance, maxDistance)
{

}

UniqueDFS::~UniqueDFS()
{

}

void UniqueDFS::reset()
{
  DFS::reset();
  outputted.clear();
}

bool UniqueDFS::enterNode(nodeid_t node, unsigned int distance)
{
  // always visit all nodes, but do not output a result twice
  if(DFS::enterNode(node, distance))
  {
    if(outputted.find(node) == outputted.end())
    {
      outputted.insert(node);
      return true;
    }
  }

  return false;
}
