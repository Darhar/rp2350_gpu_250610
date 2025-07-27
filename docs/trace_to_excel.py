import re
import sys
import pandas as pd
from collections import defaultdict

def parse_trace_log(trace_log: str):
    """
    Parse the trace log into a dictionary of tasks.
    Each task contains a list of (class, message) tuples in the order they appear.
    """
    lines = [line.strip() for line in trace_log.splitlines() if line.strip()]
    pattern_bracket = re.compile(r'^\[(.*?)\]\s*(.*?):\s*(.*)$')
    pattern_colon = re.compile(r'^([^:\[]+):\s*(.*)$')
    pattern_task = re.compile(r'^===\s*Task:\s*(.*?)\s*===', re.IGNORECASE)

    tasks = defaultdict(list)
    current_task = "Default"

    for line in lines:
        # Detect a task header
        m = pattern_task.match(line)
        if m:
            current_task = m.group(1)
            continue

        # Detect [Class] function: message
        m = pattern_bracket.match(line)
        if m:
            cls, func, msg = m.groups()
            tasks[current_task].append((cls, f"{func}: {msg}"))
            continue

        # Detect class: message
        m = pattern_colon.match(line)
        if m:
            cls, msg = m.groups()
            cls = cls.strip()
            tasks[current_task].append((cls, msg))
            continue

    return tasks

def tasks_to_excel(tasks, output_file):
    """
    Write tasks into an Excel file, each task in its own sheet.
    Each sheet contains only the columns used in that task, ordered by first use.
    """
    with pd.ExcelWriter(output_file, engine='xlsxwriter') as writer:
        for task_name, entries in tasks.items():
            # Determine column order for this task
            column_order = []
            for cls, _ in entries:
                if cls not in column_order:
                    column_order.append(cls)

            # Create rows
            rows = []
            for cls, msg in entries:
                row = {col: '' for col in column_order}
                row[cls] = msg
                rows.append(row)

            # Create DataFrame and save to sheet
            df = pd.DataFrame(rows, columns=column_order)
            df.to_excel(writer, sheet_name=task_name[:31], index=False)  # Excel limits sheet names to 31 chars

def main():
    if len(sys.argv) != 3:
        print("Usage: python trace_to_excel.py input_trace.txt output_trace.xlsx")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file, 'r', encoding='utf-8') as f:
        trace_log = f.read()

    tasks = parse_trace_log(trace_log)
    tasks_to_excel(tasks, output_file)

    print(f"Trace log parsed and saved to {output_file}")

if __name__ == '__main__':
    main()
