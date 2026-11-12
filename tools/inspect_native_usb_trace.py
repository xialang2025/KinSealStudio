"""Verify retained USB bytes and report transfer completion without opening a device."""
import argparse
import hashlib
import json
from pathlib import Path

from usb_transfer_completion import classify_native_usb_completion


def inspect_run(directory):
    """Input: Ended owned run directory. Return: Raw-byte identity and observed completion summary."""
    #20261113-3 add: Preserve original calls before decoding; tolerate a call interrupted by the old process cleanup.
    directory = directory.resolve()
    snapshot = directory / 'usb-events.json'
    record = json.loads((directory / 'run.json').read_text(encoding='utf-8'))
    if snapshot.exists():
        document = json.loads(snapshot.read_text(encoding='utf-8'))
    else:
        original = (directory / 'native-probe.json').read_bytes()
        probe = json.loads(original)
        hardware = probe.get('nativeHardware', {})
        document = dict(derivedFrom='native-probe.json', derivedFromSha256=hashlib.sha256(original).hexdigest(),
                        uiCompletedEpochMs=hardware.get('uiCompletedEpoch', hardware.get('completedEpoch', 0)) * 1000,
                        events=probe['usbTrace']['events'])
        snapshot.write_text(json.dumps(document, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    events = []
    raw_files = set()
    for original in document['events']:
        event = dict(original)
        if event.get('rawPath'):
            path = (directory / event['rawPath']).resolve()
            if not path.is_relative_to(directory):
                raise ValueError('Raw capture path escapes its run directory')
            data = path.read_bytes()
            if len(data) != event['bytes'] or hashlib.sha256(data).hexdigest() != event['sha256']:
                raise ValueError('Raw USB capture identity mismatch: ' + str(path))
            raw_files.add(str(path))
            if len(data) <= 64:
                event['dataHex'] = data.hex()
        events.append(event)
    start = record.get('nativeHardware', {}).get('usbTraceStartIndex', 0)
    hooks = [e for e in events if e.get('kind') == 'usb-hook-ready']
    completion = classify_native_usb_completion(hooks + events[start:])
    return dict(sourceSha256=record['sourceSha256'], rawFiles=len(raw_files), rawBytesShaVerified=True,
                completion=completion, uiCompletedEpochMs=document['uiCompletedEpochMs'],
                ownedProcessTerminateRequestedEpoch=record.get('ownedProcessTerminateRequestedEpoch'),
                screenResult='see screen-result.json' if (directory / 'screen-result.json').exists() else 'awaiting_user')


def main():
    """Input: Ended run path. Return: Zero after writing a derived summary; readiness is explicit in JSON."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('run', type=Path)
    args = parser.parse_args()
    result = inspect_run(args.run)
    (args.run / 'completion-analysis.json').write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: v for k, v in result.items() if k != 'completion'} | {'completion': {k: v for k, v in result['completion'].items() if k != 'commands'}}, ensure_ascii=False))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
