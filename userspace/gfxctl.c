/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libdevice/framebuffer.h>

#include <libsystem/iostream.h>
#include <libsystem/error.h>
#include <libsystem/cmdline.h>
#include <libsystem/cstring.h>

static bool option_list = false;
static bool option_get = false;
static char *option_set = NULL;

/* --- Command line application initialization -------------------------------*/

static const char *usages[] = {
    "",
    "OPTION...",
    NULL,
};

static cmdline_option_t options[] = {
    CMDLINE_OPT_HELP,

    CMDLINE_OPT_BOOL("list", 'l', option_list, "List all available graphics modes.", CMDLINE_NO_CALLBACK),
    CMDLINE_OPT_BOOL("get", 'g', option_get, "Get the current graphic mode.", CMDLINE_NO_CALLBACK),
    CMDLINE_OPT_STRING("set", 's', option_set, "Set graphic mode.", CMDLINE_NO_CALLBACK),

    CMDLINE_OPT_END,
};

static cmdline_t cmdline = CMDLINE(
    usages,
    options,
    "Get or set graphics modes.",
    "Options can be combined.");

/* --- gfxmode logic -------------------------------------------------------- */

typedef struct
{
    const char *name;
    framebuffer_mode_arg_t info;
} supported_gfxmode_t;

supported_gfxmode_t gfxmodes[] = {
    {"640x360", {{640, 360}}},
    {"800x600", {{800, 600}}},
    {"1024x768", {{1024, 768}}},
    {"1280x768", {{1280, 768}}},
    {NULL, {{0, 0}}},
};

framebuffer_mode_arg_t *gfxmode_by_name(const char *name)
{
    for (int i = 0; gfxmodes[i].name; i++)
    {
        if (strcmp(gfxmodes[i].name, name) == 0)
        {
            return &gfxmodes[i].info;
        }
    }

    return NULL;
}

int gfxmode_get(IOStream *framebuffer_device)
{
    framebuffer_mode_arg_t framebuffer_info;

    if (iostream_call(framebuffer_device, FRAMEBUFFER_CALL_GET_MODE, &framebuffer_info) < 0)
    {
        error_print("Ioctl to " FRAMEBUFFER_DEVICE " failled");
        return -1;
    }

    printf("Height: %d\nWidth: %d\n",
           framebuffer_info.size.X,
           framebuffer_info.size.Y);

    return 0;
}

int gfxmode_set(IOStream *framebuffer_device, const char *mode_name)
{
    framebuffer_mode_arg_t *framebuffer_info = gfxmode_by_name(mode_name);

    if (framebuffer_info != NULL)
    {
        if (iostream_call(framebuffer_device, FRAMEBUFFER_CALL_SET_MODE, framebuffer_info) < 0)
        {
            error_print("Ioctl to " FRAMEBUFFER_DEVICE " failled");
            return -1;
        }

        printf("Graphic mode set to: %s\n", mode_name);

        return 0;
    }
    else
    {
        printf("Error: unknow graphic mode: %s\n", mode_name);
        return -1;
    }
}

int gfxmode_list(IOStream *framebuffer_device)
{
    // FIXME: check if the framebuffer device support the followings graphics modes.

    __unused(framebuffer_device);

    for (int i = 0; gfxmodes[i].name; i++)
    {
        printf("%s\n", gfxmodes[i].name);
    }

    return 0;
}

/* --- Entry point ---------------------------------------------------------- */

int main(int argc, char **argv)
{
    argc = cmdline_parse(&cmdline, argc, argv);

    IOStream *framebuffer_device = iostream_open(FRAMEBUFFER_DEVICE, IOSTREAM_READ);

    if (framebuffer_device == NULL)
    {
        error_print("Failled to open " FRAMEBUFFER_DEVICE);
        return -1;
    }

    if (option_get)
    {
        return gfxmode_get(framebuffer_device);
    }
    else if (option_list)
    {
        return gfxmode_list(framebuffer_device);
    }
    else if (option_set != NULL)
    {
        return gfxmode_set(framebuffer_device, option_set);
    }
    else
    {
        return gfxmode_get(framebuffer_device);
    }
}
