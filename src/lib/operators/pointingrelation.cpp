#include "pointingrelation.h"
#include "wrapper.h"

using namespace annis;

PointingRelation::PointingRelation(const DB &db, std::string ns, std::string name,
                                   unsigned int minDistance, unsigned int maxDistance)
  : db(db), ns(ns), name(name),
    minDistance(minDistance), maxDistance(maxDistance),
    anyAnno(Init::initAnnotation()), edgeAnno(anyAnno)
{
  initEdgeDB();
}

PointingRelation::PointingRelation(const DB &db, std::string ns, std::string name, const Annotation &edgeAnno)
  : db(db), ns(ns), name(name),
    minDistance(1), maxDistance(1),
    anyAnno(Init::initAnnotation()), edgeAnno(edgeAnno)
{
  initEdgeDB();
}

std::unique_ptr<AnnoIt> PointingRelation::retrieveMatches(const Match &lhs)
{
  ListWrapper* w = new ListWrapper();

  // add the rhs nodes of all of the edge storages
  for(auto e : edb)
  {
    EdgeIterator* it = e->findConnected(lhs.node, minDistance, maxDistance);
    for(auto m = it->next(); m.first; m = it->next())
    {
      if(checkEdgeAnnotation(e, lhs.node, m.second))
      {
        w->addMatch(m.second);
      }
    }
    delete it;
  }

  return std::unique_ptr<AnnoIt>(w);
}

bool PointingRelation::filter(const Match &lhs, const Match &rhs)
{
  // check if the two nodes are connected in *any* of the edge storages
  for(auto e : edb)
  {
    if(e->isConnected(Init::initEdge(lhs.node, rhs.node), minDistance, maxDistance))
    {
      if(checkEdgeAnnotation(e, lhs.node, rhs.node))
      {
        return true;
      }
    }
  }
  return false;
}


void PointingRelation::initEdgeDB()
{
  if(ns == "")
  {
    edb = db.getEdgeDB(ComponentType::POINTING, name);
  }
  else
  {
    // directly add the only known edge storage
    const EdgeDB* e = db.getEdgeDB(ComponentType::POINTING, ns, name);
    if(e != nullptr)
    {
      edb.push_back(db.getEdgeDB(ComponentType::POINTING, ns, name));
    }
  }
}

bool PointingRelation::checkEdgeAnnotation(const EdgeDB* e, nodeid_t source, nodeid_t target)
{
  if(edgeAnno == anyAnno)
  {
    return true;
  }
  else
  {
    // check if the edge has the correct annotation first
    auto edgeAnnoList = e->getEdgeAnnotations(Init::initEdge(source, target));
    for(const auto& anno : edgeAnnoList)
    {
      if(checkAnnotationEqual(edgeAnno, anno))
      {
        return true;
      }
    } // end for each annotation of candidate edge
  }
  return false;
}

PointingRelation::~PointingRelation()
{

}



