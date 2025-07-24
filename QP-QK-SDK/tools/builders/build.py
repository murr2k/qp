#!/usr/bin/env python3
"""
QP-QK SDK Build Tool
AI Agent automated build system for QP-QK projects

This tool provides automated building for microcontroller projects
using the QP framework with QK kernel.
"""

import os
import sys
import argparse
import subprocess
import json
import yaml
from pathlib import Path
from typing import Dict, List, Optional
import time

class QKBuilder:
    """Automated build system for QP-QK projects"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.build_dir = self.project_root / "build"
        self.config = self.load_build_config()
        self.toolchain = self.setup_toolchain()
        
    def load_build_config(self) -> Dict:
        """Load build configuration from project"""
        config_files = [
            self.project_root / "build_config.yaml",
            self.project_root / "build_config.json",
            self.project_root / "project.yaml"
        ]
        
        for config_file in config_files:
            if config_file.exists():
                if config_file.suffix == '.yaml':
                    with open(config_file, 'r') as f:
                        return yaml.safe_load(f)
                elif config_file.suffix == '.json':
                    with open(config_file, 'r') as f:
                        return json.load(f)
        
        # Default configuration
        return {
            'platform': 'stm32f4',
            'toolchain': 'gcc-arm-none-eabi',
            'optimization': 'Os',
            'debug': True,
            'qp_path': '../qpc',
            'sources': ['src/*.c'],
            'includes': ['inc', 'src'],
            'defines': ['USE_HAL_DRIVER', 'STM32F411xE']
        }
    
    def setup_toolchain(self) -> Dict:
        """Setup toolchain configuration"""
        toolchain_configs = {
            'gcc-arm-none-eabi': {
                'cc': 'arm-none-eabi-gcc',
                'ld': 'arm-none-eabi-gcc',
                'objcopy': 'arm-none-eabi-objcopy',
                'objdump': 'arm-none-eabi-objdump',
                'size': 'arm-none-eabi-size',
                'gdb': 'arm-none-eabi-gdb'
            },
            'clang': {
                'cc': 'clang',  
                'ld': 'clang',
                'objcopy': 'llvm-objcopy',
                'objdump': 'llvm-objdump',
                'size': 'llvm-size',
                'gdb': 'gdb'
            }
        }
        
        toolchain_name = self.config.get('toolchain', 'gcc-arm-none-eabi')
        return toolchain_configs.get(toolchain_name, toolchain_configs['gcc-arm-none-eabi'])
    
    def get_platform_flags(self) -> Dict:
        """Get platform-specific compiler flags"""
        platform_flags = {
            'stm32f4': {
                'cflags': [
                    '-mcpu=cortex-m4',
                    '-mthumb',
                    '-mfpu=fpv4-sp-d16',
                    '-mfloat-abi=hard',
                    '-fdata-sections',
                    '-ffunction-sections'
                ],
                'ldflags': [
                    '-mcpu=cortex-m4',
                    '-mthumb',
                    '-mfpu=fpv4-sp-d16', 
                    '-mfloat-abi=hard',
                    '-specs=nano.specs',
                    '-specs=nosys.specs',
                    '-Wl,--gc-sections',
                    '-Wl,-Map=build/output.map'
                ],
                'linker_script': 'STM32F411RETx_FLASH.ld'
            },
            'esp32': {
                'cflags': [
                    '-mlongcalls',
                    '-ffunction-sections',
                    '-fdata-sections'
                ],
                'ldflags': [
                    '-nostdlib',
                    '-Wl,--gc-sections'
                ],
                'linker_script': 'esp32.ld'
            },
            'msp430': {
                'cflags': [
                    '-mmcu=msp430g2553',
                    '-fdata-sections',
                    '-ffunction-sections'
                ],
                'ldflags': [
                    '-mmcu=msp430g2553',
                    '-Wl,--gc-sections'
                ],
                'linker_script': 'msp430g2553.ld'
            }
        }
        
        platform = self.config.get('platform', 'stm32f4')
        return platform_flags.get(platform, platform_flags['stm32f4'])
    
    def collect_sources(self) -> List[Path]:
        """Collect all source files for compilation"""
        sources = []
        source_patterns = self.config.get('sources', ['src/*.c'])
        
        for pattern in source_patterns:
            pattern_path = self.project_root / pattern
            if '*' in pattern:
                # Glob pattern
                sources.extend(self.project_root.glob(pattern))
            else:
                # Direct file
                if pattern_path.exists():
                    sources.append(pattern_path)
        
        # Add QP framework sources
        qp_path = Path(self.config.get('qp_path', '../qpc'))
        if qp_path.exists():
            qp_src = qp_path / 'src'
            if qp_src.exists():
                sources.extend(qp_src.glob('qf/*.c'))
                sources.extend(qp_src.glob('qk/*.c'))  # QK kernel
                sources.extend(qp_src.glob('qs/*.c'))   # QS tracing
        
        return sources
    
    def get_include_dirs(self) -> List[str]:
        """Get include directories"""
        includes = []
        
        # Project includes
        project_includes = self.config.get('includes', ['inc', 'src'])
        for inc in project_includes:
            inc_path = self.project_root / inc
            if inc_path.exists():
                includes.append(str(inc_path))
        
        # QP framework includes
        qp_path = Path(self.config.get('qp_path', '../qpc'))
        if qp_path.exists():
            includes.extend([
                str(qp_path / 'include'),
                str(qp_path / 'ports' / 'arm-cm' / 'qk' / 'gnu')
            ])
        
        return includes
    
    def get_defines(self) -> List[str]:
        """Get preprocessor defines"""
        defines = self.config.get('defines', [])
        
        # Add QP-specific defines
        defines.extend([
            'QP_API_VERSION=720',  # QP version
            'Q_SPY=1' if self.config.get('debug', True) else 'Q_SPY=0'
        ])
        
        return defines
    
    def build_objects(self, sources: List[Path]) -> List[Path]:
        """Compile source files to object files"""
        print("Compiling source files...")
        
        # Create build directory
        self.build_dir.mkdir(exist_ok=True)
        obj_dir = self.build_dir / "obj"
        obj_dir.mkdir(exist_ok=True)
        
        objects = []
        platform_flags = self.get_platform_flags()
        include_dirs = self.get_include_dirs()
        defines = self.get_defines()
        
        # Base compiler flags
        cflags = [
            f'-{self.config.get("optimization", "Os")}',
            '-Wall',
            '-Wextra',
            '-std=c99'
        ]
        
        if self.config.get('debug', True):
            cflags.extend(['-g', '-DDEBUG'])
        
        # Add platform-specific flags
        cflags.extend(platform_flags['cflags'])
        
        # Add includes
        for inc in include_dirs:
            cflags.append(f'-I{inc}')
        
        # Add defines
        for define in defines:
            cflags.append(f'-D{define}')
        
        # Compile each source file
        for src in sources:
            obj_file = obj_dir / f"{src.stem}.o"
            objects.append(obj_file)
            
            # Check if compilation needed
            if (not obj_file.exists() or 
                src.stat().st_mtime > obj_file.stat().st_mtime):
                
                cmd = [self.toolchain['cc']] + cflags + ['-c', str(src), '-o', str(obj_file)]
                
                print(f"Compiling {src.name}...")
                result = subprocess.run(cmd, capture_output=True, text=True)
                
                if result.returncode != 0:
                    print(f"Error compiling {src.name}:")
                    print(result.stderr)
                    sys.exit(1)
        
        return objects
    
    def link_executable(self, objects: List[Path]) -> Path:
        """Link object files into executable"""
        print("Linking executable...")
        
        platform_flags = self.get_platform_flags()
        output_elf = self.build_dir / f"{self.config.get('project_name', 'firmware')}.elf"
        
        # Linker flags
        ldflags = platform_flags['ldflags']
        
        # Add linker script if specified
        linker_script = self.project_root / platform_flags.get('linker_script', '')
        if linker_script.exists():
            ldflags.extend(['-T', str(linker_script)])
        
        # Link command
        cmd = ([self.toolchain['ld']] + ldflags + 
               [str(obj) for obj in objects] + 
               ['-o', str(output_elf)])
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            print("Error linking:")
            print(result.stderr)
            sys.exit(1)
        
        return output_elf
    
    def generate_binary(self, elf_file: Path) -> Dict[str, Path]:
        """Generate binary files from ELF"""
        print("Generating binary files...")
        
        outputs = {}
        base_name = elf_file.stem
        
        # Generate .bin file
        bin_file = self.build_dir / f"{base_name}.bin"
        cmd = [self.toolchain['objcopy'], '-O', 'binary', str(elf_file), str(bin_file)]
        subprocess.run(cmd, check=True)
        outputs['bin'] = bin_file
        
        # Generate .hex file
        hex_file = self.build_dir / f"{base_name}.hex"
        cmd = [self.toolchain['objcopy'], '-O', 'ihex', str(elf_file), str(hex_file)]
        subprocess.run(cmd, check=True)
        outputs['hex'] = hex_file
        
        # Generate disassembly
        asm_file = self.build_dir / f"{base_name}.asm"
        with open(asm_file, 'w') as f:
            cmd = [self.toolchain['objdump'], '-d', str(elf_file)]
            subprocess.run(cmd, stdout=f, check=True)
        outputs['asm'] = asm_file
        
        return outputs
    
    def analyze_size(self, elf_file: Path):
        """Analyze memory usage"""
        print("Analyzing memory usage...")
        
        cmd = [self.toolchain['size'], '-A', str(elf_file)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print("\nMemory Usage:")
            print(result.stdout)
        
        # Save size report
        size_file = self.build_dir / "size_report.txt"
        with open(size_file, 'w') as f:
            f.write(result.stdout)
    
    def validate_build(self, elf_file: Path) -> bool:
        """Validate the built firmware"""
        print("Validating build...")
        
        validation_passed = True
        
        # Check file exists and has reasonable size
        if not elf_file.exists():
            print("ERROR: ELF file not generated")
            return False
        
        file_size = elf_file.stat().st_size
        if file_size < 1024:  # Less than 1KB seems too small
            print(f"WARNING: ELF file size ({file_size} bytes) seems very small")
            validation_passed = False
        
        # Check for required symbols (QP framework)
        cmd = [self.toolchain['objdump'], '-t', str(elf_file)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        required_symbols = ['QF_init', 'QF_run', 'QActive_start']
        missing_symbols = []
        
        for symbol in required_symbols:
            if symbol not in result.stdout:
                missing_symbols.append(symbol)
        
        if missing_symbols:
            print(f"ERROR: Missing required symbols: {missing_symbols}")
            validation_passed = False
        
        # Check for QK kernel symbols
        qk_symbols = ['QK_sched_', 'QK_activate_']
        qk_found = any(sym in result.stdout for sym in qk_symbols)
        if not qk_found:
            print("WARNING: QK kernel symbols not found - ensure QK is linked")
        
        return validation_passed
    
    def build(self, config_name: str = 'release') -> Dict:
        """Perform complete build"""
        start_time = time.time()
        print(f"Starting build for configuration: {config_name}")
        print(f"Project: {self.project_root}")
        print(f"Platform: {self.config.get('platform', 'unknown')}")
        print(f"Toolchain: {self.config.get('toolchain', 'unknown')}")
        print()
        
        try:
            # Collect sources
            sources = self.collect_sources()
            print(f"Found {len(sources)} source files")
            
            # Compile
            objects = self.build_objects(sources)
            
            # Link
            elf_file = self.link_executable(objects)
            
            # Generate binaries
            outputs = self.generate_binary(elf_file)
            outputs['elf'] = elf_file
            
            # Analyze size
            self.analyze_size(elf_file)
            
            # Validate
            if not self.validate_build(elf_file):
                print("Build validation failed!")
                sys.exit(1)
            
            build_time = time.time() - start_time
            print(f"\nBuild completed successfully in {build_time:.2f} seconds")
            print(f"Output files:")
            for file_type, file_path in outputs.items():
                print(f"  {file_type.upper()}: {file_path}")
            
            return {
                'success': True,
                'outputs': outputs,
                'build_time': build_time
            }
            
        except Exception as e:
            print(f"Build failed: {e}")
            return {
                'success': False,
                'error': str(e),
                'build_time': time.time() - start_time
            }
    
    def clean(self):
        """Clean build artifacts"""
        print("Cleaning build artifacts...")
        
        if self.build_dir.exists():
            import shutil
            shutil.rmtree(self.build_dir)
            print("Build directory cleaned")
        else:
            print("Build directory already clean")


def main():
    parser = argparse.ArgumentParser(description='QP-QK SDK Build Tool')
    parser.add_argument('--project', '-p', default='.', 
                       help='Project directory (default: current directory)')
    parser.add_argument('--config', '-c', default='release',
                       help='Build configuration (default: release)')
    parser.add_argument('--clean', action='store_true',
                       help='Clean build artifacts before building')
    parser.add_argument('--clean-only', action='store_true',
                       help='Only clean, do not build')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose output')
    
    args = parser.parse_args()
    
    # Create builder
    try:
        builder = QKBuilder(args.project)
    except Exception as e:
        print(f"Error initializing builder: {e}")
        sys.exit(1)
    
    # Clean if requested
    if args.clean or args.clean_only:
        builder.clean()
    
    # Build if not clean-only
    if not args.clean_only:
        result = builder.build(args.config)
        
        if not result['success']:
            sys.exit(1)


if __name__ == '__main__':
    main()