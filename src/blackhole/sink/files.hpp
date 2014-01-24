#pragma once

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/version.hpp>

#include "blackhole/error.hpp"
#include "blackhole/factory.hpp"
#include "blackhole/sink/files/rotator.hpp"

namespace blackhole {

namespace sink {

class boost_backend_t {
    boost::filesystem::path m_path;
    boost::filesystem::ofstream m_file;
public:
    boost_backend_t(const std::string& path) :
        m_path(path)
    {
    }

    bool opened() const {
        return m_file.is_open();
    }

    bool open() {
        if (!create_directories(m_path.parent_path())) {
            return false;
        }

        m_file.open(m_path, std::ios_base::out | std::ios_base::app);
        return m_file.is_open();
    }

    void close() {
        m_file.close();
    }

    void rename(const std::string& oldname, const std::string& newname) {
        boost::filesystem::rename(m_path.parent_path() / oldname, m_path.parent_path() / newname);
    }

    std::string filename() const {
#if BOOST_VERSION >= 104600
        return m_path.filename().string();
#else
        return m_path.filename();
#endif
    }

    std::string path() const {
        return m_path.string();
    }

    void write(const std::string& message) {
        m_file.write(message.data(), static_cast<std::streamsize>(message.size()));
        m_file.put('\n');
    }

    void flush() {
        m_file.flush();
    }

private:
    template<typename Path>
    bool create_directories(const Path& path) {
        try {
            boost::filesystem::create_directories(path);
        } catch (const boost::filesystem::filesystem_error&) {
            return false;
        }

        return true;
    }
};

namespace file {

template<template<typename> class Rotator = NoRotation>
struct config_t {
    std::string path;
    bool autoflush;

    config_t(const std::string& path = "/dev/stdout", bool autoflush = true) :
        path(path),
        autoflush(autoflush)
    {}
};

template<>
struct config_t<rotator_t> {
    std::string path;
    bool autoflush;
    rotator::config_t rotator;

    config_t(const std::string& path = "/dev/stdout", bool autoflush = true) :
        path(path),
        autoflush(autoflush)
    {}
};

} // namespace file

template<class Backend>
class writer_t {
    Backend& backend;
public:
    writer_t(Backend& backend) :
        backend(backend)
    {}

    void write(const std::string& message) {
        if (!backend.opened()) {
            if (!backend.open()) {
                throw error_t("failed to open file '%s' for writing", backend.path());
            }
        }
        backend.write(message);
    }
};

template<class Backend>
class flusher_t {
    bool autoflush;
    Backend& backend;
public:
    flusher_t(bool autoflush, Backend& backend) :
        autoflush(autoflush),
        backend(backend)
    {}

    void flush() {
        if (autoflush) {
            backend.flush();
        }
    }
};

template<class Backend = boost_backend_t, template<typename> class Rotator = NoRotation, typename = void>
class file_t;

template<class Backend>
class file_t<Backend, NoRotation, void> {
    Backend m_backend;
    writer_t<Backend> m_writer;
    flusher_t<Backend> m_flusher;
public:
    typedef file::config_t<NoRotation> config_type;

    static const char* name() {
        return "files";
    }

    file_t(const config_type& config) :
        m_backend(config.path),
        m_writer(m_backend),
        m_flusher(config.autoflush, m_backend)
    {}

    void consume(const std::string& message) {
        m_writer.write(message);
        m_flusher.flush();
    }

    Backend& backend() {
        return m_backend;
    }
};

template<class Backend, template<typename> class Rotator>
class file_t<
    Backend,
    Rotator,
    typename std::enable_if<
        !std::is_same<Rotator<Backend>, NoRotation<Backend>>::value
    >::type>
{
    Backend m_backend;
    writer_t<Backend> m_writer;
    flusher_t<Backend> m_flusher;
    Rotator<Backend> m_rotator;
public:
    typedef file::config_t<Rotator> config_type;

    static const char* name() {
        return "files";
    }

    file_t(const config_type& config) :
        m_backend(config.path),
        m_writer(m_backend),
        m_flusher(config.autoflush, m_backend),
        m_rotator(config.rotator, m_backend)
    {}

    void consume(const std::string& message) {
        m_writer.write(message);
        m_flusher.flush();
        if (m_rotator.necessary()) {
            m_rotator.rotate();
        }
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

namespace generator {

const uint ROTATOR_POS = 2;

template<class Backend, template<typename> class Rotator>
struct id<sink::file_t<Backend, Rotator>> {
    static std::string extract(const boost::any& config) {
        std::vector<boost::any> cfg;
        aux::any_to(config, cfg);
        std::string rotator;

        if (cfg.size() > ROTATOR_POS && aux::is<std::vector<boost::any>>(cfg.at(ROTATOR_POS))) {
            rotator = sink::rotator_t<Backend>::name();
        }

        return utils::format("files%s", rotator);
    }
};

} // namespace generator

template<class Backend>
struct config_traits<sink::file_t<Backend, sink::NoRotation>> {
    static std::string name() {
        return "files";
    }
};

template<class Backend, template<typename> class Rotator>
struct config_traits<sink::file_t<Backend, Rotator>> {
    static std::string name() {
        return utils::format("files%s", Rotator<Backend>::name());
    }
};

template<class Backend>
struct factory_traits<sink::file_t<Backend>> {
    typedef typename sink::file_t<Backend>::config_type config_type;

    static config_type map_config(const boost::any& config) {
        config_type cfg;
        aux::vector_to(config, cfg.path, cfg.autoflush);
        return cfg;
    }
};

template<class Backend>
struct factory_traits<sink::file_t<Backend, sink::rotator_t>> {
    typedef typename sink::file_t<Backend, sink::rotator_t>::config_type config_type;

    static config_type map_config(const boost::any& config) {
        config_type cfg;
        std::vector<boost::any> rotator;
        aux::vector_to(config, cfg.path, cfg.autoflush, rotator);
        aux::vector_to(rotator, cfg.rotator.size, cfg.rotator.count);
        return cfg;
    }
};

} // namespace blackhole
