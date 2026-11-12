"""Classify native USB completion from paired raw calls, independently of premature UI log text."""


def classify_native_usb_completion(events):
    """Input: Ordered observed libusb events. Return: Model, commit, reset and capture readiness without USB access."""
    #20261113-3 add: A native UI completion row can precede the reset write by more than one second.
    pairs = {}
    hooks = set()
    errors = []
    warnings = []
    expected_reset_reads = []
    commands = []
    pending = None
    response = bytearray()
    response_checked = False
    model_confirmed = False
    parameters_confirmed = False
    start_acknowledged = False
    commit_acknowledged = False
    reset_written = False
    reset_read_completed = False
    commit_call_id = None
    reset_call_id = None
    clear_flags = None

    for event in events:
        if event.get('kind') == 'usb-hook-ready':
            hooks.add(event.get('name'))
        if event.get('captureError') or event.get('instrumentationError'):
            errors.append({'kind': 'trace_capture_error', 'event': event})
        if event.get('kind') != 'usb-call':
            continue
        pair = pairs.setdefault(event.get('id'), {})
        if event.get('phase') in pair:
            errors.append({'kind': 'duplicate_call_phase', 'callId': event.get('id')})
        pair[event.get('phase')] = event

    # Dict insertion order retains call-entry order; transport read boundaries never define reply boundaries.
    for identity, pair in pairs.items():
        enter = pair.get('enter')
        leave = pair.get('leave')
        if enter is None or leave is None:
            if enter and enter.get('direction') == 'out':
                errors.append({'kind': 'unfinished_write', 'callId': identity})
            continue
        returned = leave.get('returned', -1)
        if enter.get('direction') == 'out':
            if returned != enter.get('requested'):
                errors.append({'kind': 'short_or_failed_write', 'callId': identity, 'requested': enter.get('requested'), 'returned': returned})
                continue
            try:
                data = bytes.fromhex(enter.get('dataHex', ''))
            except ValueError:
                errors.append({'kind': 'malformed_write_capture', 'callId': identity})
                continue
            if len(data) != 32 or not data.startswith(b'KinSealStudioKiss'):
                continue
            if pending in (0x31, 0x53) and response and not response_checked:
                warnings.append({'kind': 'incomplete_identification_before_retry', 'command': f'{pending:02X}', 'replyHex': response.hex()})
            elif pending not in (None, 0x31, 0x53, 0x1A) and not response_checked:
                errors.append({'kind': 'command_without_ack', 'command': f'{pending:02X}', 'callId': identity})
            pending = data[17]
            response = bytearray()
            response_checked = False
            commands.append({'code': f'{pending:02X}', 'callId': identity, 'epochMs': enter.get('epochMs')})
            if pending == 0x34:
                clear_flags = data[18:21].hex()
                if clear_flags != '000000':
                    errors.append({'kind': 'unexpected_clear_flags', 'callId': identity, 'flagsHex': clear_flags})
                if not model_confirmed or not parameters_confirmed:
                    errors.append({'kind': 'download_started_without_confirmed_device', 'callId': identity})
            if pending == 0x1A:
                reset_call_id = identity
                reset_written = commit_acknowledged and start_acknowledged
                if not reset_written:
                    errors.append({'kind': 'reset_before_acknowledged_commit', 'callId': identity})
            continue

        if returned < 0:
            record = {'callId': identity, 'returned': returned, 'command': f'{pending:02X}' if pending is not None else None}
            if pending == 0x1A and reset_written and returned == -5:
                expected_reset_reads.append(record)
                reset_read_completed = True
            elif pending in (0x31, 0x53) and not start_acknowledged:
                warnings.append(dict(record, kind='identification_read_retry'))
            else:
                errors.append(dict(record, kind='failed_read_before_reset'))
            continue
        if returned == 0:
            continue
        try:
            data = bytes.fromhex(leave.get('dataHex', ''))
        except ValueError:
            data = b''
        if len(data) != returned:
            errors.append({'kind': 'missing_or_truncated_read_capture', 'callId': identity, 'returned': returned})
            continue
        response.extend(data)
        if response_checked or pending is None or pending == 0x1A:
            continue
        expected = 24 if pending in (0x31, 0x53) else 1
        if len(response) < expected:
            continue
        response_checked = True
        if pending == 0x31:
            model_confirmed = response[0] == 0x4C
            if not model_confirmed:
                warnings.append({'kind': 'unexpected_model_reply', 'replyHex': response[:24].hex()})
        elif pending == 0x53:
            parameters_confirmed = response[0] == 0
            if not parameters_confirmed:
                warnings.append({'kind': 'unexpected_device_parameter_reply', 'replyHex': response[:24].hex()})
        elif response[0] != 0x4F:
            errors.append({'kind': 'unexpected_command_ack', 'command': f'{pending:02X}', 'replyHex': response.hex()})
        elif pending == 0x34:
            start_acknowledged = True
        elif pending == 0x1B:
            commit_acknowledged = True
            commit_call_id = identity

    ready = hooks.issuperset({'usb_bulk_write', 'usb_bulk_read'}) and model_confirmed and parameters_confirmed and start_acknowledged and commit_acknowledged and reset_written and reset_read_completed and not errors
    return {'ready': ready, 'modelConfirmed': model_confirmed, 'parametersConfirmed': parameters_confirmed,
            'startAcknowledged': start_acknowledged, 'commitAcknowledged': commit_acknowledged,
            'resetWriteCompleted': reset_written, 'resetReadCompleted': reset_read_completed, 'commitAckReadCallId': commit_call_id, 'resetCallId': reset_call_id,
            'clearFlagsHex': clear_flags, 'commands': commands, 'warnings': warnings, 'errors': errors,
            'expectedResetReadFailures': expected_reset_reads, 'inFlightCalls': sum('enter' not in p or 'leave' not in p for p in pairs.values())}
