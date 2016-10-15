/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/bitcoin/message/inventory_vector.hpp>

#include <cstdint>
#include <bitcoin/bitcoin/message/inventory.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace message {

uint32_t inventory_vector::to_number(type_id inventory_type)
{
    switch (inventory_type)
    {
        case type_id::compact_block:
            return 4;
        case type_id::block:
            return 2;
        case type_id::transaction:
            return 1;
        case type_id::error:
        case type_id::none:
        default:
            return 0;
    }
}

inventory_vector::type_id inventory_vector::to_type(uint32_t value)
{
    switch (value)
    {
        case 0:
            return type_id::error;
        case 1:
            return type_id::transaction;
        case 2:
            return type_id::block;
        case 4:
            return type_id::compact_block;
        default:
            return type_id::none;
    }
}

inventory_vector inventory_vector::factory_from_data(uint32_t version,
    const data_chunk& data)
{
    inventory_vector instance;
    instance.from_data(version, data);
    return instance;
}

inventory_vector inventory_vector::factory_from_data(uint32_t version,
    std::istream& stream)
{
    inventory_vector instance;
    instance.from_data(version, stream);
    return instance;
}

inventory_vector inventory_vector::factory_from_data(uint32_t version,
    reader& source)
{
    inventory_vector instance;
    instance.from_data(version, source);
    return instance;
}

inventory_vector::inventory_vector()
  : inventory_vector(type_id::error, null_hash)
{
}

inventory_vector::inventory_vector(type_id type, const hash_digest& hash)
  : type_(type), hash_(hash)
{
}

inventory_vector::inventory_vector(type_id type, hash_digest&& hash)
  : type_(type), hash_(std::move(hash))
{
}

inventory_vector::inventory_vector(const inventory_vector& other)
  : inventory_vector(other.type_, other.hash_)
{
}

inventory_vector::inventory_vector(inventory_vector&& other)
  : inventory_vector(other.type_, std::move(other.hash_))
{
}

bool inventory_vector::is_valid() const
{
    return (type_ != type_id::error) || (hash_ != null_hash);
}

void inventory_vector::reset()
{
    type_ = type_id::error;
    hash_.fill(0);
}

bool inventory_vector::from_data(uint32_t version,
    const data_chunk& data)
{
    data_source istream(data);
    return from_data(version, istream);
}

bool inventory_vector::from_data(uint32_t version,
    std::istream& stream)
{
    istream_reader source(stream);
    return from_data(version, source);
}

bool inventory_vector::from_data(uint32_t version,
    reader& source)
{
    reset();

    uint32_t raw_type = source.read_4_bytes_little_endian();
    type_ = inventory_vector::to_type(raw_type);
    hash_ = source.read_hash();
    bool result = static_cast<bool>(source);

    if (!result)
        reset();

    return result;
}

data_chunk inventory_vector::to_data(uint32_t version) const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(version, ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(version));
    return data;
}

void inventory_vector::to_data(uint32_t version,
    std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(version, sink);
}

void inventory_vector::to_data(uint32_t version,
    writer& sink) const
{
    const auto raw_type = inventory_vector::to_number(type_);
    sink.write_4_bytes_little_endian(raw_type);
    sink.write_hash(hash_);
}

uint64_t inventory_vector::serialized_size(uint32_t version) const
{
    return inventory_vector::satoshi_fixed_size(version);
}

uint64_t inventory_vector::satoshi_fixed_size(uint32_t version)
{
    return sizeof(hash_) + sizeof(uint32_t);
}

bool inventory_vector::is_block_type() const
{
    return
        type_ == type_id::block ||
        type_ == type_id::compact_block ||
        type_ == type_id::filtered_block;
}

bool inventory_vector::is_transaction_type() const
{
    return type_ == type_id::transaction;
}

inventory_vector::type_id inventory_vector::type() const
{
    return type_;
}

void inventory_vector::set_type(type_id value)
{
    type_ = value;
}

hash_digest& inventory_vector::hash()
{
    return hash_;
}

const hash_digest& inventory_vector::hash() const
{
    return hash_;
}

void inventory_vector::set_hash(const hash_digest& value)
{
    hash_ = value;
}

void inventory_vector::set_hash(hash_digest&& value)
{
    hash_ = std::move(value);
}

inventory_vector& inventory_vector::operator=(inventory_vector&& other)
{
    type_ = other.type_;
    hash_ = std::move(other.hash_);
    return *this;
}

void inventory_vector::operator=(const inventory_vector& other)
{
    type_ = other.type_;
    hash_ = other.hash_;
}

bool inventory_vector::operator==(const inventory_vector& other) const
{
    return (hash_ == other.hash_) && (type_ == other.type_);
}

bool inventory_vector::operator!=(const inventory_vector& other) const
{
    return !(*this == other);
}

} // namespace message
} // namespace libbitcoin
