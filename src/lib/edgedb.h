#ifndef EDGEDB_H
#define EDGEDB_H

#include <stdlib.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "types.h"
#include "iterators.h"
#include "stringstorage.h"

namespace annis
{
class DB;

class ReadableGraphStorage
{
public:

  ReadableGraphStorage()
  {
    statistics.valid = false;
  }

  virtual ~ReadableGraphStorage() {}

  virtual void copy(const DB& db, const ReadableGraphStorage& orig) = 0;

  virtual void clear() = 0;

  virtual bool isConnected(const Edge& edge, unsigned int minDistance = 1, unsigned int maxDistance = 1) const = 0;
  /**
   * @brief Returns a newly allocated iterator for the connected nodes.
   * @param sourceNode
   * @param minDistance
   * @param maxDistance
   * @return An iterator.
   */
  virtual std::unique_ptr<EdgeIterator> findConnected(
                                           nodeid_t sourceNode,
                                           unsigned int minDistance = 1,
                                           unsigned int maxDistance = 1) const = 0;

  virtual int distance(const Edge& edge) const = 0;

  virtual std::vector<Annotation> getEdgeAnnotations(const Edge& edge) const = 0;
  virtual std::vector<nodeid_t> getOutgoingEdges(nodeid_t node) const = 0;
  virtual std::vector<nodeid_t> getIncomingEdges(nodeid_t node) const = 0;

  virtual bool load(std::string dirPath) = 0;
  virtual bool save(std::string dirPath) = 0;

  virtual std::uint32_t numberOfEdges() const = 0;
  virtual std::uint32_t numberOfEdgeAnnotations() const = 0;

  virtual GraphStatistic getStatistics() const
  {
    return statistics;
  }

  virtual void calculateStatistics() {}

protected:
  GraphStatistic statistics;
};

class EdgeDB : public ReadableGraphStorage
{
public:

  virtual ~EdgeDB() {}

  virtual void addEdge(const Edge& edge) = 0;
  virtual void addEdgeAnnotation(const Edge& edge, const Annotation& anno) = 0;

  virtual void calculateIndex() {}
};


} // end namespace annis
#endif // EDGEDB_H
