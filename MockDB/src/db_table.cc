#include "db_table.hpp"

const unsigned int kRowGrowthRate = 2;

void DbTable::Destruct() {
  for (auto it = rows_.begin(); it != rows_.end(); it++) {
    for (unsigned int row_idx = 0; row_idx < col_descs_.size(); row_idx++) {
      void* curr_ptr = it->second[row_idx];
      if (col_descs_[row_idx].second == DataType::kString) {
        auto* string = static_cast<std::string*>(curr_ptr);
        delete string;
      } else if (col_descs_[row_idx].second == DataType::kInt) {
        auto* row_int = static_cast<int*>(curr_ptr);
        delete row_int;
      } else {
        auto* row_double = static_cast<double*>(curr_ptr);
        delete row_double;
      }
    }
    delete[] it->second;
    it->second = nullptr;
  }
  rows_.clear();
}

void DbTable::CopyRows(const DbTable& rhs) {
  for (auto it = rhs.rows_.begin(); it != rhs.rows_.end(); it++) {
    auto* arr = new void*[row_col_capacity_];
    for (unsigned int row = 0; row < col_descs_.size(); row++) {
      // check datatype of row val and allocate new row
      void* curr_ptr = it->second[row];
      if (col_descs_[row].second == DataType::kString) {
        std::string string = *static_cast<std::string*>(curr_ptr);
        arr[row] = static_cast<void*>(new std::string(string));
      } else if (col_descs_[row].second == DataType::kInt) {
        int row_int = *static_cast<int*>(curr_ptr);
        arr[row] = static_cast<void*>(new int(row_int));
      } else if (col_descs_[row].second == DataType::kDouble) {
        double row_double = *static_cast<double*>(curr_ptr);
        arr[row] = static_cast<void*>(new double(row_double));
      }
    }

    // insert new row with first as the uniqueid, second as arr.
    rows_.insert(std::pair<unsigned int, void**>(it->first, arr));
  }
}

void DbTable::ChangeRows() {
  // do
  auto new_capacity = row_col_capacity_ * kRowGrowthRate;
  for (auto it : rows_) {
    auto* copy_rows = new void*[new_capacity];
    for (unsigned idx = 0; idx < row_col_capacity_; idx++) {
      copy_rows[idx] = it.second[idx];
    }
    for (unsigned int col = row_col_capacity_; col < new_capacity; col++) {
      copy_rows[col] = nullptr;
    }
    void** old_ptr = it.second;
    it.second = copy_rows;
    delete[] old_ptr;
  }
  // do
  row_col_capacity_ = new_capacity;
}

void DbTable::AddColumn(const std::pair<std::string, DataType>& col_desc) {
  if (col_descs_.size() == row_col_capacity_) {
    auto new_capacity = row_col_capacity_ * kRowGrowthRate;
    for (auto& row : rows_) {
      void** new_data_pointer = new void*[new_capacity];
      for (unsigned int col = 0; col < row_col_capacity_; col++) {
        new_data_pointer[col] = row.second[col];
      }
      for (unsigned int col = row_col_capacity_; col < new_capacity; col++) {
        new_data_pointer[col] = nullptr;
      }
      void** old_data_pointer = row.second;
      row.second = new_data_pointer;
      delete[] old_data_pointer;
    }
    row_col_capacity_ = new_capacity;
  }
  // add the new column descriptor to vector
  col_descs_.push_back(col_desc);
  size_t new_col_index = col_descs_.size() - 1;
  // initialize the new column in each row of the row table
  for (auto& row : rows_) {
    switch (col_desc.second) {
    case DataType::kString:
      row.second[new_col_index] = new std::string();
      break;
    case DataType::kInt:
      row.second[new_col_index] = new int();
      break;
    case DataType::kDouble:
      row.second[new_col_index] = new double();
      break;
    }
  }
}
void DbTable::DeleteColumnByIdx(unsigned int col_idx) {
  if (col_idx > col_descs_.size() - 1) {
    throw std::out_of_range("col_idx out of rage");
  }
  if (col_descs_.size() == 1 && !rows_.empty()) {
    throw std::runtime_error("bad rows and cols");
  }
  for (auto row : rows_) {
    switch (col_descs_[col_idx].second) {
    case DataType::kString:
      delete static_cast<std::string*>(row.second[col_idx]);
      break;
    case DataType::kInt:
      delete static_cast<int*>(row.second[col_idx]);
      break;
    case DataType::kDouble:
      delete static_cast<double*>(row.second[col_idx]);
      break;
    }
    for (auto col = col_idx; col < col_descs_.size() - 1; col++) {
      row.second[col] = row.second[col + 1];
    }
  }
  col_descs_.erase(col_descs_.begin() + col_idx);
}

void DbTable::AddRow(const std::initializer_list<std::string>& col_data) {
  if (col_data.size() != col_descs_.size()) {
    throw std::runtime_error("invalid col_data size");
  }
  auto* arr = new void*[row_col_capacity_];
  int count = 0;

  // count to keep track of row idx, add string val to new dynamically allocated
  for (const std::string& col_val : col_data) {
    if (col_descs_[count].second == DataType::kString) {
      arr[count] = static_cast<void*>(new std::string(col_val));
    } else if (col_descs_[count].second == DataType::kInt) {
      int int_from_string = std::stoi(col_val);
      arr[count] = static_cast<void*>(new int(int_from_string));
    } else {
      double double_from_string = std::stod(col_val);
      arr[count] = static_cast<void*>(new double(double_from_string));
    }
    count++;
  }
  rows_.insert(std::pair<int, void**>(next_unique_id_, arr));
  next_unique_id_++;
}

void DbTable::DeleteRowById(unsigned int id) {
  auto it = rows_.find(id);

  if (it == rows_.end()) {
    throw std::runtime_error("Row to be deleted does not exist");
  }

  for (size_t col = 0; col < col_descs_.size(); col++) {
    // scalar delete operator because the data pointers in the 2nd dimension
    // point to strings, integers, doubles etc (scalar)
    switch (col_descs_[col].second) {
    case DataType::kString:
      delete static_cast<std::string*>(it->second[col]);
      break;
    case DataType::kInt:
      delete static_cast<int*>(it->second[col]);
      break;
    case DataType::kDouble:
      delete static_cast<double*>(it->second[col]);
      break;
    }
  }

  // delete the entire row, array version of delete[] operator
  delete[] it->second;

  rows_.erase(it);
}

DbTable::DbTable(const DbTable& rhs):
    next_unique_id_(rhs.next_unique_id_),
    row_col_capacity_(rhs.row_col_capacity_),
    col_descs_(rhs.col_descs_) {
  CopyRows(rhs);
}

DbTable& DbTable::operator=(const DbTable& rhs) {
  if (&rhs != this) {
    Destruct();
    next_unique_id_ = rhs.next_unique_id_;
    row_col_capacity_ = rhs.row_col_capacity_;
    col_descs_ = rhs.col_descs_;
    CopyRows(rhs);
  }
  return *this;
}
DbTable::~DbTable() { Destruct(); }

std::ostream& operator<<(std::ostream& os, const DbTable& table) {
  auto last_val = table.col_descs_.size() - 1;
  for (const auto& element : table.col_descs_) {
    switch (element.second) {
    case DataType::kString:
      (element != table.col_descs_[last_val]) ? os << element.first << "(std::string), "
                                              : os << element.first << "(std::string)";
      break;
    case DataType::kDouble:
      (element != table.col_descs_[last_val]) ? os << element.first << "(double), "
                                              : os << element.first << "(double)";
      break;
    case DataType::kInt:
      (element != table.col_descs_[last_val]) ? os << element.first << "(int), "
                                              : os << element.first << "(int)";
    }
  }
  os << "\n";
  for (auto row : table.rows_) {
    for (size_t col = 0; col < table.col_descs_.size(); col++) {
      auto size = table.col_descs_.size() - 1;
      switch (table.col_descs_[col].second) {
      case DataType::kString:
        (col == size)
            ? os << *static_cast<std::string*>(row.second[col])
            : os << *static_cast<std::string*>(row.second[col]) << ", ";
        break;
      case DataType::kInt:
        (col == size) ? os << *static_cast<int*>(row.second[col])
                      : os << *static_cast<int*>(row.second[col]) << ", ";
        break;
      case DataType::kDouble:
        (col == size) ? os << *static_cast<double*>(row.second[col])
                      : os << *static_cast<double*>(row.second[col]) << ", ";
      }
    }
    os << "\n";
  }
  return os;
}
