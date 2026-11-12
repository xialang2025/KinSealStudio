//20261113-3 add: Observe only the owned process's libusb calls; never mutate buffers or USB return values.
let callSequence = 0;
const installed = new Set();
Process.attachModuleObserver({
    onAdded(module) {
        if (!module.name.toLowerCase().includes('libusb0')) { return; }
        for (const name of ['usb_bulk_write', 'usb_bulk_read']) {
            const address = module.findExportByName(name);
            if (address === null || installed.has(address.toString())) { continue; }
            installed.add(address.toString());
            send({kind:'usb-hook-ready', module:module.name, path:module.path, name, address:address.toString(), epochMs:Date.now()});
            Interceptor.attach(address, {
                onEnter(args) {
                    this.id = ++callSequence; this.buffer = args[2]; this.requested = args[3].toInt32();
                    this.record = {kind:'usb-call', id:this.id, phase:'enter', name, direction:name === 'usb_bulk_write' ? 'out' : 'in', endpoint:args[1].toInt32(), requested:this.requested, timeoutMs:args[4].toInt32(), epochMs:Date.now(), threadId:Process.getCurrentThreadId()};
                    try { send(this.record, name === 'usb_bulk_write' && this.requested > 0 && this.requested <= 16 * 1024 * 1024 ? this.buffer.readByteArray(this.requested) : undefined); }
                    catch (error) { send({...this.record, captureError:String(error)}); }
                },
                onLeave(result) {
                    const returned = result.toInt32();
                    const record = {...this.record, phase:'leave', returned, epochMs:Date.now()};
                    try { send(record, name === 'usb_bulk_read' && returned > 0 && returned <= this.requested ? this.buffer.readByteArray(returned) : undefined); }
                    catch (error) { send({...record, captureError:String(error)}); }
                }
            });
        }
        const closeAddress = module.findExportByName('usb_close');
        if (closeAddress !== null && !installed.has(closeAddress.toString())) {
            installed.add(closeAddress.toString());
            send({kind:'usb-hook-ready', module:module.name, path:module.path, name:'usb_close', address:closeAddress.toString(), epochMs:Date.now()});
            Interceptor.attach(closeAddress, {
                onEnter(args) {
                    this.id = ++callSequence;
                    send({kind:'usb-close-call', id:this.id, phase:'enter', handle:args[0].toString(), epochMs:Date.now()});
                },
                onLeave(result) {
                    send({kind:'usb-close-call', id:this.id, phase:'leave', returned:result.toInt32(), epochMs:Date.now()});
                }
            });
        }
    }
});
