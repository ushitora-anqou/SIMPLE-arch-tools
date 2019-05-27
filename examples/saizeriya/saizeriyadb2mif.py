import sqlite3

con = sqlite3.connect("saizeriya.db")
cur = con.cursor()

# menu
# 0: id
# 1: name
# 2: category
# 3: type
# 4: price
# 5: calorie
# 6: salt

print("""
WIDTH=16;
DEPTH=2048;

ADDRESS_RADIX=HEX;
DATA_RADIX=DEC;

CONTENT BEGIN
	[000..400]  :   0;
""")

for row in cur.execute("select * from menu;"):
    print("\t{:04X}\t:\t{};".format(row[0] + 0x400, row[5]))

print("""
        [473..4FF]  :   0;
        500         :   32767;
""")

for row in cur.execute("select * from menu;"):
    print("\t{:04X}\t:\t{};".format(row[0] + 0x500, row[4]))

print("""
END;
""")
