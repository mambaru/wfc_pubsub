#pragma once

#include <wjrpc/errors.hpp>
#include <iow/io/types.hpp>
#include <string>

namespace wfc{ namespace detail{

inline bool json_rpc_is_bad_gateway(const iow::io::data_type& d)
{
  if ( d.size() < 12 )
    return false;

  const std::string body(d.begin(), d.end());
  if ( body.find("\"error\"") == std::string::npos )
    return false;

  const std::string code = std::to_string(
    static_cast<wjrpc::error_code_t>(wjrpc::error_codes::BadGateway)
  );
  return body.find("\"code\":" + code) != std::string::npos
      || body.find("\"code\": " + code) != std::string::npos;
}

}}
