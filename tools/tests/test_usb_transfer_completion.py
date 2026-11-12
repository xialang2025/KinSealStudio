"""Replay the immutable device trace and the actual native hardware call site without USB access."""
import ast
import copy
import hashlib
import json
from pathlib import Path
import sys
import textwrap
from types import SimpleNamespace
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
from native_usb_probe_lifecycle import build_hardware_block, instrument_probe_source
from usb_transfer_completion import classify_native_usb_completion

FIXTURE = ROOT / 'docs/stage4-evidence/screen-width-diagnosis/physical/marker-a-r1'


def captured_events():
    """Input: None. Return: SHA-verified captured calls with small raw responses available to the decoder."""
    document = json.loads((FIXTURE / 'usb-events.json').read_text(encoding='utf-8'))
    events = copy.deepcopy(document['events'])
    for event in events:
        if event.get('rawPath'):
            path = (FIXTURE / event['rawPath']).resolve()
            assert path.is_relative_to(FIXTURE.resolve())
            data = path.read_bytes()
            assert len(data) == event['bytes']
            assert hashlib.sha256(data).hexdigest() == event['sha256']
            if len(data) <= 64:
                event['dataHex'] = data.hex()
    return events


class NativeUsbCompletionTests(unittest.TestCase):
    def setUp(self):
        """Input: None. Return: A fresh copy of the complete real transfer."""
        self.events = captured_events()

    def test_complete_real_trace(self):
        result = classify_native_usb_completion(self.events)
        self.assertTrue(result['ready'], result)
        self.assertEqual(result['clearFlagsHex'], '000000')
        self.assertEqual(result['errors'], [])

    def test_ui_completion_alone_is_pending(self):
        document = json.loads((FIXTURE / 'usb-events.json').read_text(encoding='utf-8'))
        cutoff = document['uiCompletedEpochMs']
        events = [e for e in self.events if e.get('epochMs', 0) <= cutoff]
        result = classify_native_usb_completion(events)
        self.assertFalse(result['commitAcknowledged'])
        self.assertFalse(result['resetWriteCompleted'])
        self.assertFalse(result['ready'])

    def test_missing_commit_ack(self):
        result = classify_native_usb_completion([e for e in self.events if e.get('id') != 65])
        self.assertFalse(result['ready'])
        self.assertFalse(result['commitAcknowledged'])

    def test_missing_reset(self):
        result = classify_native_usb_completion([e for e in self.events if e.get('id') not in (66, 67)])
        self.assertFalse(result['ready'])

    def test_real_legacy_trace_stops_with_restart_read_in_flight(self):
        base = FIXTURE.parent / 'marker-b-legacy-r1'
        document = json.loads((base / 'usb-events.json').read_text(encoding='utf-8'))
        events = copy.deepcopy(document['events'])
        for event in events:
            if event.get('rawPath'):
                data = (base / event['rawPath']).read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), event['sha256'])
                if len(data) <= 64:
                    event['dataHex'] = data.hex()
        result = classify_native_usb_completion(events)
        self.assertTrue(result['resetWriteCompleted'])
        self.assertEqual(result['inFlightCalls'], 1)
        self.assertFalse(result['ready'])

    def test_reset_short_write(self):
        for event in self.events:
            if event.get('id') == 66 and event.get('phase') == 'leave':
                event['returned'] = 31
        result = classify_native_usb_completion(self.events)
        self.assertFalse(result['ready'])
        self.assertIn('short_or_failed_write', [e['kind'] for e in result['errors']])

    def test_reset_enter_without_return(self):
        result = classify_native_usb_completion([e for e in self.events if not (e.get('id') == 66 and e.get('phase') == 'leave')])
        self.assertFalse(result['ready'])

    def test_identification_reassembles_fragmented_reads(self):
        fragmented = []
        for event in self.events:
            if event.get('id') != 4:
                fragmented.append(event)
            elif event['phase'] == 'enter':
                enter = event
            else:
                payload = bytes.fromhex(event['dataHex'])
                for index, data in enumerate((payload[:1], payload[1:7], payload[7:])):
                    fragment_enter = dict(enter, id=400 + index, requested=len(data))
                    fragment_leave = dict(event, id=400 + index, requested=len(data), returned=len(data), dataHex=data.hex())
                    fragmented.extend([fragment_enter, fragment_leave])
        self.assertTrue(classify_native_usb_completion(fragmented)['ready'])

    def test_residual_o_then_identification_retry(self):
        result = classify_native_usb_completion(self.events)
        self.assertTrue(result['ready'])
        self.assertIn('incomplete_identification_before_retry', [e['kind'] for e in result['warnings']])

    def test_disconnect_after_reset_is_expected(self):
        result = classify_native_usb_completion(self.events)
        self.assertTrue(result['ready'])
        self.assertEqual([e['returned'] for e in result['expectedResetReadFailures']], [-5])

    def test_negative_read_before_reset_rejected(self):
        for event in self.events:
            if event.get('id') == 65 and event.get('phase') == 'leave':
                event['returned'] = -5
        result = classify_native_usb_completion(self.events)
        self.assertFalse(result['ready'])
        self.assertIn('failed_read_before_reset', [e['kind'] for e in result['errors']])

    def test_missing_hook_rejected(self):
        events = [e for e in self.events if not (e.get('kind') == 'usb-hook-ready' and e.get('name') == 'usb_bulk_read')]
        self.assertFalse(classify_native_usb_completion(events)['ready'])

    def test_clear_flags_rejected(self):
        for event in self.events:
            if event.get('id') == 7 and event.get('phase') == 'enter':
                data = bytearray.fromhex(event['dataHex'])
                data[18:21] = b'\x01\x01\x01'
                event['dataHex'] = data.hex()
        self.assertFalse(classify_native_usb_completion(self.events)['ready'])

    def test_instrumentation_error_rejected(self):
        self.events.append({'instrumentationError': 'capture unavailable'})
        self.assertFalse(classify_native_usb_completion(self.events)['ready'])

    def test_missing_command_ack_rejected(self):
        self.assertFalse(classify_native_usb_completion([e for e in self.events if e.get('id') != 10])['ready'])

    def test_missing_model_reply_rejected(self):
        self.assertFalse(classify_native_usb_completion([e for e in self.events if e.get('id') != 4])['ready'])

    def test_real_generated_call_site_waits_for_deferred_reset(self):
        #20261113-3 add: Execute the actual old recipe branch with only the completion predicate replaced.
        original = (FIXTURE / 'hardware-recipe-before.txt').read_text(encoding='utf-8')
        tree = ast.parse(textwrap.dedent(build_hardware_block(original)))
        nodes = [n for n in ast.walk(tree) if isinstance(n, ast.If) and ast.unparse(n.test) == "command.get('download')"]
        self.assertEqual(len(nodes), 1)
        branch = nodes[0]
        state = {'usbTrace': {'events': []}}
        all_events = self.events
        cutoff = json.loads((FIXTURE / 'usb-events.json').read_text(encoding='utf-8'))['uiCompletedEpochMs']
        early_events = [e for e in all_events if e.get('epochMs', 0) <= cutoff]
        class FakeTime:
            now = 1000.0
            def time(self): return self.now
            def sleep(self, seconds):
                self.now += seconds
                state['usbTrace']['events'] = all_events if self.now >= 1001.5 else early_events
        clock = FakeTime()
        def operation(request):
            if request['op'] == 'batch': return [{'qt': {'checked': 'false'}}]
            if request['op'] == 'table-read': return [['下载完成']]
            return True
        context = dict(command={'download': True}, args=SimpleNamespace(download=True), result=state, time=clock,
                       widget=lambda request: {'hwnd': request['name'], 'qt': {'checked': 'false'}}, op=operation,
                       hashlib=hashlib, source=FIXTURE / 'scenario.json', dump=lambda label: None,
                       classify_native_usb_completion=classify_native_usb_completion)
        exec(compile(ast.fix_missing_locations(ast.Module(body=[branch], type_ignores=[])), '<actual-hardware-branch>', 'exec'), context)
        self.assertTrue(state['nativeHardware']['completed'])
        self.assertGreaterEqual(clock.now, 1001.5)
        self.assertTrue(state['nativeHardware']['usbCompletion']['ready'])

    def test_generated_probe_keeps_valid_syntax_and_raw_capture(self):
        template = 'import hashlib, time\nJS = r"""agent"""\ndef run():\n        script.load()\n        try:\n            pass\n        finally:\n            if process:\n                win32process.TerminateProcess(process, 0)\n'
        source = instrument_probe_source(template, 'observation_only', (ROOT / 'tools/usb_transfer_completion.py').read_text(encoding='utf-8'))
        ast.parse(source)
        self.assertIn("event['dataHex'] = data.hex()", source)
        self.assertIn('ownedProcessTerminateRequestedEpoch', source)


if __name__ == '__main__':
    unittest.main()
