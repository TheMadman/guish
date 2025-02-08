# GuiSH - A Wayland shell

GuiSH is a shell that can read scripts and set up
relationships between GUI clients.

Where text-based shells use the pipeline abstraction
to set-up IPC between text-based commands, GuiSH uses
a client-server abstraction. Where you would use
`STDIN_FILENO` and `STDOUT_FILENO` for input/output
in a text-based shell, `GUISRV_FILENO` is connected
to the compositor and `GUICLI_FILENO` is available to
listen for client connections.

# Building

## Dependencies

- [libadt](https://github.com/TheMadman/libadt)
- [scallop-lang](https://github.com/TheMadman/scallop-lang)
- [libwayland](https://gitlab.freedesktop.org/wayland)
- cmake

## Quickstart

```bash
git clone https://github.com/TheMadman/guish.git
mkdir -p guish/build
cd guish/build
cmake ..
cmake --build .
```
