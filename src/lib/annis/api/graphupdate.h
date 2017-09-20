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

#include <stdint.h>                      // for uint64_t

#include <memory>                        // for shared_ptr
#include <string>                        // for string
#include <vector>                        // for vector

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>  // for CEREAL_REGISTER_TYPE

namespace annis { namespace api {

enum UpdateEventType
{
  add_node, delete_node, add_node_label, delete_node_label,
  add_edge
};

struct UpdateEvent
{
  std::uint64_t changeID;
  // make this class polymorphic
  virtual ~UpdateEvent() {}

  template<class Archive>
  void serialize( Archive & ar )
  {
     ar( changeID);
  }

};


struct AddNodeEvent : UpdateEvent
{
   std::string nodePath;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), nodePath);
   }
};

struct DeleteNodeEvent : UpdateEvent
{
   std::string nodePath;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), nodePath);
   }
};

struct AddNodeLabelEvent : UpdateEvent
{
   std::string nodePath;
   std::string annoNs;
   std::string annoName;
   std::string annoValue;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), nodePath, annoNs, annoName, annoValue);
   }
};

struct DeleteNodeLabelEvent : UpdateEvent
{
   std::string nodePath;
   std::string annoNs;
   std::string annoName;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), nodePath, annoNs, annoName);
   }
};

struct AddEdgeEvent : UpdateEvent
{
   std::string sourceNode;
   std::string targetNode;
   std::string layer;
   std::string componentType;
   std::string componentName;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), sourceNode, targetNode, layer, componentType, componentName);
   }
};

struct DeleteEdgeEvent : UpdateEvent
{
   std::string sourceNode;
   std::string targetNode;
   std::string layer;
   std::string componentType;
   std::string componentName;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), sourceNode, targetNode, layer, componentType, componentName);
   }
};

struct AddEdgeLabelEvent : UpdateEvent
{
   std::string sourceNode;
   std::string targetNode;
   std::string layer;
   std::string componentType;
   std::string componentName;

   std::string annoNs;
   std::string annoName;
   std::string annoValue;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), sourceNode, targetNode, layer, componentType, componentName,
         annoNs, annoName, annoValue);
   }
};

struct DeleteEdgeLabelEvent : UpdateEvent
{
   std::string sourceNode;
   std::string targetNode;
   std::string layer;
   std::string componentType;
   std::string componentName;

   std::string annoNs;
   std::string annoName;

   template<class Archive>
   void serialize( Archive & ar )
   {
      ar(cereal::base_class<UpdateEvent>(this), sourceNode, targetNode, layer, componentType, componentName,
         annoNs, annoName);
   }
};


/**
 * @brief Lists updated that can be performed on a graph.
 *
 * This class is intended to make atomical updates to a graph (as represented by
 * the \class DB class possible.
 */
class GraphUpdate
{
public:



public:
  GraphUpdate();

  /**
   * @brief Adds an empty node with the given path to the graph.
   * If an node with this path already exists, nothing is done.
   *
   * @param path Path in the form root/sub/sub/doc#nodeName
   */
  void addNode(std::string path);

  /**
   * @brief Delete a node with the give path from the graph.
   *
   * This will delete all node labels as well. If this node does not exist, nothing is done.
   * @param name
   */
  void deleteNode(std::string path);

  /**
   * @brief Adds a label to an existing node.
   *
   * If the node does not exists or there is already a label with the same namespace and name, nothing is done.
   *
   * @param nodePath
   * @param ns The namespace of the label
   * @param name
   * @param value
   */
  void addNodeLabel(std::string nodePath, std::string ns, std::string name, std::string value);

  /**
   * @brief Delete an existing label from a node.
   *
   * If the node or the label does not exist, nothing is done.
   *
   * @param nodePath
   * @param ns
   * @param name
   */
  void deleteNodeLabel(std::string nodePath, std::string ns, std::string name);

  void addEdge(std::string sourceNode, std::string targetNode,
               std::string layer,
               std::string componentType, std::string componentName);

  void deleteEdge(std::string sourceNode, std::string targetNode,
               std::string layer,
               std::string componentType, std::string componentName);

  void addEdgeLabel(std::string sourceNode, std::string targetNode,
               std::string layer,
               std::string componentType, std::string componentName,
               std::string annoNs, std::string annoName, std::string annoValue);

  void deleteEdgeLabel(std::string sourceNode, std::string targetNode,
               std::string layer,
               std::string componentType, std::string componentName,
               std::string annoNs, std::string annoName);


  /**
   * @brief Mark the current state as consistent.
   */
  void finish();

  template<class Archive>
  void serialize(Archive & archive)
  {
    archive(diffs, lastConsistentChangeID);
  }

  const std::vector<std::shared_ptr<UpdateEvent>>& getDiffs() const
  {
     return diffs;
  }

  std::uint64_t getLastConsistentChangeID() const
  {
     return lastConsistentChangeID;
  }

  bool isConsistent() const;

private:
  std::vector<std::shared_ptr<UpdateEvent>> diffs;

  std::uint64_t lastConsistentChangeID;
};

}
}

#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>

CEREAL_REGISTER_TYPE(annis::api::AddNodeEvent);
CEREAL_REGISTER_TYPE(annis::api::DeleteNodeEvent);
CEREAL_REGISTER_TYPE(annis::api::AddNodeLabelEvent);
CEREAL_REGISTER_TYPE(annis::api::DeleteNodeLabelEvent);
CEREAL_REGISTER_TYPE(annis::api::AddEdgeEvent);
CEREAL_REGISTER_TYPE(annis::api::DeleteEdgeEvent);
CEREAL_REGISTER_TYPE(annis::api::AddEdgeLabelEvent);
CEREAL_REGISTER_TYPE(annis::api::DeleteEdgeLabelEvent);
