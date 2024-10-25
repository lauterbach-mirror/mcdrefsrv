# Test Framework

**NOTE:** The python based test framework requires a version of QEMU with the MCD server patch.
This patch is currently a work in progress.

```bash
pip install -e mcdapi
```

## Under Linux

```bash
QEMU_DIR=<dir> pytest .
```

## Under Windows

QEMU is started inside the WSL.
The following environment variables are expected:

- `WSL_QEMU_DIR`: Absolute path to QEMU directory inside WSL
- `WSL_OPTS`: Options from `wsl --help` you would usually use on the command line

You can start all pytests with

```cmd
pytest .
```
