#pragma once

#include <abi-ms/abi.h>
#include <efi/base.h>
#include <karm-fmt/fmt.h>
#include <karm-sys/chan.h>
#include <karm-sys/context.h>

void __panicHandler(Karm::PanicKind kind, char const *msg);

extern "C" Efi::Status efi_main(Efi::Handle handle, Efi::SystemTable *st) {
    Efi::init(handle, st);
    Abi::Ms::init();
    Karm::registerPanicHandler(__panicHandler);

    (void)Efi::st()->conOut->clearScreen(Efi::st()->conOut);

    char const *self = "efi-app";
    char const *argv[] = {self, nullptr};
    Sys::globalCtx().add<Sys::ArgsHook>(1, argv);
    Res<> code = entryPoint(Sys::globalCtx());
    if (not code) {
        Error error = code.none();
        (void)Fmt::format(Sys::err(), "{}: {}\n", self, error.msg());
        return 1;
    }

    return EFI_SUCCESS;
}
