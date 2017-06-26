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

#include <annis/json/json.h>            // for Value
#include <annis/queryconfig.h>          // for QueryConfig
#include <stddef.h>                     // for size_t
#include <stdint.h>                     // for uint64_t
#include <boost/optional/optional.hpp>  // for optional
#include <iosfwd>                       // for istream
#include <map>                          // for map
#include <memory>                       // for shared_ptr
#include <vector>
#include <string>                       // for string
#include <annis/types.h>                // for Annotation
#include <annis/db.h>
#include <annis/dbloader.h>

#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/lock_types.hpp>

namespace annis { class DB; }
namespace annis { class Query; }
namespace annis { class SingleAlternativeQuery; }


namespace annis {

  class JSONQueryParser {
  public:
    JSONQueryParser();
    JSONQueryParser(const JSONQueryParser& orig) = delete;
    JSONQueryParser &operator=(const JSONQueryParser&) = delete;

    static std::shared_ptr<Query> parse(DB& db, std::istream& json, const QueryConfig config=QueryConfig());

    static std::shared_ptr<Query> parse(const DB& db, DB::GetGSFuncT getGraphStorageFunc,
                                        DB::GetAllGSFuncT getAllGraphStorageFunc,
                                        std::istream& json, const QueryConfig config=QueryConfig());

    static std::shared_ptr<Query> parseWithUpgradeableLock(DB& db,
                                                           std::string queryAsJSON,
                                                           boost::upgrade_lock<DBLoader>& lock,
                                                           const QueryConfig config=QueryConfig());

    virtual ~JSONQueryParser();
  private:
    
    static size_t parseNode(const DB& db, const Json::Value node, DB::GetGSFuncT getGraphStorageFunc,
                            std::shared_ptr<SingleAlternativeQuery>);
    static size_t addNodeAnnotation(const DB& db,
        std::shared_ptr<SingleAlternativeQuery> q,
        boost::optional<std::string> ns,
        boost::optional<std::string> name,
        boost::optional<std::string> value,
        boost::optional<std::string> textMatching,
        bool wrapEmptyAnno = false);
    
    static void parseJoin(const DB& db,
                          DB::GetGSFuncT getGraphStorageFunc,
                          DB::GetAllGSFuncT getAllGraphStorageFunc,
                          const Json::Value join,
      std::shared_ptr<annis::SingleAlternativeQuery> q, const  std::map<std::uint64_t, size_t>& nodeIdToPos);
    
    static boost::optional<std::string> optStr(const Json::Value& val) {
      if(val.isString()) {
        return boost::optional<std::string>(val.asString());
      } else {
        return boost::optional<std::string>();
      }
    }
    
    static boost::optional<std::string> optStr(const std::string& val) {
      return boost::optional<std::string>(val);
    }
    
    static boost::optional<std::string> optStr() {
      return boost::optional<std::string>();
    }
    
    static Annotation getEdgeAnno(const DB& db, const Json::Value& edgeAnno);
    
    static bool canReplaceRegex(const std::string& str);
    
  };

}


