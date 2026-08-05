#include "jsonrpc_query_merge.hpp"
#include <wfc/json.hpp>

#include <map>
#include <sstream>
#include <iterator>
#include <vector>

namespace wfc{

namespace {

struct jsonrpc_envelope
{
  std::string method;
  int id = 1;
  std::string params = "{}";
};

struct jsonrpc_envelope_json
{
  JSON_NAME(method)
  JSON_NAME(id)
  JSON_NAME(params)

  typedef wjson::object<
    jsonrpc_envelope,
    wjson::member_list<
      wjson::member<n_method, jsonrpc_envelope, std::string, &jsonrpc_envelope::method>,
      wjson::member<n_id, jsonrpc_envelope, int, &jsonrpc_envelope::id>,
      wjson::member<n_params, jsonrpc_envelope, std::string, &jsonrpc_envelope::params, wjson::raw_value<> >
    >
  > meta;
  typedef meta::serializer serializer;
};

typedef std::vector<std::pair<std::string, std::string>> json_members_t;
typedef std::map<std::string, std::string> json_overlay_t;

std::string json_escape_(const std::string& s)
{
  std::string out;
  out.reserve(s.size());
  for ( char ch : s )
  {
    switch ( ch )
    {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += ch; break;
    }
  }
  return out;
}

std::string query_param_(const boost::urls::url_view& uv, const char* name)
{
  const auto params = uv.params();
  const auto itr = params.find(name);
  if ( itr == params.end() )
    return std::string();
  return (*itr).value;
}

json_overlay_t overlay_from_query_(const boost::urls::url_view& uv)
{
  json_overlay_t overlay;
  const std::string oid = query_param_(uv, "oid");
  if ( !oid.empty() )
    overlay["oid"] = oid;
  const std::string sid = query_param_(uv, "sid");
  if ( !sid.empty() )
    overlay["sid"] = "\"" + json_escape_(sid) + "\"";
  const std::string uuid = query_param_(uv, "uuid");
  if ( !uuid.empty() )
    overlay["uuid"] = "\"" + json_escape_(uuid) + "\"";
  const std::string key = query_param_(uv, "key");
  if ( !key.empty() )
    overlay["key"] = key;
  const std::string confirm = query_param_(uv, "confirm");
  if ( !confirm.empty() )
    overlay["confirm"] = (confirm == "true" || confirm == "1") ? "true" : "false";
  const std::string channels = query_param_(uv, "channels");
  if ( !channels.empty() )
    overlay["channels"] = channels;
  return overlay;
}

bool parse_json_object_members_(const std::string& src, json_members_t* members)
{
  members->clear();
  wjson::json_error e;
  std::string::const_iterator beg = src.begin();
  const std::string::const_iterator end = src.end();
  beg = wjson::parser::parse_space(beg, end, &e);
  if ( beg == end || *beg != '{' )
    return false;
  ++beg;
  beg = wjson::parser::parse_space(beg, end, &e);
  if ( beg != end && *beg == '}' )
    return true;

  for (;;)
  {
    beg = wjson::parser::parse_space(beg, end, &e);
    if ( beg == end )
      return false;
    if ( *beg == '}' )
      break;

    const std::string::const_iterator key_beg = beg;
    if ( !wjson::parser::is_string(beg, end) )
      return false;
    beg = wjson::parser::parse_string(beg, end, &e);
    if ( e )
      return false;

    std::string key;
    wjson::value<std::string>::serializer()(key, key_beg, beg, &e);
    if ( e )
      return false;

    beg = wjson::parser::parse_space(beg, end, &e);
    if ( beg == end || *beg != ':' )
      return false;
    ++beg;
    beg = wjson::parser::parse_space(beg, end, &e);
    if ( beg == end )
      return false;

    const std::string::const_iterator val_beg = beg;
    beg = wjson::parser::parse_value(beg, end, &e);
    if ( e )
      return false;

    members->push_back( std::make_pair(key, std::string(val_beg, beg)) );

    beg = wjson::parser::parse_space(beg, end, &e);
    if ( beg == end )
      return false;
    if ( *beg == '}' )
      break;
    if ( *beg != ',' )
      return false;
    ++beg;
  }
  return true;
}

std::string build_json_object_(const json_members_t& members)
{
  std::ostringstream out;
  out << "{";
  for ( size_t i = 0; i < members.size(); ++i )
  {
    if ( i != 0 )
      out << ",";
    out << "\"" << json_escape_(members[i].first) << "\":" << members[i].second;
  }
  out << "}";
  return out.str();
}

std::string patch_json_object_(const std::string& src, const json_overlay_t& overlay)
{
  if ( overlay.empty() )
    return src;

  json_members_t members;
  const std::string obj = src.empty() ? "{}" : src;
  if ( !parse_json_object_members_(obj, &members) )
    return src;

  json_members_t result;
  result.reserve(members.size() + overlay.size());
  for ( const auto& member : members )
  {
    if ( overlay.find(member.first) == overlay.end() )
      result.push_back(member);
  }
  for ( const auto& field : overlay )
    result.push_back(field);

  return build_json_object_(result);
}

} // namespace

jsonrpc_query_merge::data_type jsonrpc_query_merge::make_from_query(const boost::urls::url_view& query) const
{
  const std::string method = query_param_(query, "method");
  if ( method.empty() )
    return data_type();

  std::string id_str = query_param_(query, "id");
  if ( id_str.empty() )
    id_str = "1";

  const json_overlay_t overlay = overlay_from_query_(query);
  json_members_t members;
  members.reserve(overlay.size());
  for ( const auto& field : overlay )
    members.push_back(field);

  std::ostringstream body;
  body << "{\"jsonrpc\":\"2.0\",\"method\":\"" << json_escape_(method)
       << "\",\"params\":" << build_json_object_(members)
       << ",\"id\":" << id_str << "}";
  const std::string json = body.str();
  return data_type(json.begin(), json.end());
}

bool jsonrpc_query_merge::merge_into_body(const boost::urls::url_view& query, data_type* body) const
{
  if ( body == nullptr )
    return false;

  const json_overlay_t overlay = overlay_from_query_(query);
  if ( overlay.empty() )
    return false;

  jsonrpc_envelope env;
  wjson::json_error e;
  jsonrpc_envelope_json::serializer()( env, body->begin(), body->end(), &e );
  if ( e )
    return false;

  env.params = patch_json_object_(env.params, overlay);
  body->clear();
  jsonrpc_envelope_json::serializer()( env, std::back_inserter(*body) );
  return true;
}

}

