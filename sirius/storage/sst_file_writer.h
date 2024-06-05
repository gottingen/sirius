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
        SstFileWriter(const mizar::Options &options) : _options(options) {
            _options.bottommost_compression = mizar::kLZ4Compression;
            _options.bottommost_compression_opts = mizar::CompressionOptions();
            _sst_writer.reset(new mizar::SstFileWriter(mizar::EnvOptions(), _options, nullptr, true));
        }

        mizar::Status open(const std::string &sst_file) {
            return _sst_writer->Open(sst_file);
        }

        mizar::Status put(const mizar::Slice &key, const mizar::Slice &value) {
            return _sst_writer->Put(key, value);
        }

        mizar::Status finish(mizar::ExternalSstFileInfo *file_info = nullptr) {
            return _sst_writer->Finish(file_info);
        }

        uint64_t file_size() {
            return _sst_writer->FileSize();
        }

        virtual ~SstFileWriter() {}

    private:
        mizar::Options _options;
        std::unique_ptr<mizar::SstFileWriter> _sst_writer = nullptr;
    };
}  // namespace sirius
