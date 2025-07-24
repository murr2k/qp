#!/usr/bin/env python3
"""
QP-QK SDK Flash Tool
AI Agent automated deployment system for QP-QK projects

This tool provides automated flashing and deployment for microcontroller
projects using various programming interfaces.
"""

import os
import sys
import argparse
import subprocess
import json
import time
from pathlib import Path
from typing import Dict, List, Optional

class QKFlasher:
    """Automated flash and deployment system for QP-QK projects"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.build_dir = self.project_root / "build"
        self.config = self.load_flash_config()
        
    def load_flash_config(self) -> Dict:
        """Load flash configuration from project"""
        config_file = self.project_root / "flash_config.json"
        
        if config_file.exists():
            with open(config_file, 'r') as f:
                return json.load(f)
        
        # Default configuration
        return {
            'interface': 'stlink',
            'target': 'stm32f4',
            'flash_base': '0x08000000',
            'flash_size': '512K',
            'ram_base': '0x20000000',
            'ram_size': '128K',
            'programmer_speed': 'auto',
            'reset_after_flash': True,
            'verify_after_flash': True
        }
    
    def get_interface_config(self, interface: str) -> Dict:
        """Get configuration for specific programming interface"""
        
        interface_configs = {
            'stlink': {
                'tool': 'st-flash',
                'probe_cmd': ['st-info', '--probe'],
                'flash_cmd': ['st-flash', 'write'],
                'erase_cmd': ['st-flash', 'erase'],
                'reset_cmd': ['st-flash', 'reset'],
                'supported_targets': ['stm32f0', 'stm32f1', 'stm32f2', 'stm32f3', 'stm32f4', 'stm32f7', 'stm32h7', 'stm32l0', 'stm32l1', 'stm32l4']
            },
            
            'jlink': {
                'tool': 'JLinkExe',
                'probe_cmd': ['JLinkExe', '-CommanderScript', 'probe.jlink'],
                'flash_cmd': ['JLinkExe', '-CommanderScript'],
                'erase_cmd': ['JLinkExe', '-CommanderScript'],
                'reset_cmd': ['JLinkExe', '-CommanderScript'],
                'supported_targets': ['stm32f4', 'stm32f7', 'stm32h7', 'nrf52', 'esp32']
            },
            
            'openocd': {
                'tool': 'openocd',
                'probe_cmd': ['openocd', '-f', 'interface/stlink.cfg', '-f', 'target/stm32f4x.cfg', '-c', 'init; halt; exit'],
                'flash_cmd': ['openocd', '-f', 'interface/stlink.cfg', '-f', 'target/stm32f4x.cfg'],
                'erase_cmd': ['openocd', '-f', 'interface/stlink.cfg', '-f', 'target/stm32f4x.cfg'],
                'reset_cmd': ['openocd', '-f', 'interface/stlink.cfg', '-f', 'target/stm32f4x.cfg'],
                'supported_targets': ['stm32f0', 'stm32f1', 'stm32f2', 'stm32f3', 'stm32f4', 'stm32f7', 'stm32h7', 'stm32l0', 'stm32l1', 'stm32l4', 'nrf52', 'esp32']
            },
            
            'dfu': {
                'tool': 'dfu-util',
                'probe_cmd': ['dfu-util', '-l'],
                'flash_cmd': ['dfu-util', '-a', '0', '-s'],
                'erase_cmd': ['dfu-util', '-a', '0', '-s', '0x08000000:mass-erase:force'],
                'reset_cmd': ['dfu-util', '-a', '0', '-s', '0x08000000:leave'],
                'supported_targets': ['stm32f0', 'stm32f1', 'stm32f2', 'stm32f3', 'stm32f4', 'stm32f7', 'stm32h7', 'stm32l0', 'stm32l1', 'stm32l4']
            },
            
            'esp32': {
                'tool': 'esptool.py',
                'probe_cmd': ['esptool.py', 'chip_id'],
                'flash_cmd': ['esptool.py', '--chip', 'esp32', '--baud', '921600', 'write_flash'],
                'erase_cmd': ['esptool.py', '--chip', 'esp32', 'erase_flash'],
                'reset_cmd': ['esptool.py', '--chip', 'esp32', 'run'],
                'supported_targets': ['esp32', 'esp32s2', 'esp32s3', 'esp32c3']
            },
            
            'msp430': {
                'tool': 'mspdebug',
                'probe_cmd': ['mspdebug', 'rf2500', 'exit'],
                'flash_cmd': ['mspdebug', 'rf2500', 'prog'],
                'erase_cmd': ['mspdebug', 'rf2500', 'erase'],
                'reset_cmd': ['mspdebug', 'rf2500', 'reset'],
                'supported_targets': ['msp430g2553', 'msp430f5529', 'msp430fr5969']
            }
        }
        
        return interface_configs.get(interface, interface_configs['stlink'])
    
    def check_tool_availability(self, interface: str) -> bool:
        """Check if required programming tool is available"""
        interface_config = self.get_interface_config(interface)
        tool = interface_config['tool']
        
        try:
            result = subprocess.run(['which', tool], capture_output=True, text=True)
            if result.returncode == 0:
                print(f"Found {tool}: {result.stdout.strip()}")
                return True
            else:
                print(f"Tool {tool} not found in PATH")
                return False
        except Exception as e:
            print(f"Error checking tool availability: {e}")
            return False
    
    def probe_target(self, interface: str) -> Dict:
        """Probe for connected target device"""
        print(f"Probing for target using {interface}...")
        
        interface_config = self.get_interface_config(interface)
        probe_cmd = interface_config['probe_cmd']
        
        try:
            result = subprocess.run(probe_cmd, capture_output=True, text=True, timeout=10)
            
            if result.returncode == 0:
                print("Target device found:")
                print(result.stdout)
                return {
                    'found': True,
                    'output': result.stdout,
                    'error': None
                }
            else:
                print("No target device found:")
                print(result.stderr)
                return {
                    'found': False,
                    'output': result.stdout,
                    'error': result.stderr
                }
                
        except subprocess.TimeoutExpired:
            print("Probe timeout - device may not be connected")
            return {
                'found': False,
                'output': '',
                'error': 'Timeout'
            }
        except Exception as e:
            print(f"Error probing target: {e}")
            return {
                'found': False,
                'output': '',
                'error': str(e)
            }
    
    def find_firmware_file(self, file_type: str = 'bin') -> Optional[Path]:
        """Find firmware file in build directory"""
        
        # Look for firmware files
        patterns = [
            f"*.{file_type}",
            f"firmware.{file_type}",
            f"main.{file_type}"
        ]
        
        for pattern in patterns:
            files = list(self.build_dir.glob(pattern))
            if files:
                # Return the most recently modified file
                return max(files, key=lambda f: f.stat().st_mtime)
        
        return None
    
    def flash_stlink(self, firmware_file: Path) -> bool:
        """Flash using ST-Link interface"""
        print(f"Flashing {firmware_file.name} using ST-Link...")
        
        flash_base = self.config.get('flash_base', '0x08000000')
        
        cmd = ['st-flash', 'write', str(firmware_file), flash_base]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Flash successful!")
                print(result.stdout)
                return True
            else:
                print("Flash failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during flashing: {e}")
            return False
    
    def flash_jlink(self, firmware_file: Path) -> bool:
        """Flash using J-Link interface"""
        print(f"Flashing {firmware_file.name} using J-Link...")
        
        # Create J-Link script
        script_file = self.build_dir / "flash.jlink"
        target = self.config.get('target', 'stm32f4')
        flash_base = self.config.get('flash_base', '0x08000000')
        
        jlink_script = f"""
device {target.upper()}
si 1
speed 4000
r
h
loadfile {firmware_file.absolute()} {flash_base}
r
g
exit
"""
        
        with open(script_file, 'w') as f:
            f.write(jlink_script)
        
        cmd = ['JLinkExe', '-CommanderScript', str(script_file)]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Flash successful!")
                return True
            else:
                print("Flash failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during flashing: {e}")
            return False
    
    def flash_openocd(self, firmware_file: Path) -> bool:
        """Flash using OpenOCD"""
        print(f"Flashing {firmware_file.name} using OpenOCD...")
        
        target = self.config.get('target', 'stm32f4')
        flash_base = self.config.get('flash_base', '0x08000000')
        
        # Create OpenOCD config
        config_file = self.build_dir / "openocd.cfg"
        
        openocd_config = f"""
source [find interface/stlink.cfg]
source [find target/{target}x.cfg]
init
reset halt
flash write_image erase {firmware_file.absolute()} {flash_base}
reset run
shutdown
"""
        
        with open(config_file, 'w') as f:
            f.write(openocd_config)
        
        cmd = ['openocd', '-f', str(config_file)]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Flash successful!")
                return True
            else:
                print("Flash failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during flashing: {e}")
            return False
    
    def flash_dfu(self, firmware_file: Path) -> bool:
        """Flash using DFU (Device Firmware Update)"""
        print(f"Flashing {firmware_file.name} using DFU...")
        
        flash_base = self.config.get('flash_base', '0x08000000')
        
        cmd = [
            'dfu-util',
            '-a', '0',
            '-s', f'{flash_base}:leave',
            '-D', str(firmware_file)
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Flash successful!")
                print(result.stdout)
                return True
            else:
                print("Flash failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during flashing: {e}")
            return False
    
    def flash_esp32(self, firmware_file: Path) -> bool:
        """Flash ESP32 using esptool"""
        print(f"Flashing {firmware_file.name} to ESP32...")
        
        port = self.config.get('port', '/dev/ttyUSB0')
        baud = self.config.get('baud', '921600')
        
        cmd = [
            'esptool.py',
            '--chip', 'esp32',
            '--port', port,
            '--baud', str(baud),
            'write_flash',
            '-z',
            '0x1000', str(firmware_file)
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Flash successful!")
                print(result.stdout)
                return True
            else:
                print("Flash failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during flashing: {e}")
            return False
    
    def flash_msp430(self, firmware_file: Path) -> bool:
        """Flash MSP430 using mspdebug"""
        print(f"Flashing {firmware_file.name} to MSP430...")
        
        programmer = self.config.get('programmer', 'rf2500')
        
        cmd = ['mspdebug', programmer, 'prog', str(firmware_file)]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Flash successful!")
                print(result.stdout)
                return True
            else:
                print("Flash failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during flashing: {e}")
            return False
    
    def erase_flash(self, interface: str) -> bool:
        """Erase target flash memory"""
        print(f"Erasing flash using {interface}...")
        
        interface_config = self.get_interface_config(interface)
        
        if interface == 'stlink':
            cmd = ['st-flash', 'erase']
        elif interface == 'jlink':
            # Create J-Link erase script
            script_file = self.build_dir / "erase.jlink"
            target = self.config.get('target', 'stm32f4')
            
            jlink_script = f"""
device {target.upper()}
si 1
speed 4000
r
h
erase
exit
"""
            with open(script_file, 'w') as f:
                f.write(jlink_script)
            
            cmd = ['JLinkExe', '-CommanderScript', str(script_file)]
        elif interface == 'dfu':
            flash_base = self.config.get('flash_base', '0x08000000')
            cmd = ['dfu-util', '-a', '0', '-s', f'{flash_base}:mass-erase:force']
        elif interface == 'esp32':
            cmd = ['esptool.py', '--chip', 'esp32', 'erase_flash']
        else:
            print(f"Erase not implemented for {interface}")
            return False
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Erase successful!")
                return True
            else:
                print("Erase failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during erase: {e}")
            return False
    
    def reset_target(self, interface: str) -> bool:
        """Reset target device"""
        print(f"Resetting target using {interface}...")
        
        if interface == 'stlink':
            cmd = ['st-flash', 'reset']
        elif interface == 'jlink':
            # Create J-Link reset script
            script_file = self.build_dir / "reset.jlink"
            target = self.config.get('target', 'stm32f4')
            
            jlink_script = f"""
device {target.upper()}
si 1
speed 4000
r
g
exit
"""
            with open(script_file, 'w') as f:
                f.write(jlink_script)
            
            cmd = ['JLinkExe', '-CommanderScript', str(script_file)]
        elif interface == 'esp32':
            cmd = ['esptool.py', '--chip', 'esp32', 'run']
        else:
            print(f"Reset not implemented for {interface}")
            return False
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Reset successful!")
                return True
            else:
                print("Reset failed:")
                print(result.stderr)
                return False
                
        except Exception as e:
            print(f"Error during reset: {e}")
            return False
    
    def flash(self, interface: str, firmware_file: Optional[Path] = None, 
              file_type: str = 'bin') -> Dict:
        """Perform complete flash operation"""
        
        start_time = time.time()
        print(f"Starting flash operation using {interface}")
        
        try:
            # Check tool availability
            if not self.check_tool_availability(interface):
                return {
                    'success': False,
                    'error': f'Programming tool for {interface} not available',
                    'flash_time': 0
                }
            
            # Find firmware file if not specified
            if firmware_file is None:
                firmware_file = self.find_firmware_file(file_type)
                
                if firmware_file is None:
                    return {
                        'success': False,
                        'error': f'No {file_type} file found in build directory',
                        'flash_time': 0
                    }
            
            print(f"Using firmware file: {firmware_file}")
            
            # Probe target
            probe_result = self.probe_target(interface)
            if not probe_result['found']:
                return {
                    'success': False,
                    'error': f'Target device not found: {probe_result["error"]}',
                    'flash_time': 0
                }
            
            # Erase if requested
            if self.config.get('erase_before_flash', False):
                if not self.erase_flash(interface):
                    return {
                        'success': False,
                        'error': 'Flash erase failed',
                        'flash_time': time.time() - start_time
                    }
            
            # Flash firmware
            flash_functions = {
                'stlink': self.flash_stlink,
                'jlink': self.flash_jlink,
                'openocd': self.flash_openocd,
                'dfu': self.flash_dfu,
                'esp32': self.flash_esp32,
                'msp430': self.flash_msp430
            }
            
            flash_func = flash_functions.get(interface)
            if flash_func is None:
                return {
                    'success': False,
                    'error': f'Flashing not implemented for {interface}',
                    'flash_time': 0
                }
            
            if not flash_func(firmware_file):
                return {
                    'success': False,
                    'error': 'Flash operation failed',
                    'flash_time': time.time() - start_time
                }
            
            # Reset if requested
            if self.config.get('reset_after_flash', True):
                self.reset_target(interface)
            
            flash_time = time.time() - start_time
            print(f"\nFlash completed successfully in {flash_time:.2f} seconds")
            
            return {
                'success': True,
                'firmware_file': str(firmware_file),
                'flash_time': flash_time
            }
            
        except Exception as e:
            return {
                'success': False,
                'error': str(e),
                'flash_time': time.time() - start_time
            }


def main():
    parser = argparse.ArgumentParser(description='QP-QK SDK Flash Tool')
    parser.add_argument('--project', '-p', default='.',
                       help='Project directory (default: current directory)')
    parser.add_argument('--interface', '-i', default='stlink',
                       choices=['stlink', 'jlink', 'openocd', 'dfu', 'esp32', 'msp430'],
                       help='Programming interface (default: stlink)')
    parser.add_argument('--file', '-f', type=str,
                       help='Firmware file to flash (auto-detect if not specified)')
    parser.add_argument('--type', '-t', default='bin',
                       choices=['bin', 'hex', 'elf'],
                       help='Firmware file type (default: bin)')
    parser.add_argument('--erase', action='store_true',
                       help='Erase flash before programming')
    parser.add_argument('--no-reset', action='store_true',
                       help='Do not reset target after flashing')
    parser.add_argument('--probe-only', action='store_true',
                       help='Only probe for target, do not flash')
    
    args = parser.parse_args()
    
    # Create flasher
    try:
        flasher = QKFlasher(args.project)
    except Exception as e:
        print(f"Error initializing flasher: {e}")
        sys.exit(1)
    
    # Update config based on arguments
    if args.erase:
        flasher.config['erase_before_flash'] = True
    if args.no_reset:
        flasher.config['reset_after_flash'] = False
    
    # Probe only if requested
    if args.probe_only:
        result = flasher.probe_target(args.interface)
        sys.exit(0 if result['found'] else 1)
    
    # Flash firmware
    firmware_file = Path(args.file) if args.file else None
    result = flasher.flash(args.interface, firmware_file, args.type)
    
    if not result['success']:
        print(f"Flash failed: {result['error']}")
        sys.exit(1)


if __name__ == '__main__':
    main()