// Export bounded candidate consumer functions and raw references. //20261115-0 add
// Input: private analyzed program and destination JSON. Output: source-bound static evidence. Date: 2026.11.15.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import com.google.gson.*;
import java.util.*;
import java.nio.file.*;

public class FirmwareSurvey extends GhidraScript {
    /******************************************************************************
     * Function Name   : run
     * Description     : Locate candidate consumers using bounded field offsets.
     * Input Parameters: Script argument: JSON destination.
     * Return Value    : None; exports inferred functions and raw references.
     * Date            : 2026.11.15
     ******************************************************************************/
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Map<String,Object> output = new LinkedHashMap<>();
        output.put("provider", "Direct Ghidra 12.1.4 headless; not REA");
        output.put("sourceSha256", "74d510f150dc8edadf8e967bdb3c00ca5790ca68b9da6f906125a5234a485c24");
        output.put("warning", "Static candidate functions; inferred boundaries and types, no execution evidence");
        List<Map<String,Object>> candidates = new ArrayList<>();
        Set<Long> needles = new HashSet<>(Arrays.asList(0x106L,0x114L,0x118L,0x180L,0x184L,0x188L,0x198L,0x1bdL,0x1beL,0x280L));
        FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
        int total = 0;
        while (functions.hasNext()) {
            monitor.checkCancelled();
            Function function = functions.next(); total++;
            Set<Long> values = new TreeSet<>();
            List<String> hits = new ArrayList<>();
            InstructionIterator instructions = currentProgram.getListing().getInstructions(function.getBody(), true);
            while (instructions.hasNext()) {
                Instruction instruction = instructions.next();
                for (int op=0;op<instruction.getNumOperands();op++) {
                    for (Object object : instruction.getOpObjects(op)) {
                        if (object instanceof Scalar) {
                            long value = ((Scalar)object).getUnsignedValue();
                            if (needles.contains(value)) { values.add(value); hits.add(instruction.getAddress()+" "+instruction); }
                        }
                    }
                }
            }
            if (values.size()>=3 || values.contains(0x1beL) || (values.contains(0x106L)&&values.contains(0x280L))) {
                Map<String,Object> row = new LinkedHashMap<>();
                row.put("address",function.getEntryPoint().toString()); row.put("name",function.getName());
                row.put("bodySize",function.getBody().getNumAddresses()); row.put("score",values.size());
                row.put("values",values); row.put("assemblyHits",hits); candidates.add(row);
            }
        }
        candidates.sort((a,b)->Integer.compare((Integer)b.get("score"),(Integer)a.get("score")));
        output.put("functionCount",total); output.put("candidateCount",candidates.size());
        DecompInterface decompiler = new DecompInterface(); decompiler.openProgram(currentProgram);
        for(int i=0;i<Math.min(14,candidates.size());i++) {
            Map<String,Object> row=candidates.get(i);
            Function function=getFunctionAt(toAddr((String)row.get("address")));
            DecompileResults result=decompiler.decompileFunction(function,6,monitor);
            row.put("decompileComplete",result.decompileCompleted()); row.put("error",result.getErrorMessage());
            if(result.getDecompiledFunction()!=null) row.put("pseudocode",result.getDecompiledFunction().getC());
            List<String> callers=new ArrayList<>();
            for(Reference ref:getReferencesTo(function.getEntryPoint())) callers.add(ref.getFromAddress()+" "+ref.getReferenceType());
            row.put("incoming",callers);
            List<String> callees=new ArrayList<>();
            for(Function callee:function.getCalledFunctions(monitor)) callees.add(callee.getEntryPoint().toString());
            row.put("callees",callees);
        }
        decompiler.dispose();
        output.put("candidates",candidates);
        Files.writeString(Path.of(args[0]),new GsonBuilder().setPrettyPrinting().create().toJson(output)+"\n");
        println("Survey functions="+total+" candidates="+candidates.size()+" output="+args[0]);
    }
}
