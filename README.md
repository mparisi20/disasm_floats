# disasm_floats 
###### by Max Parisi

This program disassembles a table of floats (32-bit, single precision floating point data) read from a DOL (GameCube/Wii executable file format)
and writes the results as defined data in a format accepted by the GNU Assembler. The intent of this program is to automate the process of
disassembling a table of numerical data in a DOL file regardless of its size. 

## Usage

`./disasm_floats <dol_file>, <abs_address>, <size>, <table_name>, <floats_per_line>`

The user provides the path to the DOL executable, the memory address and size of the table to
be disassembled, the desired table name, and the number of floats to emit per line. All numerical
input must be provided in hexadecimal format

## Future Work
In the future, I may generalize this program so that it can dump arrays of data types besides floats.
