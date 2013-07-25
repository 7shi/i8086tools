#!/usr/bin/env python
# This file is in the public domain.
import sys

target = "hello.s"
if len(sys.argv) == 2:
    target = sys.argv[1]
elif len(sys.argv) > 2:
    print "usage: pdp2i8086.py pdp11.s"
    sys.exit(1)

with open(target) as f:
    lines = f.readlines()

def isspace(ch):
    return ch <= ' '
def isletter(ch):
    return ch.isalnum() or ch == '_' or ch == '~'

class Lexer:
    def __init__(self, s):
        self.p = 0
        self.s = s
        self.read()

    def canread(self):
        return self.p < len(self.s)

    def readwhile(self, f):
        ret = ""
        while self.canread() and f(self.s[self.p]):
            ret += self.s[self.p]
            self.p += 1
        return ret

    def read(self):
        if not self.canread():
            self.text = ""
            return
        ch = self.s[self.p]
        self.p += 1
        if isspace(ch):
            self.readwhile(isspace)
            self.read()
        elif isletter(ch):
            self.text = ch + self.readwhile(isletter)
        elif ch == '.':
            self.text = ch + self.readwhile(str.isalnum)
        else:
            self.text = ch

regs = { "r0": "ax",
         "r1": "dx",
         "r2": "cx",
         "r3": "si",
         "r4": "di",
         "r5": "bp",
         "r6": "sp",
         "sp": "sp",
         "r7": "ip",
         "pc": "ip" }

write = sys.stdout.write

def getnum(o):
    if not o.isdigit():
        return o
    d = int(o, 8)
    if d < 10:
        return str(d)
    return "0x%x" % d

def readopr(lexer):
    if lexer.text == "":
        return "", ""
    if lexer.text == "$":
        lexer.read()
        if lexer.text == "":
            return "", ""
        tok = lexer.text
        lexer.read()
        return "#" + getnum(tok), ""
    ret, post = "", ""
    dec, addr = False, False
    if lexer.text == "-":
        lexer.read()
        if lexer.text.isdigit():
            ret += "-"
        else:
            dec = True
    if lexer.text.isdigit():
        ret += getnum(lexer.text)
        lexer.read()
    if lexer.text == "(":
        addr = True
        ret += lexer.text
        lexer.read()
    r = lexer.text
    if not regs.has_key(r):
        return ret
    rr = regs[r]
    if dec:
        write("sub " + rr + ", #2; ")
    if addr and r == "sp":
        write("mov bx, sp; ")
        ret += "bx"
    else:
        ret += rr
    lexer.read()
    if lexer.text == ")":
        ret += lexer.text
        lexer.read()
        if lexer.text == "+":
            post = "add " + rr + ", #2"
            lexer.read()
    return ret, post

for line in lines:
    lexer = Lexer(line)
    while lexer.text != "":
        tok = lexer.text
        lexer.read()
        if tok == ";":
            write("; ")
        elif tok == ".globl":
            if lexer.text != "":
                write(".extern " + lexer.text)
                lexer.read()
        elif tok == ".text":
            write(".sect .text")
        elif tok == ".data":
            write(".sect .data")
        elif tok == ".byte":
            write(".data1 ")
            while lexer.text != "":
                tok = lexer.text
                if str.isdigit(tok):
                    write("0x%02x" % int(tok, 8))
                    lexer.read()
                elif tok == ",":
                    write(", ")
                    lexer.read()
                else:
                    break
        elif lexer.text == ":":
            if tok[0] != "~":
                write(tok + ":")
        elif tok == "jsr":
            if not regs.has_key(lexer.text):
                write("! jsr")
                continue
            lexer.read()
            if lexer.text != ",":
                write("! jsr")
                continue
            lexer.read()
            if lexer.text == "*":
                lexer.read()
                if lexer.text != "$":
                    write("! jsr")
                    continue
                lexer.read()
            write("call " + lexer.text)
        elif tok == "jmp":
            write("jmp " + lexer.text)
            lexer.read()
        elif tok == "mov":
            src, p1 = readopr(lexer)
            if lexer.text != ",":
                write("! mov")
                continue
            lexer.read()
            dst, p2 = readopr(lexer)
            write("mov " + dst + ", " + src)
            if p1 != "": write("; " + p1)
            if p2 != "": write("; " + p2)
        elif tok == "tst":
            src, p1 = readopr(lexer)
            write("cmp " + src + ", #0")
            if p1 != "": write("; " + p1)
    print
