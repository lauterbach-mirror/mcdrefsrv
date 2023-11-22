# this is a startup skript for TRACE32 it helps you to link the shared library to trace32
import pathlib
import platform
import lauterbach.trace32.pystart as pystart
import argparse


class ChiplistArgumentParser(argparse.ArgumentParser):
    def __init__(self):
        super().__init__(description="Argument parser for mcd_trace32_start script")
        self.add_argument("--exe", required=False, default='t32marm', help="Desired architecture, provide the name of the executable without the extension")


def main():
    # collecting parameters
    args = ChiplistArgumentParser().parse_args()
    target = args.exe

    # detecting os
    workspace = pathlib.Path('<workspace_path>')
    operating_system = platform.system()
    if operating_system == 'Linux':
        # executing from wsl
        path_to_home = pathlib.Path('<wsl_home_directory>')
        path_to_shared_library = path_to_home.joinpath(workspace, 'build/linux-default/src/libmcd_shared_library.so')
        system_path = '<wsl_path_to_t32_installation>'
    elif operating_system == 'Windows':
        # executing from main windows machine
        path_to_home = pathlib.Path('<windows_home_directory')
        path_to_shared_library = path_to_home.joinpath(workspace, 'build/windows-default/src/Debug/mcd_shared_library.dll')
        system_path = '<windows_path_to_t32_installation>'
    else:
        raise ValueError(f'The operating system {operating_system} is not supported')
    
    # configure trace32
    pv = pystart.PowerView(connection=pystart.MCDConnection(str(path_to_shared_library)), target=target, system_path=system_path)

    # get licence information
    # TODO: Add your license

    # start trace32
    pv.start()
    pv.wait()


if __name__ == "__main__":
    main()
