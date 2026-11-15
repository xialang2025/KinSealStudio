// Add an observed task entry that lacks a conventional PUSH prologue. //20261115-0 add
// Input: none. Output: private analysis function only. Date: 2026.11.15.
import ghidra.app.script.GhidraScript;
public class FirmwareSeedExtra extends GhidraScript {
    /******************************************************************************
     * Function Name   : run
     * Description     : Seed an observed task entry without a PUSH prologue.
     * Input Parameters: None; uses the private imported memory image.
     * Return Value    : None; adds candidate metadata to the private project.
     * Date            : 2026.11.15
     ******************************************************************************/
    public void run() throws Exception {
        disassemble(toAddr(0x80082248L));
        if(getFunctionAt(toAddr(0x80082248L))==null)
            createFunction(toAddr(0x80082248L),"observed_touch_timer_task_candidate");
    }
}
