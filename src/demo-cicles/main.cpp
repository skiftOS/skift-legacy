#include <karm-main/main.h>
#include <karm-math/rand.h>
#include <karm-ui/app.h>
#include <karm-ui/funcs.h>

struct CirclesApp : public Ui::Widget<CirclesApp> {
    Math::Vec2i _mousePos{};
    int _frame = 0;

    void paint(Gfx::Context &g) override {
        Math::Rand rand{_frame};
        double size = rand.nextInt(4, 10);
        size *= size;

        g.fillStyle(Gfx::BLACK.withOpacity(0.01));
        g.fill(g.clip());

        if (_frame % 16 == 0) {
            g.begin();
            g.ellipse({
                rand.nextVec2(g.clip()).cast<double>(),
                size,
            });

            g.strokeStyle(
                Gfx::stroke(Gfx::WHITE)
                    .withWidth(rand.nextInt(2, size)));
            g.stroke();
        }

        g.begin();
        g.ellipse({_mousePos.cast<double>(), 100});
        g.fillStyle(Gfx::BLUE500);
        g.fill();
    }

    void event(Events::Event &e) override {
        e
            .handle<Events::MouseEvent>([&](auto &e) {
                if (e.type == Events::MouseEvent::MOVE) {
                    _mousePos = e.pos;
                    Ui::shouldAnimate(*this);
                    return true;
                }
                return true;
            })
            .handle<Events::AnimateEvent>([&](auto &) {
                _frame++;
                Ui::shouldRepaint(*this);
                return true;
            });
    }
};

CliResult entryPoint(CliArgs args) {
    return Ui::runApp<CirclesApp>(args);
}