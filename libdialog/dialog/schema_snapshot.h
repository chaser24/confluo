#ifndef DIALOG_SCHEMA_SNAPSHOT_H_
#define DIALOG_SCHEMA_SNAPSHOT_H_

#include "column_snapshot.h"
#include "byte_string.h"
#include "data.h"

namespace dialog {

class schema_snapshot {
 public:
  schema_snapshot() = default;

  void add_column(const column_snapshot& snap) {
    snapshot_.push_back(snap);
  }

  void add_column(column_snapshot&& snap) {
    snapshot_.push_back(std::move(snap));
  }

  immutable_value get(void* data, uint32_t i) const {
    return immutable_value(
        snapshot_[i].type,
        reinterpret_cast<uint8_t*>(data) + snapshot_[i].offset);
  }

  byte_string get_key(void* ptr, uint32_t i) const {
    return snapshot_[i].type.keytransform()(
        data(reinterpret_cast<uint8_t*>(ptr) + snapshot_[i].offset,
             snapshot_[i].type.size),
        snapshot_[i].index_bucket_size);
  }

  int64_t get_timestamp(void* ptr) const {
    return *reinterpret_cast<int64_t*>(ptr);
  }

  bool is_indexed(size_t i) const {
    return snapshot_[i].indexed;
  }

  bool index_id(size_t i) const {
    return snapshot_[i].index_id;
  }

  bool index_bucket_size(size_t i) const {
    return snapshot_[i].index_bucket_size;
  }

  size_t num_columns() const {
    return snapshot_.size();
  }

 private:
  std::vector<column_snapshot> snapshot_;
};

}

#endif /* DIALOG_SCHEMA_SNAPSHOT_H_ */
