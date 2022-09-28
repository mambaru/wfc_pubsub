#include "merge_operator.hpp"
#include <pubsub/logger.hpp>
#include <wjson/wjson.hpp>
#include <algorithm>
#include "../api/message.hpp"
#include "../api/message_json.hpp"
#include "../message_queue.hpp"

namespace wfc{ namespace pubsub{

namespace{

void add_to_queue(message_queue& mqueue, std::vector<message>* message_list, const rocksdb::Slice& value )
{
  typedef wjson::vector_of<message_json> message_list_json;
  const char *beg = value.data();
  const char *end = beg + value.size();
  wjson::json_error je;
  message_list->clear();
  message_list_json::serializer()(*message_list, beg, end, &je);
  if ( je )
  {
    PUBSUB_LOG_ERROR("PUBSUB merge_operator::FullMergeV2 json_error: "
        << wjson::strerror::message_trace(je, beg, end)  )
    return;
  }
  for ( message& m : *message_list)
    mqueue.push( std::move(m) );
  message_list->clear();
}

}

merge_operator::merge_operator()
{
}

bool merge_operator::FullMergeV2( const MergeOperationInput& merge_in, MergeOperationOutput* merge_out) const
try
{
  typedef std::vector<message> message_list_t;
  typedef wjson::vector_of<message_json> message_list_json;
  const auto& operands = merge_in.operand_list;

  if ( operands.empty() )
    return true;

  message_queue mqueue;
  message_list_t message_list;
  message_list.reserve(16);
  if ( merge_in.existing_value != nullptr )
  {
    add_to_queue(mqueue, &message_list, *merge_in.existing_value);
  }

  for ( const auto& op : operands  )
  {
    add_to_queue(mqueue, &message_list, op);
  }

  message_list.clear();

  mqueue.remove_death( time(nullptr) );
  mqueue.copy_to(std::back_inserter(message_list));
  merge_out->new_value.clear();
  message_list_json::serializer()(message_list, std::back_inserter(merge_out->new_value));

  PUBSUB_LOG_TRACE("Merge result: " << merge_out->new_value)
  return true;
}
catch(const std::exception& e)
{
  if ( merge_in.existing_value )
    merge_out->new_value = merge_in.existing_value->ToString();

  PUBSUB_LOG_ERROR("PUBSUB merge_operator::FullMergeV2 exception: "<< e.what() << ": key="
                  << merge_in.key.ToString() << " existing="
                  << ( merge_in.existing_value ? merge_in.existing_value ->ToString() : std::string("nullptr") )
                  << " operands.size()=" << merge_in.operand_list.size() )
  return true;
}
catch(...)
{
  if ( merge_in.existing_value )
    merge_out->new_value = merge_in.existing_value->ToString();

  PUBSUB_LOG_ERROR("PUBSUB merge_operator::FullMergeV2 unhandled exception: key="
                  << merge_in.key.ToString() << " existing="
                  << ( merge_in.existing_value ? merge_in.existing_value ->ToString() : std::string("nullptr") )
                  << " operands=" << merge_in.operand_list.size() )
  return true;
}

const char* merge_operator::Name() const
{
  return "PubsubMergeOperator";
}

}}
