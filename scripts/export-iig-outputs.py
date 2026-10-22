#!/usr/bin/env python3
"""Collect the DriverKit files Apple's iig generated during a macOS kernel build, for kernel builds on Linux.

  export-iig-outputs.py OUTPUT_DIR

Reads the iig outputs (DriverKit/<Name>.h and <Name>.iig.cpp) of the XNU kernel build in this build tree and writes
them to OUTPUT_DIR with manifest.json: per .iig, the sha256 of the source it was generated from, and the iig version
banner. scripts/unixnu/host/iig-replay serves them on Linux only for a source with the same sha256.
"""
import hashlib
import json
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / '.cache/xnu-work/iokit/DriverKit'
# The kernel build's own copies first (they are compiled into the kernel); installhdrs makes the same files.
CANDIDATES = [*sorted((ROOT / 'build/xnu/ios-obj').glob('*/iokit/*/DriverKit')),
              *sorted((ROOT / 'build/xnu/obj').glob('*/iokit/DriverKit'))]


def main():
    out = Path(sys.argv[1])
    sources = sorted(SOURCE.glob('*.iig'))
    if not sources:
        raise SystemExit(f'no .iig sources in {SOURCE}')
    directory = next((d for d in CANDIDATES if all((d / f'{s.stem}.h').is_file() and (d / f'{s.stem}.iig.cpp').is_file()
                                                    for s in sources)), None)
    if directory is None:
        raise SystemExit('no complete set of iig outputs in build/xnu; run the kernel step on macOS first')
    out.mkdir(parents=True, exist_ok=True)
    manifest = {'from': str(directory.relative_to(ROOT)), 'files': {}}
    for source in sources:
        header, impl = directory / f'{source.stem}.h', directory / f'{source.stem}.iig.cpp'
        shutil.copyfile(header, out / header.name)
        shutil.copyfile(impl, out / impl.name)
        manifest['files'][source.name] = {
            'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
            'iig': header.read_text(errors='replace').splitlines()[0],
            'header_sha256': hashlib.sha256(header.read_bytes()).hexdigest(),
            'impl_sha256': hashlib.sha256(impl.read_bytes()).hexdigest()}
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'{len(sources)} iig outputs from {directory} -> {out}')


if __name__ == '__main__':
    main()
