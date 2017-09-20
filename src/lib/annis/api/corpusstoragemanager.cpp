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

#include "corpusstoragemanager.h"

#include <annis/db.h>                                   // for DB
#include <annis/util/relannisloader.h>
#include <humblelogging/api.h>                          // for HL_DEBUG, HUM...
#include <humblelogging/logger.h>                       // for Logger
#include <stdint.h>                                     // for uint32_t
#include <algorithm>                                    // for sort
#include <boost/thread.hpp>  // for thread
#include <boost/filesystem/operations.hpp>              // for directory_ite...
#include <boost/filesystem/path.hpp>                    // for path, operator/
#include <boost/filesystem/path_traits.hpp>             // for filesystem
#include <boost/iterator/iterator_facade.hpp>           // for iterator_faca...
#include <boost/thread/lock_guard.hpp>                  // for lock_guard
#include <boost/thread/lock_options.hpp>                // for adopt_lock
#include <boost/thread/thread.hpp>                      // for interruption_...
#include <boost/thread/shared_lock_guard.hpp>           // for shared_lock_g...
#include <boost/algorithm/string.hpp>
#include <cereal/archives/binary.hpp>                   // for BinaryOutputA...
#include <cereal/cereal.hpp>                            // for OutputArchive
#include <fstream>                                      // for stringstream
#include <set>                                          // for _Rb_tree_iter...
#include <utility>                                      // for pair
#include <vector>                                       // for vector
#include "annis/api/graphupdate.h"                      // for GraphUpdate
#include "annis/dbloader.h"                             // for DBLoader
#include "annis/json/jsonqueryparser.h"                 // for JSONQueryParser
#include "annis/query/query.h"
#include "annis/annostorage.h"                          // for AnnoStorage
#include "annis/stringstorage.h"                        // for StringStorage
#include "annis/types.h"                                // for Match, Annota...
#include <annis/annosearch/exactannovaluesearch.h>
#include <annis/annosearch/exactannokeysearch.h>
#include <annis/annosearch/regexannosearch.h>
#include <annis/graphstorage/graphstorage.h>
#include <annis/operators/overlap.h>
#include <annis/operators/precedence.h>
#include <annis/operators/partofsubcorpus.h>
#include <annis/operators/identicalnode.h>

#include <functional>



using namespace annis;
using namespace annis::api;

namespace bf = boost::filesystem;

HUMBLE_LOGGER(logger, "annis4");

CorpusStorageManager::CorpusStorageManager(std::string databaseDir, size_t maxAllowedCacheSize)
  : databaseDir(databaseDir), maxAllowedCacheSize(maxAllowedCacheSize)
{
}

CorpusStorageManager::~CorpusStorageManager() {}

long long CorpusStorageManager::count(std::vector<std::string> corpora, std::string queryAsJSON)
{
  long long result = 0;

  // sort corpora by their name
  std::sort(corpora.begin(), corpora.end());

  for(const std::string& c : corpora)
  {
    std::shared_ptr<DBLoader> loader = getCorpusFromCache(c);

    if(loader)
    {
      boost::upgrade_lock<DBLoader> lock(*loader);

      std::shared_ptr<annis::Query> q = annis::JSONQueryParser::parseWithUpgradeableLock(loader->get(), queryAsJSON, lock);
      while(q->next())
      {
        result++;
      }
    }
  }
  return result;
}

CorpusStorageManager::CountResult CorpusStorageManager::countExtra(std::vector<std::string> corpora, std::string queryAsJSON)
{
  CountResult result = {0,0};

  std::unordered_set<std::string> documents;

  // sort corpora by their name
  std::sort(corpora.begin(), corpora.end());

  for(const std::string& c : corpora)
  {
    std::shared_ptr<DBLoader> loader = getCorpusFromCache(c);

    if(loader)
    {
      boost::upgrade_lock<DBLoader> lock(*loader);
      std::shared_ptr<annis::Query> q = annis::JSONQueryParser::parseWithUpgradeableLock(loader->get(), queryAsJSON, lock);

      const DB& db = loader->get();

      while(q->next())
      {
        result.matchCount++;
        const std::vector<Match>& m = q->getCurrent();
        if(!m.empty())
        {
          const Match& n  = m[0];
          boost::optional<Annotation> anno = db.nodeAnnos.getAnnotations(db.strings, n.node, annis_ns, annis_node_name);
          if(anno)
          {
            // extract the document path from the node name
            const std::string& value = db.strings.str(anno->val);
            std::string docPath = value.substr(0, value.size()-value.find_first_of('#'));
            documents.insert(docPath);
          }
        }
      }
    }
  }

  result.documentCount = documents.size();
  return result;
}

std::vector<std::string> CorpusStorageManager::find(std::vector<std::string> corpora, std::string queryAsJSON, long long offset, long long limit)
{
  std::vector<std::string> result;

  long long counter = 0;

  // sort corpora by their name
  std::sort(corpora.begin(), corpora.end());

  for(const std::string& c : corpora)
  {
    std::shared_ptr<DBLoader> loader = getCorpusFromCache(c);

    if(loader)
    {
      boost::upgrade_lock<DBLoader> lock(*loader);

      std::shared_ptr<annis::Query> q = annis::JSONQueryParser::parseWithUpgradeableLock(loader->get(), queryAsJSON, lock);

      const DB& db = loader->get();

      while((limit <= 0 || counter < (offset + limit)) && q->next())
      {
        if(counter >= offset)
        {
          const std::vector<Match>& m = q->getCurrent();
          std::stringstream matchDesc;
          for(size_t i = 0; i < m.size(); i++)
          {
            const Match& n = m[i];


            if(!db.getNodeName(n.node).empty())
            {
              if(n.anno.ns != 0 && n.anno.name != 0
                 && n.anno.ns != db.getNamespaceStringID() && n.anno.name != db.getNodeNameStringID())
              {
                matchDesc << db.strings.str(n.anno.ns)
                  << "::" << db.strings.str(n.anno.name)
                  << "::";
              }

              // we expect that the document path including the corpus name is included in the path
              matchDesc << "salt:/" << db.getNodePath(n.node);

              if(i < m.size()-1)
              {
               matchDesc << " ";
              }
            }
          }
          result.push_back(matchDesc.str());
        } // end if result in offset-limit range
        counter++;
      }
    }
  }

  return result;
}

void CorpusStorageManager::applyUpdate(std::string corpus, GraphUpdate &update)
{

   killBackgroundWriter(corpus);

   if(!update.isConsistent())
   {
      // Always mark the update state as consistent, even if caller forgot this.
      update.finish();
   }

   const bf::path corpusPath = bf::path(databaseDir) / corpus;

   // we have to make sure that the corpus is fully loaded (with all components) before we can apply the update.
   std::shared_ptr<DBLoader> loader = getCorpusFromCache(corpus);

   if(loader)
   {
      boost::lock_guard<DBLoader> lock(*loader);

      DB& db = loader->getFullyLoaded();
      try {

         db.update(update);

         // if successfull write log
         bf::create_directories(corpusPath / "current");
         std::ofstream logStream((corpusPath / "current" / "update_log.cereal").string());
         cereal::BinaryOutputArchive ar(logStream);
         ar(update);

         // Until now only the write log is persisted. Start a background thread that writes the whole
         // corpus to the folder (without the need to apply the write log).
         startBackgroundWriter(corpus, loader);

      } catch (...)
      {
         db.load(corpusPath.string());
      }
   }
}


std::vector<annis::api::Node> CorpusStorageManager::subgraph(std::string corpus, std::vector<std::string> nodeIDs, int ctxLeft, int ctxRight)
{
  std::shared_ptr<DBLoader> loader = getCorpusFromCache(corpus);

  std::vector<Node> nodes;

  if(loader)
  {
    boost::upgrade_lock<DBLoader> lock(*loader);

    DB& db = loader->get();
    if(!db.allGraphStoragesLoaded())
    {
      boost::upgrade_to_unique_lock<DBLoader> uniqueLock(lock);
      db.ensureAllComponentsLoaded();
    }

    std::vector<std::shared_ptr<SingleAlternativeQuery>> alts;

    // find all nodes covering the same token
    for(std::string sourceNodeID : nodeIDs)
    {
      if(boost::starts_with(sourceNodeID, "salt:/"))
      {
        // remove the "salt:/" prefix
        sourceNodeID = sourceNodeID.substr(6);
      }

      auto splitted = DB::splitNodePath(sourceNodeID);
      std::string sourceContainer = splitted.first;
      std::string sourceNodeName = splitted.second;

      // left context
      {
        std::shared_ptr<SingleAlternativeQuery> qLeft = std::make_shared<SingleAlternativeQuery>(db);
        size_t nIdx = qLeft->addNode(std::make_shared<ExactAnnoValueSearch>(db, annis_ns, annis_node_name, sourceNodeName));

        size_t nContainerIdx = qLeft->addNode(std::make_shared<ExactAnnoValueSearch>(db, annis_ns, annis_node_container, sourceContainer));
        size_t tokCoveredIdx = qLeft->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_tok));
        size_t tokPrecedenceIdx = qLeft->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_tok));
        size_t anyNodeIdx = qLeft->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_node_name));

        qLeft->addOperator(std::make_shared<PartOfSubCorpus>(db.f_getGraphStorage, db), nIdx, nContainerIdx);
        qLeft->addOperator(std::make_shared<Overlap>(db, db.f_getGraphStorage), nIdx, tokCoveredIdx);
        qLeft->addOperator(std::make_shared<Precedence>(db, db.f_getGraphStorage, 0, ctxLeft), tokPrecedenceIdx, tokCoveredIdx);
        qLeft->addOperator(std::make_shared<Overlap>(db, db.f_getGraphStorage), tokPrecedenceIdx, anyNodeIdx);

        alts.push_back(qLeft);
      }

      // right context
      {
        std::shared_ptr<SingleAlternativeQuery> qRight = std::make_shared<SingleAlternativeQuery>(db);
        size_t nIdx = qRight->addNode(std::make_shared<ExactAnnoValueSearch>(db, annis_ns, annis_node_name, sourceNodeName));
        size_t nContainerIdx = qRight->addNode(std::make_shared<ExactAnnoValueSearch>(db, annis_ns, annis_node_container, sourceContainer));
        size_t tokCoveredIdx = qRight->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_tok));
        size_t tokPrecedenceIdx = qRight->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_tok));
        size_t anyNodeIdx = qRight->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_node_name));

        qRight->addOperator(std::make_shared<PartOfSubCorpus>(db.f_getGraphStorage, db), nIdx, nContainerIdx);
        qRight->addOperator(std::make_shared<Overlap>(db, db.f_getGraphStorage), nIdx, tokCoveredIdx);
        qRight->addOperator(std::make_shared<Precedence>(db, db.f_getGraphStorage, 0, ctxRight), tokCoveredIdx, tokPrecedenceIdx);
        qRight->addOperator(std::make_shared<Overlap>(db, db.f_getGraphStorage), tokPrecedenceIdx, anyNodeIdx);

        alts.push_back(qRight);
      }
    }

    Query queryAny(alts);

    std::vector<Component> components = db.getAllComponents();

    // We have to keep our own unique set because the query will return "duplicates" whenever the other parts of the
    // match vector differ.
    btree::btree_set<Match> matchResult;

    // create the subgraph description
    while(queryAny.next())
    {
      const Match& m = queryAny.getCurrent()[4];

      if(matchResult.find(m) == matchResult.end())
      {
        matchResult.insert(m);

        Node newNode = createSubgraphNode(m.node, db, components);

        nodes.emplace_back(std::move(newNode));
      }

    } // end for each given node ID
  }

  return nodes;
}

std::vector<Node> CorpusStorageManager::subcorpusGraph(std::string corpus, std::vector<std::string> corpusIDs)
{
  std::shared_ptr<DBLoader> loader = getCorpusFromCache(corpus);

  std::vector<Node> nodes;

  if(loader)
  {
    boost::upgrade_lock<DBLoader> lock(*loader);

    DB& db = loader->get();
    if(!db.allGraphStoragesLoaded())
    {
      boost::upgrade_to_unique_lock<DBLoader> uniqueLock(lock);
      db.ensureAllComponentsLoaded();
    }

    std::vector<std::shared_ptr<SingleAlternativeQuery>> alts;

    // find all nodes that a connected with the corpus IDs
    for(std::string sourceCorpusID : corpusIDs)
    {
      if(boost::starts_with(sourceCorpusID, "salt:/"))
      {
        // remove the "salt:/" prefix
        sourceCorpusID = sourceCorpusID.substr(6);
      }
      {
        std::shared_ptr<SingleAlternativeQuery> q = std::make_shared<SingleAlternativeQuery>(db);
        size_t containerIdx = q->addNode(std::make_shared<ExactAnnoValueSearch>(db, annis_ns, annis_node_container, sourceCorpusID));
        size_t anyNodeIdx = q->addNode(std::make_shared<ExactAnnoKeySearch>(db, annis_ns, annis_node_name));

        q->addOperator(std::make_shared<PartOfSubCorpus>(db.f_getGraphStorage, db), anyNodeIdx, containerIdx);


        alts.push_back(q);
      }
    }

    Query queryAny(alts);

    std::vector<Component> components = db.getAllComponents();

    // We have to keep our own unique set because the query will return "duplicates" whenever the other parts of the
    // match vector differ.
    btree::btree_set<Match> matchResult;

    // create the subgraph description
    while(queryAny.next())
    {
      const Match& m = queryAny.getCurrent()[1];

      if(matchResult.find(m) == matchResult.end())
      {
        matchResult.insert(m);

        Node newNode = createSubgraphNode(m.node, db, components);

        nodes.emplace_back(std::move(newNode));
      }

    } // end for each given node ID
  }

  return nodes;
}

std::vector<std::string> CorpusStorageManager::list()
{
  std::vector<std::string> result;

  bf::path root(databaseDir);

  if(bf::is_directory(root))
  {
    for(bf::directory_iterator it(root); it != bf::directory_iterator(); ++it)
    {
      if(bf::is_directory(it->status()))
      {
        bf::path corpusPath = it->path();
        result.push_back(corpusPath.filename().string());
      }
    }
  }
  return result;
}

void CorpusStorageManager::importCorpus(std::string pathToCorpus, std::string newCorpusName)
{

   // load an existing corpus or create a our common database directory
   std::shared_ptr<DBLoader> loader = getCorpusFromCache(newCorpusName);
   if(loader)
   {
      boost::lock_guard<DBLoader> lock(*loader);
      DB& db = loader->get();
      // load the corpus data from the external location
      db.load(pathToCorpus);
      // make sure the corpus is properly saved at least once (so it is in a consistent state)
      db.save((bf::path(databaseDir) / newCorpusName).string());
   }
}

void CorpusStorageManager::exportCorpus(std::string corpusName, std::string exportPath)
{
  std::shared_ptr<DBLoader> loader = getCorpusFromCache(corpusName);
  if(loader)
  {
     boost::unique_lock<DBLoader> lock(*loader);
     // load the corpus data from the external location
     loader->getFullyLoaded().save(exportPath);
  }
}

void CorpusStorageManager::importRelANNIS(std::string pathToCorpus, std::string newCorpusName)
{
  std::shared_ptr<DBLoader> loader = getCorpusFromCache(newCorpusName);
  if(loader)
  {
    boost::unique_lock<DBLoader> lock(*loader);

    DB& db = loader->get();

    RelANNISLoader::loadRelANNIS(db, pathToCorpus);
    // make sure the corpus is properly saved at least once (so it is in a consistent state)
    db.save((bf::path(databaseDir) / newCorpusName).string());
  }
}

bool CorpusStorageManager::deleteCorpus(std::string corpusName)
{
  bf::path root(databaseDir);
  bf::path corpusPath  = root / corpusName;

  // This will block until the internal map is available, thus do this before locking the database to avoid any deadlock
  killBackgroundWriter(corpusPath.string());

  // Get the DB and hold a lock on it until we are finished.
  // Preloading all components so we are able to restore the complete DB if anything goes wrong.
  std::shared_ptr<DBLoader> loader = getCorpusFromCache(corpusPath.string());
  if(loader)
  {

    boost::lock_guard<DBLoader> lock(*loader);

    DB& db = loader->getFullyLoaded();

    try
    {
      // delete the corpus on the disk first, if we are interrupted the data is still in memory and can be restored
      bf::remove_all(corpusPath);
    }
    catch(...)
    {
      // if anything goes wrong write the corpus back to it's original location to have a consistent state
      db.save(corpusPath.string());

      return false;
    }


    // delete the corpus from the cache and thus from memory
    std::lock_guard<std::mutex> lockCorpusCache(mutex_corpusCache);
    corpusCache.erase(corpusName);

    return true;

  }
  return false;
}

CorpusStorageManager::CorpusInfo CorpusStorageManager::info(std::string corpusName)
{
  std::lock_guard<std::mutex> lock(mutex_corpusCache);

  CorpusInfo result;
  result.loadStatus = "NOT_IN_CACHE";
  result.memoryUsageInBytes = 0;

  auto it = corpusCache.find(corpusName);
  if(it != corpusCache.end())
  {
    std::shared_ptr<DBLoader> loader = it->second;
    boost::shared_lock_guard<DBLoader> lockDB(*loader);

    result.loadStatus = loader->statusString();
    result.memoryUsageInBytes = loader->estimateMemorySize();
  }
  return result;
}

void CorpusStorageManager::startBackgroundWriter(std::string corpus, std::shared_ptr<DBLoader> loader)
{
  bf::path root = bf::path(databaseDir) / corpus;

  std::lock_guard<std::mutex> lock(mutex_writerThreads);
  writerThreads[corpus] = boost::thread([loader, root] () {

    // Get a write-lock for the database. The thread is started from another function which will have the database locked,
    // thus this thread will only really start as soon as the calling function has returned.
    boost::unique_lock<DBLoader> lock(*loader);

    // We could have been interrupted right after we waited for the lock, so check here just to be sure.
    boost::this_thread::interruption_point();


    DB& db = loader->getFullyLoaded();

    boost::this_thread::interruption_point();

    // Move the old corpus to the backup sub-folder. When the corpus is loaded again and there is backup folder
    // the backup will be used instead of the original possible corrupted files.
    // The current version is only the real one if no backup folder exists. If there is a backup folder
    // there is nothing to do since the backup already contains the last consistent version.
    // A sub-folder is used to ensure that all directories are on the same file system and moving (instead of copying)
    // is possible.
    if(!bf::exists(root / "backup"))
    {
      bf::rename(root / "current", root / "backup");
    }

    boost::this_thread::interruption_point();

    // Save the complete corpus without the write log to the target location
    db.save(root.string());

    boost::this_thread::interruption_point();

    // remove the backup folder (since the new folder was completly written)
    bf::remove_all(root / "backup");

  });

}

void CorpusStorageManager::killBackgroundWriter(std::string corpus)
{
  std::lock_guard<std::mutex> lock(mutex_writerThreads);
  auto itThread = writerThreads.find(corpus);
  if(itThread != writerThreads.end())
  {
    itThread->second.interrupt();

    // wait until thread is finished
    itThread->second.join();

    writerThreads.erase(itThread);
  }
}

std::shared_ptr<DBLoader> CorpusStorageManager::getCorpusFromCache(std::string corpusName)
{
  using SizeListEntry = std::pair<std::shared_ptr<DBLoader>, size_t>;

  std::lock_guard<std::mutex> lock(mutex_corpusCache);

  std::shared_ptr<DBLoader> result;

  auto it = corpusCache.find(corpusName);

  if(it == corpusCache.end())
  {
    // Create a new DBLoader and put it into the cache.
    // This will not load the database itself, this can be done with the resulting object from the caller
    // after it locked the DBLoader.
    result = std::make_shared<DBLoader>((bf::path(databaseDir) / corpusName).string(),
      [this, corpusName]()
      {
        // perform garbage collection whenever something was loaded
        std::lock_guard<std::mutex> lock(mutex_corpusCache);

        // Calculate size of all corpora: This temporarly locks a DB but unlocks it as soon as it can.
        // Other calls to this API might try to load corpora into memory while we are doing this, so the size
        // might not be consistent. Since these other calls will also invoke this callback later (and only one
        // callback can be executed at one time), there will always be another garbage collection run which will
        // ensure that the overall size limits are not exceeded in the end.
        size_t overallSize = 0;
        std::vector<SizeListEntry> corpusSizes;
        for(const auto& entry : corpusCache)
        {
          if(entry.first != corpusName)
          {
            std::shared_ptr<DBLoader> loader = entry.second;
            if(loader->try_lock_shared())
            {
              HL_DEBUG(logger, "Locked \"" + entry.first + "\" for garbage collection size estimation.");
              boost::shared_lock_guard<DBLoader> lock(*loader, boost::adopt_lock);
              size_t estimatedSize = entry.second->estimateMemorySize();
              overallSize += estimatedSize;
              // do not add the corpus which was just recently loaded to the list of candidates to be unloaded
              corpusSizes.push_back({entry.second, estimatedSize});
            }
            HL_DEBUG(logger, "Unlocked \"" + entry.first + "\" after garbage collection size estimation.");
          }
          else
          {
            HL_DEBUG(logger, "Can't lock \"" + entry.first + "\" for garbage collection since it is already locked by another thread.");
          }
        }

        if(overallSize <= maxAllowedCacheSize)
        {
          // there is nothing to do
          return;
        }

        // sort the corpora by their size
        std::sort(corpusSizes.begin(), corpusSizes.end(),
                  [](const SizeListEntry& lhs, const SizeListEntry& rhs)
        {
          return lhs.second < rhs.second;
        });

        // delete entries from the sorted list until the list is empty or the memory does not exceed the limit any longer
        while(!corpusSizes.empty() && overallSize > maxAllowedCacheSize)
        {
          SizeListEntry& largestCorpus = corpusSizes.back();
          if(largestCorpus.first->try_lock())
          {
            boost::lock_guard<DBLoader> lock(*(largestCorpus.first), boost::adopt_lock);
            largestCorpus.first->unload();
            overallSize -= largestCorpus.second;
          }
          corpusSizes.pop_back();
        }

      });
    corpusCache[corpusName] =  result;
  }
  else
  {
    result = it->second;
  }

  return result;
}

Node CorpusStorageManager::createSubgraphNode(uint32_t nodeID,
                                              DB& db,
                                              const std::vector<Component> &allComponents)
{

  Node newNode;
  newNode.id = nodeID;
  // add all node labels
  std::vector<Annotation> nodeAnnos = db.nodeAnnos.getAnnotations(nodeID);
  for(const Annotation& a : nodeAnnos)
  {
    newNode.labels[db.strings.str(a.ns) + "::" + db.strings.str(a.name)] = db.strings.str(a.val);
  }

  // find outgoing edges
  for(const auto&c : allComponents)
  {
    std::shared_ptr<const ReadableGraphStorage>  gs = db.getGraphStorage(c.type, c.layer, c.name);
    if(gs)
    {
      for(nodeid_t target : gs->getOutgoingEdges(nodeID))
      {
        Edge newEdge;
        newEdge.sourceID = nodeID;
        newEdge.targetID = target;

        newEdge.componentType = ComponentTypeHelper::toString(c.type);
        newEdge.componentLayer = c.layer;
        newEdge.componentName = c.name;

        for(const Annotation& a : gs->getEdgeAnnotations({nodeID, target}))
        {
          newEdge.labels[db.strings.str(a.ns) + "::" + db.strings.str(a.name)] = db.strings.str(a.val);
        }
        newNode.outgoingEdges.emplace_back(std::move(newEdge));

      }
    }
  }

  return std::move(newNode);
}
