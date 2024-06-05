//
// Copyright (C) 2024 EA group inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#pragma once

#include <mizar/listener.h>
#include <sirius/storage/rocks_storage.h>
#include <sirius/base/log.h>

namespace sirius {
class SimpleListener : public mizar::EventListener {
public:
    virtual ~SimpleListener() {}
    virtual void OnStallConditionsChanged(const mizar::WriteStallInfo& info) {
        bool is_stall = info.condition.cur != mizar::WriteStallCondition::kNormal;
        LOG(INFO) << "OnStallConditionsChanged, cf:" << info.cf_name << " is_stall:" << is_stall;
    }
    virtual void OnFlushCompleted(mizar::DB* /*db*/, const mizar::FlushJobInfo& info) {
        uint64_t file_number = info.file_number;
        RocksStorage::get_instance()->set_flush_file_number(info.cf_name, file_number);
        LOG(INFO) << "OnFlushCompleted, cf:" << info.cf_name << " file_number:" << file_number;
    }
    virtual void OnExternalFileIngested(mizar::DB* /*db*/, const mizar::ExternalFileIngestionInfo& info) {
        LOG(INFO) << "OnExternalFileIngested, cf:" << info.cf_name << " table_properties:" << info.table_properties.ToString();
    }
};
}
