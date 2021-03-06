#pragma once

#include <string>

#include <msgpack.hpp>

#include "blackhole/formatter/base.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

template<typename Stream>
class msgpack_visitor : public boost::static_visitor<> {
    msgpack::packer<Stream>* packer;

public:
    msgpack_visitor(msgpack::packer<Stream>* packer) :
        packer(packer)
    {}

    template<typename T>
    void operator ()(const T& value) const {
        packer->pack(value);
    }

    void operator ()(const timeval& tv) const {
        packer->pack(tv.tv_sec);
    }
};

class msgpack_t : public base_t {
public:
    std::string format(const record_t& record) const {
        msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> packer(&buffer);
        msgpack_visitor<msgpack::sbuffer> visitor(&packer);

        size_t size = 0;
        const auto& attributes = record.attributes();
        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
            size++;
        }

        packer.pack_map(size);
        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
            const std::string& name = it->first;
            const attribute_t& attribute = it->second;
            packer.pack(name);
            boost::apply_visitor(visitor, attribute.value);
        }

        return std::string(buffer.data(), buffer.size());
    }
};

} // namespace formatter

} // namespace blackhole
