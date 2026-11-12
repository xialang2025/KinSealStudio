"""Reproduce only the old UI completion predicate for the explicitly named marker-B comparison."""
import sys
import run_stage4_native_physical as runner

assert '--family' in sys.argv and sys.argv[sys.argv.index('--family') + 1] == 'screen-width-diagnosis'
assert '--run' in sys.argv and sys.argv[sys.argv.index('--run') + 1] == 'marker-b-legacy-r1'
assert '--project' in sys.argv and sys.argv[sys.argv.index('--project') + 1].endswith('marker-b.sm')
fixed_builder = runner.build_hardware_block


def legacy_ui_completion(original):
    """Input: Same hardware recipe. Return: Same options and trace scope with the old completion predicate."""
    hardware = fixed_builder(original)
    before = """                        if any(word in text for word in ('下载成功','下载完成','传输完成')):
                            hardware.setdefault('uiCompletedEpoch', time.time())
                        completion_events = list(result['usbTrace']['events'])
                        completion_hooks = [event for event in completion_events if event.get('kind') == 'usb-hook-ready']
                        completion = classify_native_usb_completion(completion_hooks + completion_events[usb_trace_start:])
                        hardware['usbCompletion'] = completion
                        if completion['ready']:
                            hardware['completed'] = True
                            break"""
    after = """                        if any(word in text for word in ('下载成功','下载完成','传输完成')):
                            hardware['uiCompletedEpoch'] = time.time()
                            hardware['completed'] = True
                            break"""
    assert hardware.count(before) == 1
    return hardware.replace(before, after)


runner.build_hardware_block = legacy_ui_completion
raise SystemExit(runner.main())
