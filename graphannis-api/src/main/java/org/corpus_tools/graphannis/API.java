// Targeted by JavaCPP version 1.3.2: DO NOT EDIT THIS FILE

package org.corpus_tools.graphannis;

import java.nio.*;
import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;

public class API extends org.corpus_tools.graphannis.info.AnnisApiInfo {
    static { Loader.load(); }

@Name("std::map<std::string,std::string>") public static class StringMap extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public StringMap(Pointer p) { super(p); }
    public StringMap()       { allocate();  }
    private native void allocate();
    public native @Name("operator=") @ByRef StringMap put(@ByRef StringMap x);

    public native long size();

    @Index public native @StdString BytePointer get(@StdString BytePointer i);
    public native StringMap put(@StdString BytePointer i, BytePointer value);
    @ValueSetter @Index public native StringMap put(@StdString BytePointer i, @StdString String value);

    public native @ByVal Iterator begin();
    public native @ByVal Iterator end();
    @NoOffset @Name("iterator") public static class Iterator extends Pointer {
        public Iterator(Pointer p) { super(p); }
        public Iterator() { }

        public native @Name("operator++") @ByRef Iterator increment();
        public native @Name("operator==") boolean equals(@ByRef Iterator it);
        public native @Name("operator*().first") @MemberGetter @StdString BytePointer first();
        public native @Name("operator*().second") @MemberGetter @StdString BytePointer second();
    }
}

@Name("std::vector<std::string>") public static class StringVector extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public StringVector(Pointer p) { super(p); }
    public StringVector(BytePointer ... array) { this(array.length); put(array); }
    public StringVector(String ... array) { this(array.length); put(array); }
    public StringVector()       { allocate();  }
    public StringVector(long n) { allocate(n); }
    private native void allocate();
    private native void allocate(@Cast("size_t") long n);
    public native @Name("operator=") @ByRef StringVector put(@ByRef StringVector x);

    public native long size();
    public native void resize(@Cast("size_t") long n);

    @Index public native @StdString BytePointer get(@Cast("size_t") long i);
    public native StringVector put(@Cast("size_t") long i, BytePointer value);
    @ValueSetter @Index public native StringVector put(@Cast("size_t") long i, @StdString String value);

    public StringVector put(BytePointer ... array) {
        if (size() != array.length) { resize(array.length); }
        for (int i = 0; i < array.length; i++) {
            put(i, array[i]);
        }
        return this;
    }

    public StringVector put(String ... array) {
        if (size() != array.length) { resize(array.length); }
        for (int i = 0; i < array.length; i++) {
            put(i, array[i]);
        }
        return this;
    }
}

@Name("std::vector<annis::api::Node>") public static class NodeVector extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public NodeVector(Pointer p) { super(p); }
    public NodeVector(Node ... array) { this(array.length); put(array); }
    public NodeVector()       { allocate();  }
    public NodeVector(long n) { allocate(n); }
    private native void allocate();
    private native void allocate(@Cast("size_t") long n);
    public native @Name("operator=") @ByRef NodeVector put(@ByRef NodeVector x);

    public native long size();
    public native void resize(@Cast("size_t") long n);

    @Index public native @ByRef Node get(@Cast("size_t") long i);
    public native NodeVector put(@Cast("size_t") long i, Node value);

    public NodeVector put(Node ... array) {
        if (size() != array.length) { resize(array.length); }
        for (int i = 0; i < array.length; i++) {
            put(i, array[i]);
        }
        return this;
    }
}

@Name("std::vector<annis::api::Edge>") public static class EdgeVector extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public EdgeVector(Pointer p) { super(p); }
    public EdgeVector(Edge ... array) { this(array.length); put(array); }
    public EdgeVector()       { allocate();  }
    public EdgeVector(long n) { allocate(n); }
    private native void allocate();
    private native void allocate(@Cast("size_t") long n);
    public native @Name("operator=") @ByRef EdgeVector put(@ByRef EdgeVector x);

    public native long size();
    public native void resize(@Cast("size_t") long n);

    @Index public native @ByRef Edge get(@Cast("size_t") long i);
    public native EdgeVector put(@Cast("size_t") long i, Edge value);

    public EdgeVector put(Edge ... array) {
        if (size() != array.length) { resize(array.length); }
        for (int i = 0; i < array.length; i++) {
            put(i, array[i]);
        }
        return this;
    }
}

@Name("std::vector<annis::api::UpdateEvent>") public static class UpdateEventList extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public UpdateEventList(Pointer p) { super(p); }
    public UpdateEventList(UpdateEvent ... array) { this(array.length); put(array); }
    public UpdateEventList()       { allocate();  }
    public UpdateEventList(long n) { allocate(n); }
    private native void allocate();
    private native void allocate(@Cast("size_t") long n);
    public native @Name("operator=") @ByRef UpdateEventList put(@ByRef UpdateEventList x);

    public native long size();
    public native void resize(@Cast("size_t") long n);

    @Index public native @ByRef UpdateEvent get(@Cast("size_t") long i);
    public native UpdateEventList put(@Cast("size_t") long i, UpdateEvent value);

    public UpdateEventList put(UpdateEvent ... array) {
        if (size() != array.length) { resize(array.length); }
        for (int i = 0; i < array.length; i++) {
            put(i, array[i]);
        }
        return this;
    }
}

// Parsed from annis/api/corpusstoragemanager.h

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

// #pragma once

// #include <annis/api/graphupdate.h>
// #include <annis/api/graph.h>

// #include <stddef.h>                        // for size_t
// #include <map>                             // for map
// #include <memory>                          // for shared_ptr
// #include <mutex>                           // for mutex
// #include <string>                          // for string
// #include <vector>                          // for vector


@Namespace("annis") @Opaque public static class DBLoader extends Pointer {
    /** Empty constructor. Calls {@code super((Pointer)null)}. */
    public DBLoader() { super((Pointer)null); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public DBLoader(Pointer p) { super(p); }
} 
@Namespace("annis") @Opaque public static class DB extends Pointer {
    /** Empty constructor. Calls {@code super((Pointer)null)}. */
    public DB() { super((Pointer)null); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public DB(Pointer p) { super(p); }
} 
@Namespace("annis") @Opaque public static class Component extends Pointer {
    /** Empty constructor. Calls {@code super((Pointer)null)}. */
    public Component() { super((Pointer)null); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public Component(Pointer p) { super(p); }
}   

@Namespace("boost") @Opaque public static class thread extends Pointer {
    /** Empty constructor. Calls {@code super((Pointer)null)}. */
    public thread() { super((Pointer)null); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public thread(Pointer p) { super(p); }
}
/**
 * An API for managing corpora stored in a common location on the file system.
 */
@Namespace("annis::api") @NoOffset public static class CorpusStorageManager extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public CorpusStorageManager(Pointer p) { super(p); }


  public static class CountResult extends Pointer {
      static { Loader.load(); }
      /** Default native constructor. */
      public CountResult() { super((Pointer)null); allocate(); }
      /** Native array allocator. Access with {@link Pointer#position(long)}. */
      public CountResult(long size) { super((Pointer)null); allocateArray(size); }
      /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
      public CountResult(Pointer p) { super(p); }
      private native void allocate();
      private native void allocateArray(long size);
      @Override public CountResult position(long position) {
          return (CountResult)super.position(position);
      }
  
    public native long matchCount(); public native CountResult matchCount(long matchCount);
    public native long documentCount(); public native CountResult documentCount(long documentCount);
  }

  public static class CorpusInfo extends Pointer {
      static { Loader.load(); }
      /** Default native constructor. */
      public CorpusInfo() { super((Pointer)null); allocate(); }
      /** Native array allocator. Access with {@link Pointer#position(long)}. */
      public CorpusInfo(long size) { super((Pointer)null); allocateArray(size); }
      /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
      public CorpusInfo(Pointer p) { super(p); }
      private native void allocate();
      private native void allocateArray(long size);
      @Override public CorpusInfo position(long position) {
          return (CorpusInfo)super.position(position);
      }
  
    public native @StdString BytePointer loadStatus(); public native CorpusInfo loadStatus(BytePointer loadStatus);
    public native long memoryUsageInBytes(); public native CorpusInfo memoryUsageInBytes(long memoryUsageInBytes);
  }

  public CorpusStorageManager(@StdString BytePointer databaseDir, @Cast("size_t") long maxAllowedCacheSize/*=1073741824*/) { super((Pointer)null); allocate(databaseDir, maxAllowedCacheSize); }
  private native void allocate(@StdString BytePointer databaseDir, @Cast("size_t") long maxAllowedCacheSize/*=1073741824*/);
  public CorpusStorageManager(@StdString BytePointer databaseDir) { super((Pointer)null); allocate(databaseDir); }
  private native void allocate(@StdString BytePointer databaseDir);
  public CorpusStorageManager(@StdString String databaseDir, @Cast("size_t") long maxAllowedCacheSize/*=1073741824*/) { super((Pointer)null); allocate(databaseDir, maxAllowedCacheSize); }
  private native void allocate(@StdString String databaseDir, @Cast("size_t") long maxAllowedCacheSize/*=1073741824*/);
  public CorpusStorageManager(@StdString String databaseDir) { super((Pointer)null); allocate(databaseDir); }
  private native void allocate(@StdString String databaseDir);

  /**
   * Count all occurrences of an AQL query in a single corpus.
   *
   * @param corpus
   * @param queryAsJSON
   * @return
   */
  public native long count(@ByVal StringVector corpora,
                    @StdString BytePointer queryAsJSON);
  public native long count(@ByVal StringVector corpora,
                    @StdString String queryAsJSON);


  /**
   * Count all occurrences of an AQL query in a single corpus.
   *
   * @param corpus
   * @param queryAsJSON
   * @return
   */
  public native @ByVal CountResult countExtra(@ByVal StringVector corpora,
                    @StdString BytePointer queryAsJSON);
  public native @ByVal CountResult countExtra(@ByVal StringVector corpora,
                    @StdString String queryAsJSON);


  /**
   * Find occurrences of an AQL query in a single corpus.
   * @param corpora
   * @param queryAsJSON
   * @param offset
   * @param limit
   * @return
   */
  public native @ByVal StringVector find(@ByVal StringVector corpora, @StdString BytePointer queryAsJSON, long offset/*=0*/,
                                  long limit/*=0*/);
  public native @ByVal StringVector find(@ByVal StringVector corpora, @StdString BytePointer queryAsJSON);
  public native @ByVal StringVector find(@ByVal StringVector corpora, @StdString String queryAsJSON, long offset/*=0*/,
                                  long limit/*=0*/);
  public native @ByVal StringVector find(@ByVal StringVector corpora, @StdString String queryAsJSON);

  public native void applyUpdate(@StdString BytePointer corpus, @ByRef GraphUpdate update);
  public native void applyUpdate(@StdString String corpus, @ByRef GraphUpdate update);

  /**
   * \brief Return a sub-graph consisting of the nodes given as argument and all nodes that cover the same token.
   * @param corpus
   * @param nodeIDs The IDs/names of the nodes to include.
   * @param ctxLeft Left token context
   * @param ctxRight Right token context
   * @return
   */
  public native @ByVal NodeVector subgraph(@StdString BytePointer corpus, @ByVal StringVector nodeIDs, int ctxLeft, int ctxRight);
  public native @ByVal NodeVector subgraph(@StdString String corpus, @ByVal StringVector nodeIDs, int ctxLeft, int ctxRight);

  public native @ByVal NodeVector subcorpusGraph(@StdString BytePointer corpus, @ByVal StringVector corpusIDs);
  public native @ByVal NodeVector subcorpusGraph(@StdString String corpus, @ByVal StringVector corpusIDs);

  /**
   * \brief Lists the name of all corpora.
   * @return
   */
  public native @ByVal StringVector list();

  public native void importCorpus(@StdString BytePointer pathToCorpus, @StdString BytePointer newCorpusName);
  public native void importCorpus(@StdString String pathToCorpus, @StdString String newCorpusName);
  public native void exportCorpus(@StdString BytePointer corpusName, @StdString BytePointer exportPath);
  public native void exportCorpus(@StdString String corpusName, @StdString String exportPath);

  public native void importRelANNIS(@StdString BytePointer pathToCorpus, @StdString BytePointer newCorpusName);
  public native void importRelANNIS(@StdString String pathToCorpus, @StdString String newCorpusName);

  public native @Cast("bool") boolean deleteCorpus(@StdString BytePointer corpusName);
  public native @Cast("bool") boolean deleteCorpus(@StdString String corpusName);

  public native @ByVal CorpusInfo info(@StdString BytePointer corpusName);
  public native @ByVal CorpusInfo info(@StdString String corpusName);

}

 // end namespace annis


// Parsed from annis/api/admin.h

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

// #pragma once

// #include <string>
  @Namespace("annis::api") public static class Admin extends Pointer {
      static { Loader.load(); }
      /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
      public Admin(Pointer p) { super(p); }
      /** Native array allocator. Access with {@link Pointer#position(long)}. */
      public Admin(long size) { super((Pointer)null); allocateArray(size); }
      private native void allocateArray(long size);
      @Override public Admin position(long position) {
          return (Admin)super.position(position);
      }
  
    public Admin() { super((Pointer)null); allocate(); }
    private native void allocate();

    /**
    * Imports data in the relANNIS format to the internal format used by graphANNIS.
    * @param sourceFolder
    * @param targetFolder
    */
   public static native void importRelANNIS(@StdString BytePointer sourceFolder, @StdString BytePointer targetFolder);
   public static native void importRelANNIS(@StdString String sourceFolder, @StdString String targetFolder);
  }
 // end namespace annis::api


// Parsed from annis/api/graphupdate.h

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

// #pragma once

// #include <stdint.h>                      // for uint64_t

// #include <memory>                        // for shared_ptr
// #include <string>                        // for string
// #include <vector>                        // for vector

// #include <cereal/types/string.hpp>
// #include <cereal/types/vector.hpp>
// #include <cereal/types/polymorphic.hpp>  // for CEREAL_REGISTER_TYPE

/** enum annis::api::UpdateEventType */
public static final int
  add_node = 0, delete_node = 1, add_node_label = 2, delete_node_label = 3,
  add_edge = 4;

@Namespace("annis::api") public static class UpdateEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public UpdateEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public UpdateEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public UpdateEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public UpdateEvent position(long position) {
        return (UpdateEvent)super.position(position);
    }

  public native long changeID(); public native UpdateEvent changeID(long changeID);
  // make this class polymorphic

}


@Namespace("annis::api") @NoOffset public static class AddNodeEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public AddNodeEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public AddNodeEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public AddNodeEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public AddNodeEvent position(long position) {
        return (AddNodeEvent)super.position(position);
    }

   public native @StdString BytePointer nodePath(); public native AddNodeEvent nodePath(BytePointer nodePath);
   public native @StdString BytePointer nodeType(); public native AddNodeEvent nodeType(BytePointer nodeType);
}

@Namespace("annis::api") @NoOffset public static class DeleteNodeEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public DeleteNodeEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public DeleteNodeEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public DeleteNodeEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public DeleteNodeEvent position(long position) {
        return (DeleteNodeEvent)super.position(position);
    }

   public native @StdString BytePointer nodePath(); public native DeleteNodeEvent nodePath(BytePointer nodePath);
}

@Namespace("annis::api") @NoOffset public static class AddNodeLabelEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public AddNodeLabelEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public AddNodeLabelEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public AddNodeLabelEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public AddNodeLabelEvent position(long position) {
        return (AddNodeLabelEvent)super.position(position);
    }

   public native @StdString BytePointer nodePath(); public native AddNodeLabelEvent nodePath(BytePointer nodePath);
   public native @StdString BytePointer annoNs(); public native AddNodeLabelEvent annoNs(BytePointer annoNs);
   public native @StdString BytePointer annoName(); public native AddNodeLabelEvent annoName(BytePointer annoName);
   public native @StdString BytePointer annoValue(); public native AddNodeLabelEvent annoValue(BytePointer annoValue);
}

@Namespace("annis::api") @NoOffset public static class DeleteNodeLabelEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public DeleteNodeLabelEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public DeleteNodeLabelEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public DeleteNodeLabelEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public DeleteNodeLabelEvent position(long position) {
        return (DeleteNodeLabelEvent)super.position(position);
    }

   public native @StdString BytePointer nodePath(); public native DeleteNodeLabelEvent nodePath(BytePointer nodePath);
   public native @StdString BytePointer annoNs(); public native DeleteNodeLabelEvent annoNs(BytePointer annoNs);
   public native @StdString BytePointer annoName(); public native DeleteNodeLabelEvent annoName(BytePointer annoName);
}

@Namespace("annis::api") @NoOffset public static class AddEdgeEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public AddEdgeEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public AddEdgeEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public AddEdgeEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public AddEdgeEvent position(long position) {
        return (AddEdgeEvent)super.position(position);
    }

   public native @StdString BytePointer sourceNode(); public native AddEdgeEvent sourceNode(BytePointer sourceNode);
   public native @StdString BytePointer targetNode(); public native AddEdgeEvent targetNode(BytePointer targetNode);
   public native @StdString BytePointer layer(); public native AddEdgeEvent layer(BytePointer layer);
   public native @StdString BytePointer componentType(); public native AddEdgeEvent componentType(BytePointer componentType);
   public native @StdString BytePointer componentName(); public native AddEdgeEvent componentName(BytePointer componentName);
}

@Namespace("annis::api") @NoOffset public static class DeleteEdgeEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public DeleteEdgeEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public DeleteEdgeEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public DeleteEdgeEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public DeleteEdgeEvent position(long position) {
        return (DeleteEdgeEvent)super.position(position);
    }

   public native @StdString BytePointer sourceNode(); public native DeleteEdgeEvent sourceNode(BytePointer sourceNode);
   public native @StdString BytePointer targetNode(); public native DeleteEdgeEvent targetNode(BytePointer targetNode);
   public native @StdString BytePointer layer(); public native DeleteEdgeEvent layer(BytePointer layer);
   public native @StdString BytePointer componentType(); public native DeleteEdgeEvent componentType(BytePointer componentType);
   public native @StdString BytePointer componentName(); public native DeleteEdgeEvent componentName(BytePointer componentName);
}

@Namespace("annis::api") @NoOffset public static class AddEdgeLabelEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public AddEdgeLabelEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public AddEdgeLabelEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public AddEdgeLabelEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public AddEdgeLabelEvent position(long position) {
        return (AddEdgeLabelEvent)super.position(position);
    }

   public native @StdString BytePointer sourceNode(); public native AddEdgeLabelEvent sourceNode(BytePointer sourceNode);
   public native @StdString BytePointer targetNode(); public native AddEdgeLabelEvent targetNode(BytePointer targetNode);
   public native @StdString BytePointer layer(); public native AddEdgeLabelEvent layer(BytePointer layer);
   public native @StdString BytePointer componentType(); public native AddEdgeLabelEvent componentType(BytePointer componentType);
   public native @StdString BytePointer componentName(); public native AddEdgeLabelEvent componentName(BytePointer componentName);

   public native @StdString BytePointer annoNs(); public native AddEdgeLabelEvent annoNs(BytePointer annoNs);
   public native @StdString BytePointer annoName(); public native AddEdgeLabelEvent annoName(BytePointer annoName);
   public native @StdString BytePointer annoValue(); public native AddEdgeLabelEvent annoValue(BytePointer annoValue);
}

@Namespace("annis::api") @NoOffset public static class DeleteEdgeLabelEvent extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public DeleteEdgeLabelEvent() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public DeleteEdgeLabelEvent(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public DeleteEdgeLabelEvent(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public DeleteEdgeLabelEvent position(long position) {
        return (DeleteEdgeLabelEvent)super.position(position);
    }

   public native @StdString BytePointer sourceNode(); public native DeleteEdgeLabelEvent sourceNode(BytePointer sourceNode);
   public native @StdString BytePointer targetNode(); public native DeleteEdgeLabelEvent targetNode(BytePointer targetNode);
   public native @StdString BytePointer layer(); public native DeleteEdgeLabelEvent layer(BytePointer layer);
   public native @StdString BytePointer componentType(); public native DeleteEdgeLabelEvent componentType(BytePointer componentType);
   public native @StdString BytePointer componentName(); public native DeleteEdgeLabelEvent componentName(BytePointer componentName);

   public native @StdString BytePointer annoNs(); public native DeleteEdgeLabelEvent annoNs(BytePointer annoNs);
   public native @StdString BytePointer annoName(); public native DeleteEdgeLabelEvent annoName(BytePointer annoName);
}


/**
 * \brief Lists updated that can be performed on a graph.
 *
 * This class is intended to make atomical updates to a graph (as represented by
 * the \class DB class possible.
 */
@Namespace("annis::api") @NoOffset public static class GraphUpdate extends Pointer {
    static { Loader.load(); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public GraphUpdate(Pointer p) { super(p); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public GraphUpdate(long size) { super((Pointer)null); allocateArray(size); }
    private native void allocateArray(long size);
    @Override public GraphUpdate position(long position) {
        return (GraphUpdate)super.position(position);
    }

  public GraphUpdate() { super((Pointer)null); allocate(); }
  private native void allocate();

  /**
   * \brief Adds an empty node with the given path and type to the graph.
   * If an node with this path already exists, nothing is done.
   *
   * @param path
   * @param type The type, "node" per default.
   */
  public native void addNode(@StdString BytePointer path, @StdString BytePointer type/*="node"*/);
  public native void addNode(@StdString BytePointer path);
  public native void addNode(@StdString String path, @StdString String type/*="node"*/);
  public native void addNode(@StdString String path);

  /**
   * \brief Delete a node with the give path from the graph.
   *
   * This will delete all node labels as well. If this node does not exist, nothing is done.
   * @param name
   */
  public native void deleteNode(@StdString BytePointer path);
  public native void deleteNode(@StdString String path);

  /**
   * \brief Adds a label to an existing node.
   *
   * If the node does not exists or there is already a label with the same namespace and name, nothing is done.
   *
   * @param nodePath
   * @param ns The namespace of the label
   * @param name
   * @param value
   */
  public native void addNodeLabel(@StdString BytePointer nodePath, @StdString BytePointer ns, @StdString BytePointer name, @StdString BytePointer value);
  public native void addNodeLabel(@StdString String nodePath, @StdString String ns, @StdString String name, @StdString String value);

  /**
   * \brief Delete an existing label from a node.
   *
   * If the node or the label does not exist, nothing is done.
   *
   * @param nodePath
   * @param ns
   * @param name
   */
  public native void deleteNodeLabel(@StdString BytePointer nodePath, @StdString BytePointer ns, @StdString BytePointer name);
  public native void deleteNodeLabel(@StdString String nodePath, @StdString String ns, @StdString String name);

  public native void addEdge(@StdString BytePointer sourceNode, @StdString BytePointer targetNode,
                 @StdString BytePointer layer,
                 @StdString BytePointer componentType, @StdString BytePointer componentName);
  public native void addEdge(@StdString String sourceNode, @StdString String targetNode,
                 @StdString String layer,
                 @StdString String componentType, @StdString String componentName);

  public native void deleteEdge(@StdString BytePointer sourceNode, @StdString BytePointer targetNode,
                 @StdString BytePointer layer,
                 @StdString BytePointer componentType, @StdString BytePointer componentName);
  public native void deleteEdge(@StdString String sourceNode, @StdString String targetNode,
                 @StdString String layer,
                 @StdString String componentType, @StdString String componentName);

  public native void addEdgeLabel(@StdString BytePointer sourceNode, @StdString BytePointer targetNode,
                 @StdString BytePointer layer,
                 @StdString BytePointer componentType, @StdString BytePointer componentName,
                 @StdString BytePointer annoNs, @StdString BytePointer annoName, @StdString BytePointer annoValue);
  public native void addEdgeLabel(@StdString String sourceNode, @StdString String targetNode,
                 @StdString String layer,
                 @StdString String componentType, @StdString String componentName,
                 @StdString String annoNs, @StdString String annoName, @StdString String annoValue);

  public native void deleteEdgeLabel(@StdString BytePointer sourceNode, @StdString BytePointer targetNode,
                 @StdString BytePointer layer,
                 @StdString BytePointer componentType, @StdString BytePointer componentName,
                 @StdString BytePointer annoNs, @StdString BytePointer annoName);
  public native void deleteEdgeLabel(@StdString String sourceNode, @StdString String targetNode,
                 @StdString String layer,
                 @StdString String componentType, @StdString String componentName,
                 @StdString String annoNs, @StdString String annoName);


  /**
   * \brief Mark the current state as consistent.
   */
  public native void finish();

  

  

  public native @Cast("bool") boolean isConsistent();
}




// #include <cereal/archives/binary.hpp>
// #include <cereal/archives/xml.hpp>
// #include <cereal/archives/json.hpp>











// Parsed from annis/api/graph.h

// #pragma once

// #include <string>
// #include <vector>
// #include <map>


/**
 * \brief The Edge struct
 */
@Namespace("annis::api") public static class Edge extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public Edge() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public Edge(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public Edge(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public Edge position(long position) {
        return (Edge)super.position(position);
    }

  public native long sourceID(); public native Edge sourceID(long sourceID);
  public native long targetID(); public native Edge targetID(long targetID);

  public native @StdString BytePointer componentType(); public native Edge componentType(BytePointer componentType);
  public native @StdString BytePointer componentLayer(); public native Edge componentLayer(BytePointer componentLayer);
  public native @StdString BytePointer componentName(); public native Edge componentName(BytePointer componentName);

  /** Maps a fully qualified label name (seperated by "::") to a label value */
  public native @ByRef StringMap labels(); public native Edge labels(StringMap labels);
}


/**
 * \brief The Node struct
 */
@Namespace("annis::api") public static class Node extends Pointer {
    static { Loader.load(); }
    /** Default native constructor. */
    public Node() { super((Pointer)null); allocate(); }
    /** Native array allocator. Access with {@link Pointer#position(long)}. */
    public Node(long size) { super((Pointer)null); allocateArray(size); }
    /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
    public Node(Pointer p) { super(p); }
    private native void allocate();
    private native void allocateArray(long size);
    @Override public Node position(long position) {
        return (Node)super.position(position);
    }

  public native long id(); public native Node id(long id);
  /** Maps a fully qualified label name (seperated by "::") to a label value */
  public native @ByRef StringMap labels(); public native Node labels(StringMap labels);

  public native @ByRef EdgeVector outgoingEdges(); public native Node outgoingEdges(EdgeVector outgoingEdges);
}






}
