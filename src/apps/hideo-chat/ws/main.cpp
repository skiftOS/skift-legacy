#include <karm-db/connect.h>
#include <karm-net/router.h>
#include <karm-sys/entry.h>

namespace Hideo::Chat {

// struct User {
//     u64 id;
//     String email;
//     String name;
//     String password;
// };
//
// struct Chat {
//     u64 id;
//     String name;
// };
//
// struct ChatUser {
//     u64 chat;
//     u64 user;
// };
//
// struct Invite {
//     u64 id;
//
//     u64 chat;
//     u64 user;
//     bool accepted;
//
//     TimeStamp time;
// };
//
// struct Message {
//     u64 chat;
//     u64 user;
//     String text;
//     TimeStamp time;
// };

Async::Task<Net::Asgi> app() {
    auto db = co_trya$(Db::connectAsync("file:./chat.db"_url));

    co_return Net::router(
        Net::post(
            "/auth/register",
            [db](Net::Scope, Net::Recv recv, Net::Send) -> Async::Task<> {
                auto json = co_trya$(recv.jsonAsync());

                auto email = json["email"];
                auto name = json["name"];
                auto password = json["password"];
                auto confirm = json["confirm"];

                bool userExist =
                    iter(db->users)
                        .any([&](auto &user) {
                            return user.email == email;
                        });

                co_return Ok();
            }
        ),
        Net::post(
            "/auth/login",
            [db](Net::Scope, Net::Recv recv, Net::Send) -> Async::Task<> {
                auto json = co_trya$(recv.jsonAsync());

                auto email = json.get("email");
                auto password = json.get("password");

                co_return Ok();
            }
        ),

        Net::get(
            "/users",
            [db](Net::Scope, Net::Recv, Net::Send) -> Async::Task<> {
                co_return Ok();
            }
        ),
        Net::get(
            "/users/:id",
            [db](Net::Scope const &, Net::Recv, Net::Send send) -> Async::Task<> {
                co_return Ok();
            }
        ),
        Net::staticFile("/media", "file:./media/"_url)
    );
}

} // namespace Hideo::Chat

Async::Task<> entryPointAsync(Sys::Context &) {
    co_return co_await Net::servAsync(co_trya$(Hideo::Chat::app()));
}
