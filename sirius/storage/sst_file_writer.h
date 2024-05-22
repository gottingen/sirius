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

#include <string>
#include <sirius/storage/rocks_storage.h>

namespace sirius {

    class SstFileWriter {
    public:
        SstFileWriter(const rocksdb::Options &options) : _options(options) {
            _options.bottommost_compression = rocksdb::kLZ4Compression;
            _options.bottommost_compression_opts = rocksdb::CompressionOptions();
            _sst_writer.reset(new rocksdb::SstFileWriter(rocksdb::EnvOptions(), _options, nullptr, true));
        }

        rocksdb::Status open(const std::string &sst_file) {
            return _sst_writer->Open(sst_file);
        }

        rocksdb::Status put(const rocksdb::Slice &key, const rocksdb::Slice &value) {
            return _sst_writer->Put(key, value);
        }

        rocksdb::Status finish(rocksdb::ExternalSstFileInfo *file_info = nullptr) {
            return _sst_writer->Finish(file_info);
        }

        uint64_t file_size() {
            return _sst_writer->FileSize();
        }

        virtual ~SstFileWriter() {}

    private:
        rocksdb::Options _options;
        std::unique_ptr<rocksdb::SstFileWriter> _sst_writer = nullptr;
    };
}  // namespace sirius
