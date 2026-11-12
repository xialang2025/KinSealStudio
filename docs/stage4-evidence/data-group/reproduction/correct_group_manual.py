"""Reconcile old C26 type, count and line-number claims against the retained native widget properties."""
from pathlib import Path
root=Path(__file__).resolve().parent;work=root/'worktree';p=work/'design_plan/数据群组功能与交互复刻手册.md';s=p.read_text(encoding='utf-8')
s=s.replace('**16位整数、16位正整数、16位BCD码、32位整数、32位正整数、32位BCD码、32位浮点数**','**16位整数、32位整数、16位正整数、32位正整数、16位BCD码、32位BCD码、32位浮点数**（2026-11-13原厂Qt属性补证订正旧轮顺序）')
s=s.replace('帮助：设置 X 轴坐标最大值，最高只能设置 800 [H]；上限未实测 [待验]','帮助：设置 X 轴坐标最大值，最高800 [H]；2026-11-13原厂微调框范围实测 **10..800**，常量和地址模式下均保持可编辑')
s=s.replace('V5.1 中该框可编辑且默认 1，实际可设范围 [待验]','2026-11-13原厂Qt属性实测范围 **1..1**，固定单通道，不能增加')
s=s.replace('V5.1 实际提供下拉（','V5.1 实际提供下拉，当前机型固定只有1（')
s=s.replace('默认 **1（实线）**','默认 **0（实线）**（2026-11-13订正）').replace('五项：1/2/3/4/5','五项：0/1/2/3/4（2026-11-13原厂Qt属性订正旧编号）')
s=s.replace('另一选项内容 [待验]（内部名 `m_LimitSourcecbBox` [B]）','2026-11-13补证另一项为 **地址**：最小值仍是常量，只有最大值切换为地址 `Y_Max_lineEdit`（默认LW0），不能编造最小值地址')
s=s.replace('其余（两个刻度复选框组、四个色块、次分隔数、范围类型、最小/最大值等）在窗口枚举中不带名，需按矩形定位复刻。','旧轮Win32窗口枚举未读出部分内部名；2026-11-13通过原厂GUI线程Qt属性补齐，详见本轮字段证据，优先按对象名复现。')
p.write_text(s,encoding='utf-8',newline='\n')
print('Corrected old C26 handbook field order, exact bounds and source-mode claims from native evidence.')
