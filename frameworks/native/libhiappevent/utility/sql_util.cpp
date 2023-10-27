/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sql_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace SqlUtil {
std::string CreateTable(const std::string& table, const std::vector<std::pair<std::string, std::string>>& fields)
{
    // default field: seq
    std::string sql = "CREATE TABLE IF NOT EXISTS " + table + "(" + "seq INTEGER PRIMARY KEY AUTOINCREMENT";
    for (auto field : fields) {
        sql += ", " + field.first + " " + field.second;
    }
    sql += ")";
    return sql;
}

std::string DropTable(const std::string& table)
{
    std::string sql = "DROP TABLE IF EXISTS " + table;
    return sql;
}
} // namespace SqlUtil
} // namespace HiviewDFX
} // namespace OHOS
