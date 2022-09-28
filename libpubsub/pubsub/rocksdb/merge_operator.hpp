#pragma once

#include <rocksdb/merge_operator.h>
#include <rocksdb/env.h>

namespace wfc{ namespace pubsub{

class merge_operator
  : public ::rocksdb::MergeOperator
{
public:
  typedef ::rocksdb::Slice slice_type;
  typedef std::vector<std::string> update_list;

  merge_operator();

  virtual const char* Name() const override;

  virtual bool FullMergeV2(const MergeOperationInput& merge_in,
                           MergeOperationOutput* merge_out) const override;
};

}}
