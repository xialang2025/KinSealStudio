"""Build the observed native download lifecycle without treating a UI row as USB completion."""
import re
import textwrap


def build_hardware_block(original):
    """Input: Retained hardware recipe. Return: Recipe waiting for observed commit ACK and reset write."""
    #20261113-3 add: Modify the retained recipe in memory; never edit the user's Stage3 script.
    before = "                    button = widget({'name':'m_downloadbtn','kind':'QPushButton'})"
    after = """                    download_table = widget({'name':'m_table','kind':'QTableWidget'})
                    transfer_options = []
                    for option_name in ('clear_recipe_CB','clear_hisData_CB','clear_hisAlarm_CB','update_CB'):
                        option = widget({'name':option_name,'kind':'QCheckBox'})
                        initially_checked = option['qt'].get('checked') == 'true'
                        if initially_checked:
                            op({'op':'invoke','hwnd':option['hwnd'],'slot':'click'})
                            time.sleep(.15)
                        state = op({'op':'batch','hwnds':[option['hwnd']]})[0]['qt']
                        assert state.get('checked') == 'false', 'Clear-history/firmware-update option must be disabled'
                        transfer_options.append({'name':option_name,'beforeChecked':initially_checked,'afterChecked':False})
                    usb_trace_start = len(result['usbTrace']['events'])
                    button = widget({'name':'m_downloadbtn','kind':'QPushButton'})"""
    assert original.count(before) == 1
    hardware = original.replace(before, after)
    before = "                    result['nativeHardware'] = hardware"
    after = before + "\n                    hardware['transferOptions'] = transfer_options\n                    hardware['usbTraceStartIndex'] = usb_trace_start\n                    hardware['completionLogWidget'] = 'DownLoadDlg/m_table'"
    assert hardware.count(before) == 1
    hardware = hardware.replace(before, after)
    before = "                        current_tables = [op({'op':'table-read','hwnd':w['hwnd']}) for w in tables]"
    assert hardware.count(before) == 1
    hardware = hardware.replace(before, "                        current_tables = [op({'op':'table-read','hwnd':download_table['hwnd']})]")
    before = """                        if any(word in text for word in ('下载成功','下载完成','传输完成')):
                            hardware['completed'] = True
                            break"""
    after = """                        if any(word in text for word in ('下载成功','下载完成','传输完成')):
                            hardware.setdefault('uiCompletedEpoch', time.time())
                        completion_events = list(result['usbTrace']['events'])
                        completion_hooks = [event for event in completion_events if event.get('kind') == 'usb-hook-ready']
                        completion = classify_native_usb_completion(completion_hooks + completion_events[usb_trace_start:])
                        hardware['usbCompletion'] = completion
                        if completion['ready']:
                            hardware['completed'] = True
                            break"""
    assert hardware.count(before) == 1
    return hardware.replace(before, after)


def instrument_probe_source(source, trace_js, classifier_source):
    """Input: Owned probe and read-only agent. Return: Probe retaining raw calls and termination timing."""
    # Embed the classifier so the generated probe remains replayable after runtime cleanup.
    assert source.count('JS = r"""') == 1
    source = source.replace('JS = r"""', classifier_source + '\n\nUSB_TRACE_JS = ' + repr(trace_js) + '\n\nJS = r"""', 1)
    setup = textwrap.indent(textwrap.dedent('''\
result['usbTrace'] = {'events': [], 'agentSha256': hashlib.sha256(USB_TRACE_JS.encode('utf-8')).hexdigest(), 'observationOnly': True}
trace_directory = OUT / 'usb-raw'
trace_directory.mkdir(exist_ok=True)
def capture_usb(message, data):
    event = dict(message.get('payload', {})) if message.get('type') == 'send' else {'instrumentationError':message}
    event['hostEpochNs'] = time.time_ns()
    if data:
        filename = 'event-%06d-%s.bin' % (event['id'], event['phase'])
        path = trace_directory / filename
        path.write_bytes(data)
        event.update(rawPath='usb-raw/' + filename, bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
        if len(data) <= 64:
            event['dataHex'] = data.hex()
    result['usbTrace']['events'].append(event)
usb_trace_script = session.create_script(USB_TRACE_JS)
usb_trace_script.on('message', capture_usb)
usb_trace_script.load()
'''), '        ')
    assert source.count('        script.load()') == 1
    source = source.replace('        script.load()', '        script.load()\n' + setup, 1)
    pattern = r'(?m)^( +)win32process\.TerminateProcess\(process, 0\)$'
    def record_termination(match):
        """Input: One owned termination site. Return: Timestamped call with its original indentation."""
        indent = match.group(1)
        return (indent + "result.setdefault('ownedProcessTerminateRequestedEpoch', time.time())\n" +
                match.group(0) + '\n' + indent + "result['ownedProcessTerminateReturnedEpoch'] = time.time()")
    source, replacements = re.subn(pattern, record_termination, source)
    assert replacements >= 1
    compile(source, '<native-usb-probe>', 'exec')
    return source
