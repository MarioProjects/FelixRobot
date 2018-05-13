#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    There we will define all the constants of our project

    __author__ = "Mario Parreño Lara"
    __license__ = "MIT"
    __email__ = "maparla@inf.upv.es"
    __status__ = "Production"
"""

CMD_RECORDER = [
                'sox', '-t', 'alsa', 'hw:1,0', '@param_file',
                'rate', '16k', 'silence', '-l',
                '1', '0.02', '2.7%',
                '1', '2.25', '3.5%',
                'noisered', 'noise.prof', '0.22',
                'vol', '8dB',
                'pad', '0.25'
               ]

AFIRMATIVE_ANSWERS = ["y", "Y", "yes"]

YN_TO_BOOL = {"y": True, "n": False}

# Takes a writted number and returns his int representation
STR_TO_INT = {
    "cero": 0,
    "uno": 1,
    "unn": 1,
    "dos": 2,
    "tres": 3,
    "cuatro": 4,
    "cinco": 5,
    "seis": 6,
    "siete": 7,
    "ocho": 8,
    "nueve": 9,
    "diez": 10,
}
