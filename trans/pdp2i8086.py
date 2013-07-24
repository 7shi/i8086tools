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
    return str.isalnum(ch) or ch == '_' or ch == '~'

class Lexer:
    def __init__(self, s):
        self.p = 0
        self.s = s

    def canread(self):
        return self.p < len(self.s)

    def readwhile(self, f):
        ret = ""
        while self.canread() and f(self.s[self.p]):
            ret += self.s[self.p]
            self.p += 1
        return ret

    def read(self):
        if not self.canread(): return ""
        ch = self.s[self.p]
        self.p += 1
        if isspace(ch):
            return self.read()
        elif isletter(ch):
            return ch + self.readwhile(isletter)
        elif ch == '.':
            return ch + self.readwhile(str.isalnum)
        return ch

def gettokens(s):
    ret = []
    lexer = Lexer(s)
    while True:
        tok = lexer.read()
        if tok == "": break
        ret += [tok]
    return ret

def convr(r):
    return { "r0": "ax",
             "r1": "dx",
             "r2": "cx",
             "r3": "si",
             "r4": "di",
             "r5": "bp",
             "r6": "sp",
             "sp": "sp" }[r]

write = sys.stdout.write

for line in lines:
    toks = gettokens(line)
    i = 0
    while i < len(toks):
        tok = toks[i]
        i += 1
        if tok == ";":
            write("; ")
        if tok == ".globl":
            if i < len(toks):
                write(".extern " + toks[i])
                i += 1
        elif tok == ".text":
            write(".sect .text")
        elif tok == ".data":
            write(".sect .data")
        elif tok == ".byte":
            write(".data1 ")
            while i < len(toks):
                tok = toks[i]
                if str.isdigit(tok):
                    write("0x%02x" % int(tok, 8))
                elif tok == ",":
                    write(", ")
                else:
                    break
                i += 1
        elif i < len(toks) and toks[i] == ":":
            if tok[0] != "~":
                write(tok + ":")
        elif tok == "jsr":
            r = toks[i]
            i += 2
            if toks[i] == "*": i += 2
            write("call " + toks[i])
            i += 1
        elif tok == "jmp":
            write("jmp " + toks[i])
            i += 1
        elif tok == "mov":
            src = toks[i]
            i += 1
            if src == "$":
                src = "#" + toks[i]
                i += 1
            i += 1
            dst = toks[i]
            i += 1
            if dst == "(":
                dst += toks[i] + toks[i + 1]
                i += 2
                if dst == "(sp)":
                    write("mov bx, sp; ")
                    dst = "(bx)"
            write("mov " + dst + ", " + src)
    print
