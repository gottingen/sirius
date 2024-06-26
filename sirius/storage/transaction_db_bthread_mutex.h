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

#include <mizar/utilities/transaction_db_mutex.h>

namespace sirius {

    // Default implementation of TransactionDBMutexFactory.  May be overridden
    // by TransactionDBOptions.custom_mutex_factory.
    class TransactionDBBthreadFactory : public mizar::TransactionDBMutexFactory {
    public:
        std::shared_ptr<mizar::TransactionDBMutex> AllocateMutex() override;

        std::shared_ptr<mizar::TransactionDBCondVar> AllocateCondVar() override;
    };

}  // namespace sirius