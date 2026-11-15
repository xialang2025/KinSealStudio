// Export selected ARM consumers with assembly to check decompiler limitations. //20261115-0 add
// Input: destination followed by virtual addresses. Output: bounded JSON evidence. Date: 2026.11.15.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import com.google.gson.*;
import java.util.*;
import java.nio.file.*;

public class FirmwareDetails extends GhidraScript {
    /******************************************************************************
     * Function Name   : run
     * Description     : Export selected consumers, literal words and callers.
     * Input Parameters: Script arguments: JSON destination and VA selectors.
     * Return Value    : None; writes bounded JSON static-analysis evidence.
     * Date            : 2026.11.15
     ******************************************************************************/
    public void run() throws Exception {
        String[] args=getScriptArgs();
        List<Map<String,Object>> functions=new ArrayList<>();
        DecompInterface decompiler=new DecompInterface(); decompiler.openProgram(currentProgram);
        for(int n=1;n<args.length;n++) {
            monitor.checkCancelled();
            if(args[n].startsWith("@")) {
                Address data=toAddr(args[n].substring(1));
                Map<String,Object> row=new LinkedHashMap<>(); row.put("dataAddress",data.toString());
                row.put("value32","0x"+Integer.toHexString(getInt(data)));
                List<String> references=new ArrayList<>();
                for(Reference ref:getReferencesTo(data)) {
                    Function owner=getFunctionContaining(ref.getFromAddress());
                    references.add(ref.getFromAddress()+" "+ref.getReferenceType()+" owner="+(owner==null?"unknown":owner.getEntryPoint()));
                }
                row.put("references",references); functions.add(row); continue;
            }
            Address requested=toAddr(args[n]);
            Function function=getFunctionContaining(requested);
            Map<String,Object> row=new LinkedHashMap<>(); row.put("requested",args[n]);
            if(function==null) { row.put("error","No containing function"); functions.add(row); continue; }
            row.put("address",function.getEntryPoint().toString());row.put("body",function.getBody().toString());
            List<String> assembly=new ArrayList<>(); Map<String,String> literals=new LinkedHashMap<>();
            InstructionIterator instructions=currentProgram.getListing().getInstructions(function.getBody(),true);
            while(instructions.hasNext()) {
                Instruction instruction=instructions.next();
                assembly.add(instruction.getAddress()+" "+instruction.toString());
                for(Reference ref:instruction.getReferencesFrom()) {
                    Address to=ref.getToAddress();
                    if(ref.getReferenceType().isData() && currentProgram.getMemory().contains(to)) {
                        try { literals.put(to.toString(),"0x"+Integer.toHexString(getInt(to))); } catch(Exception ex) { }
                    }
                }
            }
            row.put("assembly",assembly);row.put("literalWords",literals);
            DecompileResults result=decompiler.decompileFunction(function,7,monitor);
            row.put("decompileComplete",result.decompileCompleted());row.put("decompileError",result.getErrorMessage());
            if(result.getDecompiledFunction()!=null)row.put("pseudocode",result.getDecompiledFunction().getC());
            List<String> incoming=new ArrayList<>();
            for(Reference ref:getReferencesTo(function.getEntryPoint())) {
                Function caller=getFunctionContaining(ref.getFromAddress());
                incoming.add(ref.getFromAddress()+" "+ref.getReferenceType()+" owner="+(caller==null?"unknown":caller.getEntryPoint()));
            }
            row.put("incoming",incoming);
            List<String> callees=new ArrayList<>();
            for(Function callee:function.getCalledFunctions(monitor))callees.add(callee.getEntryPoint().toString());
            row.put("callees",callees);functions.add(row);
        }
        decompiler.dispose();
        Map<String,Object> output=new LinkedHashMap<>();
        output.put("provider","Direct Ghidra 12.1.4; no REA evidence ID");
        output.put("sourceSha256","74d510f150dc8edadf8e967bdb3c00ca5790ca68b9da6f906125a5234a485c24");
        output.put("limitations","Static assembly and inferred decompilation only; validate tail calls, calling convention and condition flags in assembly");
        output.put("functions",functions);
        Files.writeString(Path.of(args[0]),new GsonBuilder().setPrettyPrinting().create().toJson(output)+"\n");
        println("Exported selected functions="+functions.size());
    }
}
