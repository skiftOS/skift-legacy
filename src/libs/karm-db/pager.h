#pragma once

#include <karm-sys/file.h>
#include <karm-sys/mmap.h>

namespace Karm::Db {

struct Page {};

struct Pager {
    virtual ~Pager() = default;
};

struct Cache : public Pager {
    virtual ~Cache() = default;
};

struct MemPager : public Pager {
    static Strong<MemPager> create(usize size) {
        return makeStrong<MemPager>(size);
    }
};

struct FilePager : public Pager {
    static Strong<FilePager> create(String const &) {
        return makeStrong<FilePager>();
    }

    static Strong<FilePager> open(String const &, usize) {
        return makeStrong<FilePager>();
    }
};

} // namespace Karm::Db
