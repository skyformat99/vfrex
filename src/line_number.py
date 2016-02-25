#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: ts=4 sw=4 expandtab:

import os
import subprocess
import sys

def read_txt_file(filename):
    with open(filename,'r') as f:
        txt = f.read().strip()
    # Python should be able to do it automatically, but just in case...
    txt = txt.replace('\r','')
    return txt

def main():
    names = sys.argv[1:]
    if not names:
        names = sorted(os.listdir('.'))
    total = 0;
    for name in names:
        if not os.path.splitext(name)[1] in set(['.h', '.c', '.py', '.bat']):
            continue
        with open(name, "r") as fin:
            num = 0
            for line in fin:
                s = line.strip()
                if s != "" and s[:2] != "/*" and s[:2] != "* ":
                    num += 1
        print '{0}: {1}'.format(name, num)
        total += num;

    print "\nTotal: {0}".format(total)
    if os.name == 'nt':
        print('Press Enter to continue...')
        try:
            raw_input() # Python 2
        except:
            input() # Python 3

if __name__ == '__main__':
    main()
