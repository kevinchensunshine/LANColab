#include <iostream>

#include "db.hpp"
#include "db_table.hpp"

int main() {
  DbTable a;
  a.AddColumn(std::pair<std::string, DataType>("Kevin", DataType::kString));
  a.AddColumn(std::pair<std::string, DataType>("Three", DataType::kInt));
  a.AddColumn(std::pair<std::string, DataType>("FourPointO", DataType::kDouble));

  a.AddRow({"Kevin", "2", "3.0"});
  a.AddRow({"Kule", "3", "2.0"});
  
  std::cout << a;
}