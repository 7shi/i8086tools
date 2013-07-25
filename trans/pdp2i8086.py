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

def getnum(o):
    if not o.isdigit():
        return o
    d = int(o, 8)
    if d < 10:
        return str(d)
    return "0x%x" % d

def readopr(lexer):
    if lexer.text == "":
        return "", -1
    if lexer.text == "$":
        lexer.read()
        if lexer.text == "":
            return "", -1
        tok = lexer.text
        lexer.read()
        return "#" + getnum(tok), -1
    ret = ""
    mode = 0
    if lexer.text == "-":
        lexer.read()
        if lexer.text.isdigit():
            ret += "-"
        else:
            mode = 4 # -(R)
    if lexer.text.isdigit():
        ret += getnum(lexer.text)
        lexer.read()
    if lexer.text == "(":
        if mode == 0: mode = 1
        ret += lexer.text
        lexer.read()
    r = lexer.text
    if not regs.has_key(r):
        return ret, mode
    ret += regs[r]
    lexer.read()
    if lexer.text == ")":
        ret += lexer.text
        lexer.read()
        if lexer.text == "+":
            mode = 2 # (R)+
            lexer.read()
    return ret, mode

written = False

def write(a):
    global written
    sys.stdout.write(a)
    written = True

def convsrc(src):
    p = src.find("(")
    r = src[p+1:p+3]
    if r != "bp" and r != "si" and r != "di":
        write("mov bx, " + r + "; ")
        src = src[:p+1] + "bx" + src[p+3:]
    write("mov bx, " + src + "; ")
    return "bx"

for line in lines:
    written = False
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
                written = lexer.text != ""
            lexer.read()
        elif tok == "jsr":
            if not regs.has_key(lexer.text):
                continue
            lexer.read()
            if lexer.text != ",":
                continue
            lexer.read()
            if lexer.text == "*":
                lexer.read()
                if lexer.text != "$":
                    continue
                lexer.read()
            write("call " + lexer.text)
        elif tok == "jmp":
            write("jmp " + lexer.text)
            lexer.read()
        elif tok == "mov":
            src, m1 = readopr(lexer)
            if lexer.text != ",":
                continue
            lexer.read()
            dst, m2 = readopr(lexer)
            if dst == "(sp)" and (m2 == 1 or m2 == 4):
                if m2 == 1:
                    write("add sp, #2; ")
                if src[0] == "#":
                    write("mov bx, " + src + "; ")
                    src = "bx"
                elif m1 != 0:
                    src = convsrc(src)
                write("push " + src)
            else:
                write("mov " + dst + ", " + src)
            assert m1 != 2 and m2 != 2, line
        elif tok == "tst":
            src, m1 = readopr(lexer)
            if src == "(sp)" and m1 == 2:
                write("add sp, #2")
            elif src == "(sp)" and m1 == 4:
                write("sub sp, #2")
            else:
                write("cmp " + src + ", #0")
        elif tok == "cmp":
            src, m1 = readopr(lexer)
            if lexer.text != ",":
                continue
            lexer.read()
            dst, m2 = readopr(lexer)
            if src == "(sp)" and dst == "(sp)" and m1 == 2 and m2 == 2:
                write("add sp, #4")
                continue
            assert m1 != 2 and m2 != 2, line
            assert m1 != 4 and m2 != 4, line
            if src[0] == "#":
                write("mov bx, " + src + "; ")
                src = "bx"
            elif m1 != 0:
                src = convsrc(src)
            write("cmp " + src + ", " + dst)
        elif tok == "add":
            src, m1 = readopr(lexer)
            if lexer.text != ",":
                continue
            lexer.read()
            dst, m2 = readopr(lexer)
            assert m1 != 2 and m2 != 2, line
            assert m1 != 4 and m2 != 4, line
            write("add " + dst + ", " + src)
        elif tok == "clr":
            dst, m1 = readopr(lexer)
            assert m1 != 2 and m1 != 4, line
            write("mov " + dst + ", #0")
        elif tok == "inc":
            dst, m1 = readopr(lexer)
            assert m1 != 2 and m1 != 4, line
            write("inc " + dst)
        elif tok == "asl":
            src, m1 = readopr(lexer)
            assert m1 == 0, line
            write("sal " + src + ", #1")
        elif tok == "jbr":
            write("jmp " + lexer.text)
            lexer.read()
        elif tok == "jle":
            write("jle " + lexer.text)
            lexer.read()
    if written:
        print
    else:
        write("! " + line)
