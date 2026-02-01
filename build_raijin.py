#!/usr/bin/env python3
"""
Raijin Complete Build System - Python Version
Comprehensive compilation for all Raijin components using distutils
"""

import os
import sys
import subprocess
import shutil
from distutils.ccompiler import new_compiler
from distutils.sysconfig import get_config_vars

def run_command(cmd, description=""):
    """Run a shell command and return success status"""
    print(f"[BUILD] {description}")
    print(f"[CMD] {cmd}")
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
        print(f"[SUCCESS] {description}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] {description} failed:")
        print(f"STDOUT: {e.stdout}")
        print(f"STDERR: {e.stderr}")
        return False

def setup_compiler():
    """Setup the C/C++ compiler"""
    print("=" * 60)
    print("    R A I J I N   B U I L D   S Y S T E M")
    print("=" * 60)
    print("Python Build Version")
    print()

    # Create output directories (Bin per README)
    os.makedirs('Bin', exist_ok=True)
    os.makedirs('obj', exist_ok=True)

    # Get compiler
    compiler = new_compiler()
    if compiler is None:
        print("[ERROR] No suitable compiler found!")
        return None

    print(f"[INFO] Using compiler: {compiler.__class__.__name__}")
    return compiler

def compile_sources(compiler):
    """Compile all source files"""
    sources = [
        # HAL
        ('Core/HAL/hal_13700k.cpp', 'hal_13700k.obj'),

        # Hypervisor
        ('Core/Hypervisor/hypervisor_layer.c', 'hypervisor_layer.obj'),

        # Neural Substrate
        ('Core/Neural/neural_substrate.cpp', 'neural_substrate.obj'),

        # Ethics System
        ('Core/Ethics/ethics_system.cpp', 'ethics_system.obj'),

        # Screen Control
        ('Core/ScreenControl/screen_control.cpp', 'screen_control.obj'),

        # Internet Acquisition
        ('Core/InternetAcquisition/internet_acquisition.cpp', 'internet_acquisition.obj'),
        ('Core/InternetAcquisition/http_client.cpp', 'http_client.obj'),

        # Programming Domination
        ('Core/ProgrammingDomination/programming_domination.cpp', 'programming_domination.obj'),

        # Autonomous Manager
        ('Core/Autonomous/autonomous_manager.cpp', 'autonomous_manager.obj'),

        # Evolution Engine
        ('Core/Evolution/evolution_engine.cpp', 'evolution_engine.obj'),

        # Main
        ('Core/Main/raijin_main.cpp', 'raijin_main.obj'),
        ('Core/Main/dominate_main.cpp', 'dominate_main.obj'),
    ]

    objects = []
    include_dirs = ['Include', 'Core/HAL', 'Core/Hypervisor', 'Core/Neural',
                   'Core/Ethics', 'Core/ScreenControl', 'Core/InternetAcquisition',
                   'Core/ProgrammingDomination', 'Core/Autonomous', 'Core/Evolution']

    for source, obj in sources:
        obj_path = f'obj/{obj}'
        print(f"[1/2] Compiling {source}...")

        # Determine if C or C++ file
        if source.endswith('.cpp'):
            extra_preargs = ['/std:c++17', '/EHsc']
        else:
            extra_preargs = []

        try:
            compiler.compile([source],
                           output_dir='obj',
                           include_dirs=include_dirs,
                           extra_preargs=['/c', '/O2', '/W3', '/nologo', '/D_CRT_SECURE_NO_WARNINGS', '/DWIN32_LEAN_AND_MEAN'] + extra_preargs)
            objects.append(obj_path)
            print(f"[SUCCESS] Compiled {source} -> {obj_path}")
        except Exception as e:
            print(f"[ERROR] Failed to compile {source}: {e}")
            return None

    return objects

def link_executable(compiler, objects):
    """Link all object files into final executable"""
    print("[2/2] Linking Raijin executable...")

    libraries = ['kernel32', 'user32', 'advapi32', 'ws2_32', 'gdi32', 'ole32', 'dxgi', 'd3d11']
    library_dirs = []

    objects_main = [o for o in objects if o != 'obj/dominate_main.obj']
    objects_dominate = [o for o in objects if o != 'obj/raijin_main.obj']
    try:
        compiler.link_executable(objects_main,
                               'Bin/raijin.exe',
                               libraries=libraries,
                               library_dirs=library_dirs,
                               extra_preargs=['/SUBSYSTEM:CONSOLE', '/MACHINE:X64'])
        print("[SUCCESS] Linked Bin/raijin.exe")
        compiler.link_executable(objects_dominate,
                               'Bin/raijin-dominate.exe',
                               libraries=libraries,
                               library_dirs=library_dirs,
                               extra_preargs=['/SUBSYSTEM:CONSOLE', '/MACHINE:X64'])
        print("[SUCCESS] Linked Bin/raijin-dominate.exe")
        return True
    except Exception as e:
        print(f"[ERROR] Failed to link executable: {e}")
        return False

def main():
    """Main build function"""
    # Setup compiler
    compiler = setup_compiler()
    if not compiler:
        return False

    # Compile sources
    objects = compile_sources(compiler)
    if not objects:
        return False

    # Link executable
    if not link_executable(compiler, objects):
        return False

    # Success message
    print()
    print("=" * 60)
    print("    R A I J I N   B U I L D   S U C C E S S")
    print("=" * 60)
    print()
    print("raijin.exe and raijin-dominate.exe have been created in Bin\\")
    print()
    print("To run Raijin:")
    print("  Bin\\raijin.exe")
    print("To run Programming Domination CLI:")
    print("  Bin\\raijin-dominate.exe analyze \"code\" --lang python")
    print("  Bin\\raijin-dominate.exe generate \"description\" --lang javascript")
    print("  Bin\\raijin-dominate.exe stats")
    print()

    return True

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)