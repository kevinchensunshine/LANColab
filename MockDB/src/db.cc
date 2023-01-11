#include "db.hpp"

void Database::CreateTable(const std::string& table_name) {
  auto* db = new DbTable();
  tables_.insert(std::pair<std::string, DbTable*>(table_name, db));
}
void Database::DropTable(const std::string& table_name) {
  if (tables_.contains(table_name)) {
    delete tables_[table_name];
    tables_.erase(table_name);
  } else {
    throw std::runtime_error("table_name not found");
  }
}
DbTable& Database::GetTable(const std::string& table_name) {
  return *(tables_[table_name]);
}

Database::Database(const Database& rhs) {
  for (const auto& it : rhs.tables_) {
    auto* curr_ptr = it.second;
    DbTable new_db = DbTable(*curr_ptr);
    auto* db = new DbTable(new_db);
    tables_.insert(std::pair<std::string, DbTable*>(it.first, db));
  }
}
Database& Database::operator=(const Database& rhs) {
  if (&rhs != this) {
    tables_.clear();
    for (const auto& it : tables_) {
      auto *curr_ptr = it.second;
      delete curr_ptr;
      curr_ptr = nullptr;
    }
    for (auto it = rhs.tables_.begin(); it != rhs.tables_.end(); it++) {
      auto* db = new DbTable(*(it->second));
      this->tables_.insert(std::pair<std::string, DbTable*>(it->first, db));
    }
  }
  return *this;
}
Database::~Database() {
  for (auto it = tables_.begin(); it != tables_.end(); it++) {
    delete it->second;
    it->second = nullptr;
  }
  tables_.clear();
}