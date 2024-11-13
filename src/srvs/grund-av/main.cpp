#include <karm-sys/entry.h>
#include <karm-sys/rpc.h>

namespace Grund::Av {

Async::Task<> serv(Sys::Context &ctx) {
    Sys::Rpc rpc = Sys::Rpc::create(ctx);

    logInfo("service started");
    while (true) {
        co_trya$(rpc.recvAsync());
        logDebug("received message from system");
    }
}

} // namespace Grund::Av

Async::Task<> entryPointAsync(Sys::Context &ctx) {
    return Grund::Av::serv(ctx);
}
