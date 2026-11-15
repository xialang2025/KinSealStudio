// Seed only observed startup paths and bounded ARM prologue candidates. //20261115-0 add
// Input: reconstructed analysis image. Output: private Ghidra database metadata. Date: 2026.11.15.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.SourceType;
import java.math.BigInteger;

public class FirmwareSetup extends GhidraScript {
    /******************************************************************************
     * Function Name   : run
     * Description     : Seed startup and bounded ARM prologue candidates.
     * Input Parameters: None; uses the private imported memory image.
     * Return Value    : None; adds candidate metadata to the private project.
     * Date            : 2026.11.15
     ******************************************************************************/
    public void run() throws Exception {
        long base = 0x8001ffe0L;
        createLabel(toAddr(0x80020000L), "observed_ARM_startup", true);
        addEntryPoint(toAddr(0x80020000L));
        currentProgram.getProgramContext().setValue(currentProgram.getRegister("TMode"), toAddr(0x80020000L), toAddr(0x80020000L), BigInteger.ZERO);
        disassemble(toAddr(0x80020000L));
        createFunction(toAddr(0x80020000L), "observed_ARM_startup");
        int seeded = 0;
        // Prologue seeds are candidates, never treated as original function symbols.
        for (long off = 0x20; off < 0x88000; off += 4) {
            monitor.checkCancelled();
            Address address = toAddr(base + off);
            int value = getInt(address);
            if ((value & 0xffff4000) == 0xe92d4000) {
                if (getFunctionContaining(address) == null) {
                    disassemble(address);
                    if (createFunction(address, null) != null) seeded++;
                }
            }
        }
        println("ARM prologue candidate seeds=" + seeded);
    }
}
