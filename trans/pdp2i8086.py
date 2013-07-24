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
            lexer.read()
            lexer.read()
            if lexer.text == "*":
                lexer.read()
                lexer.read()
            write("call " + lexer.text)
        elif tok == "jmp":
            write("jmp " + lexer.text)
            lexer.read()
        elif tok == "mov":
            src = lexer.text
            lexer.read()
            if src == "$":
                tok = lexer.text
                lexer.read()
                if tok.isdigit():
                    src = "#0x%x" % int(tok, 8)
                else:
                    src = "#" + tok
            dst = lexer.text
            lexer.read()
            if dst == "(":
                dst += regs[lexer.text]
                lexer.read()
                dst += lexer.text
                lexer.read()
                if dst == "(sp)":
                    write("mov bx, sp; ")
                    dst = "(bx)"
            write("mov " + dst + ", " + src)
    print
