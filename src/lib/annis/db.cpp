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

#include "db.h"
#include <annis/annostorage.h>                          // for AnnoStorage
#include <annis/api/graphupdate.h>                      // for UpdateEvent
#include <annis/db.h>                                   // for DB
#include <annis/graphstorage/graphstorage.h>            // for WriteableGrap...
#include <annis/graphstorageregistry.h>                 // for GraphStorageR...
#include <annis/util/helper.h>                          // for Helper
#include <google/btree.h>                               // for btree_iterator
#include <google/btree_container.h>                     // for btree_unique_...
#include <google/btree_map.h>                           // for btree_map
#include <humblelogging/api.h>                          // for HL_INFO, HL_E...
#include <humblelogging/logger.h>                       // for Logger
#include <boost/algorithm/string/predicate.hpp>         // for starts_with
#include <boost/filesystem/operations.hpp>              // for directory_ite...
#include <boost/filesystem/path.hpp>                    // for path, operator/
#include <boost/iterator/iterator_facade.hpp>           // for iterator_faca...
#include <boost/thread/thread.hpp>                      // for interruption_...
#include <cereal/archives/binary.hpp>                   // for BinaryInputAr...
#include <cereal/cereal.hpp>                            // for InputArchive
#include <iostream>                                     // for ifstream, ope...
#include <limits>                                       // for numeric_limits
#include <list>                                         // for list
#include <sstream>
#include "annis/graphstorageholder.h"                   // for GraphStorageH...
#include "annis/stringstorage.h"                        // for StringStorage
#include "annis/types.h"                                // for TextProperty
#include <boost/format.hpp>

#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <malloc.h>
#endif // LINUX


HUMBLE_LOGGER(logger, "annis4");

using namespace annis;
using namespace std;

DB::DB()
: edges(strings), currentChangeID(0)
{
  addDefaultStrings();
}

bool DB::load(string dir, bool preloadComponents)
{
  clear();
  addDefaultStrings();

  boost::filesystem::path dirPath(dir);
  boost::filesystem::path dir2load = dirPath / "current";

  boost::filesystem::path backup = dirPath / "backup";
  bool backupWasLoaded = false;
  if(boost::filesystem::exists(backup) && boost::filesystem::is_directory(backup))
  {
    // load backup instead
    dir2load = backup;
    backupWasLoaded = true;
  }

  std::ifstream is((dir2load / "nodes.cereal").string(), std::ios::binary);
  if(is.is_open())
  {
    cereal::BinaryInputArchive archive(is);
    archive(strings, nodeAnnos);
  }

  bool logfileExists = false;
  // check if we have to apply a log file to get to the last stable snapshot version
  std::ifstream logStream((dir2load / "update_log.cereal").string(), std::ios::binary);
  if(logStream.is_open())
  {
    logfileExists = true;
  }

  // If backup is active or a write log exists, always  a pre-load to get the complete corpus.
  edges.load(dir2load.string(), backupWasLoaded || logfileExists || preloadComponents);

  if(logStream.is_open())
  {
     // apply any outstanding log file updates
     cereal::BinaryInputArchive log(logStream);
     api::GraphUpdate u;
     log(u);
     if(u.getLastConsistentChangeID() > currentChangeID)
     {
       update(u);
     }
  }
  else
  {
    currentChangeID = 0;
  }

  if(backupWasLoaded)
  {
    // save the current corpus under the actual location
    save(dirPath.string());

    // rename backup folder (renaming is atomic and deleting could leave an incomplete backup folder on disk)
    boost::filesystem::path tmpDir =
        boost::filesystem::unique_path(dirPath / "temporary-%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::rename(backup, tmpDir);

    // remove it after renaming it
    boost::filesystem::remove_all(tmpDir);

  }

  // TODO: return false on failure
  return true;
}

bool DB::save(string dir)
{

  // always save to the "current" sub-directory
  boost::filesystem::path dirPath = boost::filesystem::path(dir) / "current";

  boost::filesystem::create_directories(dirPath);

  boost::this_thread::interruption_point();

  std::ofstream os((dirPath / "nodes.cereal").string(), std::ios::binary);
  cereal::BinaryOutputArchive archive( os );
  archive(strings, nodeAnnos);

  boost::this_thread::interruption_point();

  edges.save(dirPath.string());

  boost::this_thread::interruption_point();

  // this is a good time to remove all uncessary data like backups or write logs
  for(auto fileIt = boost::filesystem::directory_iterator(dirPath);
      fileIt != boost::filesystem::directory_iterator(); fileIt++)
  {
    boost::this_thread::interruption_point();
    if(boost::filesystem::is_directory(fileIt->path()))
    {
      if(boost::algorithm::starts_with(fileIt->path().filename().string(), "temporary-"))
      {
        boost::filesystem::remove_all(fileIt->path());
      }
    }
    else if(fileIt->path().filename() == "update_log.cereal")
    {
      boost::filesystem::remove(fileIt->path());
    }
  }

  // TODO: return false on failure
  return true;
}

std::string DB::getNodeDebugName(const nodeid_t &id) const
{
  std::stringstream ss;
  ss << getNodeDocument(id) << ":" << getNodeName(id) << "(" << id << ")";

  return ss.str();
}

bool DB::loadRelANNIS(string dirPath)
{
  clear();
  addDefaultStrings();

  // check if this is the ANNIS 3.3 import format
  bool isANNIS33Format = false;
  if(boost::filesystem::is_regular_file(dirPath + "/annis.version"))
  {
    ifstream inVersion;
    inVersion.open(dirPath + "/annis.version", ifstream::in);
    if (inVersion.good())
    {
      std::string versionStr;
      inVersion >> versionStr;
      if(versionStr == "3.3")
      {
        isANNIS33Format = true;
      }
    }
  }
  
  map<uint32_t, std::string> corpusIDToName;
  std::string toplevelCorpusName = loadRelANNISCorpusTab(dirPath, corpusIDToName, isANNIS33Format);
  if(toplevelCorpusName.empty())
  {
    std::cerr << "Could not find toplevel corpus name" << std::endl;
    return false;
  }

  if(loadRelANNISNode(dirPath, corpusIDToName, toplevelCorpusName, isANNIS33Format) == false)
  {
    return false;
  }

  string componentTabPath = dirPath + "/component" + (isANNIS33Format ? ".annis" : ".tab");
  HL_INFO(logger, (boost::format("loading %1%") % componentTabPath).str());

  ifstream in;
  vector<string> line;

  in.open(componentTabPath, ifstream::in);
  if(!in.good()) return false;

  map<uint32_t, std::shared_ptr<WriteableGraphStorage>> componentToGS;
  while((line = Helper::nextCSV(in)).size() > 0)
  {
    uint32_t componentID = Helper::uint32FromString(line[0]);
    if(line[1] != "NULL")
    {
      ComponentType ctype = edges.componentTypeFromShortName(line[1]);
      std::shared_ptr<WriteableGraphStorage> gs = edges.createWritableGraphStorage(ctype, line[2], line[3]);
      componentToGS[componentID] = gs;
    }
  }

  in.close();

  bool result = loadRelANNISRank(dirPath, componentToGS, isANNIS33Format);


  // construct the complex indexes for all components
  std::list<Component> componentCopy;
  for(auto& gs : edges.container)
  {
    componentCopy.push_back(gs.first);
  }
  for(auto c : componentCopy)
  {
    convertComponent(c);
  }

  HL_INFO(logger, "Updating statistics");
  nodeAnnos.calculateStatistics(strings);

  #if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
  malloc_trim(0);
  #endif // LINUX

  HL_INFO(logger, "Finished loading relANNIS");
  return result;
}


std::string DB::loadRelANNISCorpusTab(string dirPath, map<uint32_t, std::string>& corpusIDToName,
  bool isANNIS33Format)
{
  std::string toplevelCorpus = "";

  string corpusTabPath = dirPath + "/corpus" + (isANNIS33Format ? ".annis" : ".tab");
  HL_INFO(logger, (boost::format("loading %1%") % corpusTabPath).str());

  ifstream in;
  in.open(corpusTabPath, ifstream::in);
  if(!in.good())
  {
    string msg = "Can't find corpus";
    msg += (isANNIS33Format ? ".annis" : ".tab");
    HL_ERROR(logger, msg);
    return "";
  }
  vector<string> line;
  while((line = Helper::nextCSV(in)).size() > 0)
  {
    std::uint32_t corpusID = Helper::uint32FromString(line[0]);
    corpusIDToName[corpusID] = line[1];

    if(line[2] == "CORPUS" && line[4] == "0")
    {
      toplevelCorpus = line[1];
    }
  }
  return toplevelCorpus;
}

bool DB::loadRelANNISNode(string dirPath, map<uint32_t, std::string>& corpusIDToName, std::string toplevelCorpusName,
  bool isANNIS33Format)
{
  typedef multimap<TextProperty, uint32_t>::const_iterator TextPropIt;

  // maps a token index to an node ID
  map<TextProperty, uint32_t> tokenByIndex;

  // map the "left" value to the nodes it belongs to
  multimap<TextProperty, nodeid_t> leftToNode;
  // map the "right" value to the nodes it belongs to
  multimap<TextProperty, nodeid_t> rightToNode;
  // map as node to it's "left" value
  map<nodeid_t, uint32_t> nodeToLeft;
  // map as node to it's "right" value
  map<nodeid_t, uint32_t> nodeToRight;

  // maps a character position to it's token
  map<TextProperty, nodeid_t> tokenByTextPosition;

  string nodeTabPath = dirPath + "/node" + (isANNIS33Format ? ".annis" : ".tab");
  HL_INFO(logger, (boost::format("loading %1%") % nodeTabPath).str());

  ifstream in;
  in.open(nodeTabPath, ifstream::in);
  if(!in.good())
  {
    std::string msg = "Can't find node";
    msg += isANNIS33Format ? ".annis" : ".tab";
    HL_ERROR(logger, msg);
    return false;
  }

  std::list<std::pair<NodeAnnotationKey, uint32_t>> annoList;

  {
    vector<string> line;
    while((line = Helper::nextCSV(in)).size() > 0)
    {
      uint32_t nodeNr;
      stringstream nodeNrStream(line[0]);
      nodeNrStream >> nodeNr;

      bool hasSegmentations = line.size() > 10;
      string tokenIndexRaw = line[7];
      uint32_t textID = Helper::uint32FromString(line[1]);
      uint32_t corpusID = Helper::uint32FromString(line[2]);

      std::string docName = corpusIDToName[corpusID];

      Annotation nodeNameAnno;
      nodeNameAnno.ns = strings.add(annis_ns);
      nodeNameAnno.name =  strings.add(annis_node_name);
      nodeNameAnno.val = strings.add(toplevelCorpusName + "/" + docName + "#" + line[4]);
      annoList.push_back(std::pair<NodeAnnotationKey, uint32_t>({nodeNr, nodeNameAnno.name, nodeNameAnno.ns }, nodeNameAnno.val));


      Annotation documentNameAnno;
      documentNameAnno.ns = strings.add(annis_ns);
      documentNameAnno.name = strings.add("document");
      documentNameAnno.val = strings.add(docName);
      annoList.push_back(std::pair<NodeAnnotationKey, uint32_t>({nodeNr, documentNameAnno.name, documentNameAnno.ns }, documentNameAnno.val));

      TextProperty left;
      left.val = Helper::uint32FromString(line[5]);
      left.textID = textID;
      left.corpusID = corpusID;

      TextProperty right;
      right.val = Helper::uint32FromString(line[6]);
      right.textID = textID;
      right.corpusID = corpusID;

      if(tokenIndexRaw != "NULL")
      {
        string span = hasSegmentations ? line[12] : line[9];

        Annotation tokAnno;
        tokAnno.ns = strings.add(annis_ns);
        tokAnno.name = strings.add(annis_tok);
        tokAnno.val = strings.add(span);
        annoList.push_back(std::pair<NodeAnnotationKey, uint32_t>({nodeNr, tokAnno.name, tokAnno.ns }, tokAnno.val));

        TextProperty index;
        index.val = Helper::uint32FromString(tokenIndexRaw);
        index.textID = textID;
        index.corpusID = corpusID;

        tokenByIndex[index] = nodeNr;

        TextProperty textPos;
        textPos.textID = textID;
        textPos.corpusID = corpusID;
        for(uint32_t i=left.val; i <= right.val; i++)
        {
          textPos.val = i;
          tokenByTextPosition[textPos] = nodeNr;
        }

      } // end if token

      leftToNode.insert(pair<TextProperty, uint32_t>(left, nodeNr));
      rightToNode.insert(pair<TextProperty, uint32_t>(right, nodeNr));
      nodeToLeft[nodeNr] = left.val;
      nodeToRight[nodeNr] = right.val;
    }

    in.close();
  }

  // TODO: cleanup, better variable naming and put this into it's own function
  // iterate over all token by their order, find the nodes with the same
  // text coverage (either left or right) and add explicit ORDERING, LEFT_TOKEN and RIGHT_TOKEN edges
  if(!tokenByIndex.empty())
  {
    HL_INFO(logger, "calculating the automatically generated ORDERING, LEFT_TOKEN and RIGHT_TOKEN edges");
    std::shared_ptr<WriteableGraphStorage> gsOrder = edges.createWritableGraphStorage(ComponentType::ORDERING, annis_ns, "");
    std::shared_ptr<WriteableGraphStorage> gsLeft = edges.createWritableGraphStorage(ComponentType::LEFT_TOKEN, annis_ns, "");
    std::shared_ptr<WriteableGraphStorage> gsRight = edges.createWritableGraphStorage(ComponentType::RIGHT_TOKEN, annis_ns, "");

    map<TextProperty, uint32_t>::const_iterator tokenIt = tokenByIndex.begin();
    uint32_t lastTextID = numeric_limits<uint32_t>::max();
    uint32_t lastCorpusID = numeric_limits<uint32_t>::max();
    uint32_t lastToken = numeric_limits<uint32_t>::max();

    while(tokenIt != tokenByIndex.end())
    {
      uint32_t currentToken = tokenIt->second;
      uint32_t currentTextID = tokenIt->first.textID;
      uint32_t currentCorpusID = tokenIt->first.corpusID;

      // find all nodes that start together with the current token
      TextProperty currentTokenLeft;
      currentTokenLeft.textID = currentTextID;
      currentTokenLeft.corpusID = currentCorpusID;
      currentTokenLeft.val = nodeToLeft[currentToken];

      pair<TextPropIt, TextPropIt> leftAlignedNodes = leftToNode.equal_range(currentTokenLeft);
      for(TextPropIt itLeftAligned=leftAlignedNodes.first; itLeftAligned != leftAlignedNodes.second; itLeftAligned++)
      {
        gsLeft->addEdge(Init::initEdge(itLeftAligned->second, currentToken));
        gsLeft->addEdge(Init::initEdge(currentToken, itLeftAligned->second));
      }

      // find all nodes that end together with the current token
      TextProperty currentTokenRight;
      currentTokenRight.textID = currentTextID;
      currentTokenRight.corpusID = currentCorpusID;
      currentTokenRight.val = nodeToRight[currentToken];


      pair<TextPropIt, TextPropIt> rightAlignedNodes = rightToNode.equal_range(currentTokenRight);
      for(TextPropIt itRightAligned=rightAlignedNodes.first;
          itRightAligned != rightAlignedNodes.second;
          itRightAligned++)
      {
        gsRight->addEdge(Init::initEdge(itRightAligned->second, currentToken));
        gsRight->addEdge(Init::initEdge(currentToken, itRightAligned->second));
      }

      // if the last token/text value is valid and we are still in the same text
      if(tokenIt != tokenByIndex.begin()
         && currentCorpusID == lastCorpusID
         && currentTextID == lastTextID)
      {
        // we are still in the same text
        uint32_t nextToken = tokenIt->second;
        // add ordering between token
        gsOrder->addEdge(Init::initEdge(lastToken, nextToken));

      } // end if same text

      // update the iterator and other variables
      lastTextID = currentTextID;
      lastCorpusID = currentCorpusID;
      lastToken = tokenIt->second;
      tokenIt++;
    } // end for each token
  }

  // add explicit coverage edges for each node in the special annis namespace coverage component
  std::shared_ptr<WriteableGraphStorage> gsCoverage = edges.createWritableGraphStorage(ComponentType::COVERAGE, annis_ns, "");
  std::shared_ptr<WriteableGraphStorage> gsInverseCoverage = edges.createWritableGraphStorage(ComponentType::INVERSE_COVERAGE, annis_ns, "");
  HL_INFO(logger, "calculating the automatically generated COVERAGE edges");
  for(multimap<TextProperty, nodeid_t>::const_iterator itLeftToNode = leftToNode.begin();
      itLeftToNode != leftToNode.end(); itLeftToNode++)
  {
    nodeid_t n = itLeftToNode->second;

    TextProperty textPos;
    textPos.textID = itLeftToNode->first.textID;
    textPos.corpusID = itLeftToNode->first.corpusID;

    uint32_t left = itLeftToNode->first.val;
    uint32_t right = nodeToRight[n];

    for(uint32_t i = left; i < right; i++)
    {
      // get the token that belongs to this text position
      textPos.val = i;
      nodeid_t tokenID = tokenByTextPosition[textPos];
      if(n != tokenID)
      {
        gsCoverage->addEdge(Init::initEdge(n, tokenID));
        gsInverseCoverage->addEdge(Init::initEdge(tokenID, n));
      }
    }
  }

  {
    string nodeAnnoTabPath = dirPath + "/node_annotation"  + (isANNIS33Format ? ".annis" : ".tab");
    HL_INFO(logger, (boost::format("loading %1%") % nodeAnnoTabPath).str());

    in.open(nodeAnnoTabPath, ifstream::in);
    if(!in.good()) return false;

    vector<string> line;
    while((line = Helper::nextCSV(in)).size() > 0)
    {
      NodeAnnotationKey key;
      key.id = Helper::uint32FromString(line[0]);
      key.anno_ns = strings.add(line[1]);
      key.anno_name = strings.add(line[2]);

      uint32_t annoVal = strings.add(line[3]);
      annoList.push_back({key, annoVal});
    }

    in.close();
  }

  HL_INFO(logger, "bulk inserting node annotations");
  nodeAnnos.addAnnotationBulk(annoList);

  return true;
}


bool DB::loadRelANNISRank(const string &dirPath,
                          const map<uint32_t, std::shared_ptr<WriteableGraphStorage>>& componentToEdgeGS,
                          bool isANNIS33Format)
{
  typedef btree::btree_map<uint32_t, uint32_t>::const_iterator UintMapIt;
  typedef map<uint32_t, std::shared_ptr<WriteableGraphStorage>>::const_iterator ComponentIt;
  bool result = true;

  ifstream in;
  string rankTabPath = dirPath + "/rank" + (isANNIS33Format ? ".annis" : ".tab");
  HL_INFO(logger, (boost::format("loading %1%") % rankTabPath).str());

  in.open(rankTabPath, ifstream::in);
  if(!in.good()) return false;

  vector<string> line;

  const size_t nodeRefPos = isANNIS33Format ? 3 : 2;
  const size_t componentRefPos = isANNIS33Format ? 4 : 3;
  const size_t parentPos = isANNIS33Format ? 5 : 4;
  
  // first run: collect all pre-order values for a node
  btree::btree_map<uint32_t, uint32_t> pre2NodeID;
  map<uint32_t, Edge> pre2Edge;

  while((line = Helper::nextCSV(in)).size() > 0)
  {
    pre2NodeID.insert({Helper::uint32FromString(line[0]),Helper::uint32FromString(line[nodeRefPos])});
  }

  in.close();

  in.open(rankTabPath, ifstream::in);
  if(!in.good()) return false;

  map<uint32_t, std::shared_ptr<WriteableGraphStorage> > pre2GS;

  // second run: get the actual edges
  while((line = Helper::nextCSV(in)).size() > 0)
  {
    std::string parentAsString = line[parentPos];
    if(parentAsString != "NULL")
    {
      uint32_t parent = Helper::uint32FromString(parentAsString);
      UintMapIt it = pre2NodeID.find(parent);
      if(it != pre2NodeID.end())
      {
        // find the responsible edge database by the component ID
        ComponentIt itGS = componentToEdgeGS.find(Helper::uint32FromString(line[componentRefPos]));
        if(itGS != componentToEdgeGS.end())
        {
          std::shared_ptr<WriteableGraphStorage> gs = itGS->second;
          Edge edge = Init::initEdge(it->second, Helper::uint32FromString(line[nodeRefPos]));

          gs->addEdge(edge);
          pre2Edge[Helper::uint32FromString(line[0])] = edge;
          pre2GS[Helper::uint32FromString(line[0])] = gs;
        }
      }
      else
      {
        result = false;
      }
    }
  }
  in.close();


  if(result)
  {

    result = loadEdgeAnnotation(dirPath, pre2GS, pre2Edge, isANNIS33Format);
  }

  return result;
}


bool DB::loadEdgeAnnotation(const string &dirPath,
                            const map<uint32_t, std::shared_ptr<WriteableGraphStorage> >& pre2GS,
                            const map<uint32_t, Edge>& pre2Edge,
                            bool isANNIS33Format)
{

  bool result = true;

  ifstream in;
  string edgeAnnoTabPath = dirPath + "/edge_annotation" + (isANNIS33Format ? ".annis" : ".tab");
  HL_INFO(logger, (boost::format("loading %1%") % edgeAnnoTabPath).str());

  in.open(edgeAnnoTabPath, ifstream::in);
  if(!in.good()) return false;

  vector<string> line;

  while((line = Helper::nextCSV(in)).size() > 0)
  {
    uint32_t pre = Helper::uint32FromString(line[0]);
    map<uint32_t, std::shared_ptr<WriteableGraphStorage>>::const_iterator itDB = pre2GS.find(pre);
    map<uint32_t, Edge>::const_iterator itEdge = pre2Edge.find(pre);
    if(itDB != pre2GS.end() && itEdge != pre2Edge.end())
    {
      std::shared_ptr<WriteableGraphStorage> e = itDB->second;
      Annotation anno;
      anno.ns = strings.add(line[1]);
      anno.name = strings.add(line[2]);
      anno.val = strings.add(line[3]);
      if(e != NULL)
      {
        e->addEdgeAnnotation(itEdge->second, anno);
      }
    }
    else
    {
      result = false;
    }
  }

  in.close();

  return result;
}

void DB::clear()
{
  strings.clear();
  nodeAnnos.clear();
  edges.clear();
}

void DB::addDefaultStrings()
{
  annisNamespaceStringID = strings.add(annis_ns);
  annisEmptyStringID = strings.add("");
  annisTokStringID = strings.add(annis_tok);
  annisNodeNameStringID = strings.add(annis_node_name);
}

nodeid_t DB::nextFreeNodeID() const
{
  return nodeAnnos.annotations.empty() ? 0 : (nodeAnnos.annotations.rbegin()->first.id) + 1;
}

void DB::convertComponent(Component c, std::string impl)
{
  map<Component, std::shared_ptr<ReadableGraphStorage>>::const_iterator
      it = edges.container.find(c);
  if(it != edges.container.end() && it->second)
  {
    std::shared_ptr<ReadableGraphStorage> oldStorage = it->second;

    if (!(oldStorage->getStatistics().valid))
    {
      oldStorage->calculateStatistics(strings);
    }

    std::string currentImpl = edges.registry.getName(oldStorage);
    if(impl == "")
    {
      // no manual implementation given as argument, automatically determine the best one.
      impl = edges.registry.getOptimizedImpl(c, oldStorage->getStatistics());
    }
    std::shared_ptr<ReadableGraphStorage> newStorage = oldStorage;
    if(currentImpl != impl)
    {
      HL_DEBUG(logger, (boost::format("converting component %1% from %2% to %3%")
                       % edges.debugComponentString(c)
                       % currentImpl
                       % impl).str());

      newStorage = edges.registry.createGraphStorage(impl, strings, c);
      newStorage->copy(*this, *oldStorage);
      edges.container[c] = newStorage;
    }

    // perform index calculations
    std::shared_ptr<WriteableGraphStorage> asWriteableGS = std::dynamic_pointer_cast<WriteableGraphStorage>(newStorage);
    if(asWriteableGS)
    {
      asWriteableGS->calculateIndex();
    }
  }
}

void DB::optimizeAll(const std::map<Component, string>& manualExceptions)
{
  for(const auto& c : getAllComponents())
  {
    edges.ensureComponentIsLoaded(c);
    auto find = manualExceptions.find(c);
    if(find == manualExceptions.end())
    {
      // get the automatic calculated best implementation
      convertComponent(c);
    }
    else
    {
      convertComponent(c, find->second);
    }
  }
}

void DB::ensureAllComponentsLoaded()
{
  for(const auto& c : getAllComponents())
  {
    edges.ensureComponentIsLoaded(c);
  }
}

size_t DB::estimateMemorySize()
{
  return
    nodeAnnos.estimateMemorySize()
      + strings.estimateMemorySize()
      + edges.estimateMemorySize();
}

string DB::info()
{
  stringstream ss;
  ss  << "Number of node annotations: " << nodeAnnos.numberOfAnnotations() << endl
      << "Number of strings in storage: " << strings.size() << endl
      << "Average string length: " << strings.avgLength() << endl
      << "--------------------" << std::endl
      << edges.info() << std::endl;

  return ss.str();
}


std::vector<Component> DB::getDirectConnected(const Edge &edge) const
{
  std::vector<Component> result;
  map<Component, std::shared_ptr<ReadableGraphStorage>>::const_iterator itGS = edges.container.begin();

  while(itGS != edges.container.end())
  {
    std::shared_ptr<ReadableGraphStorage> gs = itGS->second;
    if(gs != NULL)
    {
      if(gs->isConnected(edge))
      {
        result.push_back(itGS->first);
      }
    }
    itGS++;
  }

  return result;
}

std::vector<Component> DB::getAllComponents() const
{
  std::vector<Component> result;
  map<Component, std::shared_ptr<ReadableGraphStorage>>::const_iterator itGS = edges.container.begin();

  while(itGS != edges.container.end())
  {
    result.push_back(itGS->first);
    itGS++;
  }

  return result;
}

vector<Annotation> DB::getEdgeAnnotations(const Component &component,
                                          const Edge &edge)
{
  std::map<Component,std::shared_ptr<ReadableGraphStorage>>::const_iterator it = edges.container.find(component);
  if(it != edges.container.end() && it->second != NULL)
  {
    std::shared_ptr<ReadableGraphStorage> gs = it->second;
    return gs->getEdgeAnnotations(edge);
  }

  return vector<Annotation>();

}

void DB::update(const api::GraphUpdate& u)
{
   for(std::shared_ptr<api::UpdateEvent> change : u.getDiffs())
   {
      if(change->changeID <= u.getLastConsistentChangeID())
      {
         if(std::shared_ptr<api::AddNodeEvent> evt = std::dynamic_pointer_cast<api::AddNodeEvent>(change))
         {
            auto existingNodeID = getNodeID(evt->nodeName);
            // only add node if it does not exist yet
            if(!existingNodeID)
            {
               nodeid_t newNodeID = nextFreeNodeID();
               Annotation newAnno =
                  {getNodeNameStringID(), getNamespaceStringID(), strings.add(evt->nodeName)};
               nodeAnnos.addAnnotation(newNodeID, newAnno);
            }
         }
         else if(std::shared_ptr<api::DeleteNodeEvent> evt = std::dynamic_pointer_cast<api::DeleteNodeEvent>(change))
         {
            auto existingNodeID = getNodeID(evt->nodeName);
            if(existingNodeID)
            {
               // add all annotations
               std::vector<Annotation> annoList = nodeAnnos.getAnnotations(*existingNodeID);
               for(Annotation anno : annoList)
               {
                  AnnotationKey annoKey = {anno.name, anno.ns};
                  nodeAnnos.deleteAnnotation(*existingNodeID, annoKey);
               }
               // delete all edges pointing to this node either as source or target
               for(Component c : getAllComponents())
               {
                  std::shared_ptr<WriteableGraphStorage> gs =
                    edges.createWritableGraphStorage(c.type, c.layer, c.name);
                  gs->deleteNode(*existingNodeID);
               }

            }
         }
         else if(std::shared_ptr<api::AddNodeLabelEvent> evt = std::dynamic_pointer_cast<api::AddNodeLabelEvent>(change))
         {
            auto existingNodeID = getNodeID(evt->nodeName);
            if(existingNodeID)
            {
              Annotation anno = {strings.add(evt->annoName),
                                 strings.add(evt->annoNs),
                                 strings.add(evt->annoValue)};
              nodeAnnos.addAnnotation(*existingNodeID, anno);
            }
         }
         else if(std::shared_ptr<api::DeleteNodeLabelEvent> evt = std::dynamic_pointer_cast<api::DeleteNodeLabelEvent>(change))
         {
            auto existingNodeID = getNodeID(evt->nodeName);
            if(existingNodeID)
            {
              AnnotationKey annoKey = {strings.add(evt->annoName),
                                       strings.add(evt->annoNs)};
              nodeAnnos.deleteAnnotation(*existingNodeID, annoKey);
            }
         }
         else if(std::shared_ptr<api::AddEdgeEvent> evt = std::dynamic_pointer_cast<api::AddEdgeEvent>(change))
         {
            auto existingSourceID = getNodeID(evt->sourceNode);
            auto existingTargetID = getNodeID(evt->targetNode);
            // only add edge if both nodes already exist
            if(existingSourceID && existingTargetID)
            {
               ComponentType type = ComponentTypeHelper::fromString(evt->componentType);
               if(type < ComponentType::ComponentType_MAX)
               {
                  std::shared_ptr<WriteableGraphStorage> gs =
                    edges.createWritableGraphStorage(type, evt->layer, evt->componentName);
                  gs->addEdge({*existingSourceID, *existingTargetID});
               }
            }
         }
         else if(std::shared_ptr<api::DeleteEdgeEvent> evt = std::dynamic_pointer_cast<api::DeleteEdgeEvent>(change))
         {
            auto existingSourceID = getNodeID(evt->sourceNode);
            auto existingTargetID = getNodeID(evt->targetNode);
            // only delete edge if both nodes actually exist
            if(existingSourceID && existingTargetID)
            {
               ComponentType type = ComponentTypeHelper::fromString(evt->componentType);
               if(type < ComponentType::ComponentType_MAX)
               {
                  std::shared_ptr<WriteableGraphStorage> gs =
                    edges.createWritableGraphStorage(type, evt->layer, evt->componentName);
                  gs->deleteEdge({*existingSourceID, *existingTargetID});
               }
            }
         }
         else if(std::shared_ptr<api::AddEdgeLabelEvent> evt = std::dynamic_pointer_cast<api::AddEdgeLabelEvent>(change))
         {
           auto existingSourceID = getNodeID(evt->sourceNode);
           auto existingTargetID = getNodeID(evt->targetNode);
           // only add label if both nodes already exists
           if(existingSourceID && existingTargetID)
           {
              ComponentType type = ComponentTypeHelper::fromString(evt->componentType);
              if(type < ComponentType::ComponentType_MAX)
              {
                 std::shared_ptr<WriteableGraphStorage> gs =
                   edges.createWritableGraphStorage(type, evt->layer, evt->componentName);

                 // only add label if the edge already exists
                 if(gs->isConnected({*existingSourceID, *existingTargetID}, 1, 1))
                 {
                   Annotation anno = {strings.add(evt->annoName), strings.add(evt->annoNs), strings.add(evt->annoValue)};
                   gs->addEdgeAnnotation({*existingSourceID, *existingTargetID}, anno);
                 }

              }
           }
         }
         else if(std::shared_ptr<api::DeleteEdgeLabelEvent> evt = std::dynamic_pointer_cast<api::DeleteEdgeLabelEvent>(change))
         {
           auto existingSourceID = getNodeID(evt->sourceNode);
           auto existingTargetID = getNodeID(evt->targetNode);
           // only add label if both nodes actually exists
           if(existingSourceID && existingTargetID)
           {
              ComponentType type = ComponentTypeHelper::fromString(evt->componentType);
              if(type < ComponentType::ComponentType_MAX)
              {
                 std::shared_ptr<WriteableGraphStorage> gs =
                   edges.createWritableGraphStorage(type, evt->layer, evt->componentName);

                 // only delete label if the edge actually exists
                 if(gs->isConnected({*existingSourceID, *existingTargetID}, 1, 1))
                 {
                   AnnotationKey annoKey = {strings.add(evt->annoName), strings.add(evt->annoNs)};
                   gs->deleteEdgeAnnotation({*existingSourceID, *existingTargetID}, annoKey);
                 }

              }
           }
         }
         currentChangeID = change->changeID;
      } // end if changeID is behind last consistent
   } // end for each change in update list

}

DB::~DB()
{
}


