//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/io/types.hpp>
#include <boost/url/url_view.hpp>
#include <string>

namespace wfc{

/**
 * @brief Подмешивание GET-параметров в JSON-RPC запрос
 * @details params трактуется как JSON-объект без полной десериализации прикладных структур
 */
class jsonrpc_query_merge
{
public:
  typedef iow::io::data_type data_type;

  // Собрать JSON-RPC из query (пустое тело POST или чистый GET)
  data_type make_from_query(const boost::urls::url_view& query) const;

  // Подмешать query-параметры в params существующего JSON-RPC тела
  bool merge_into_body(const boost::urls::url_view& query, data_type* body) const;
};

}

