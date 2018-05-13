#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    This file is intended to record for as long as we wish,
    phrases such that these will be cut from a threshold already defined

    __author__ = "Mario Parreño Lara"
    __license__ = "MIT"
    __email__ = "maparla@inf.upv.es"
    __status__ = "Production"
"""

import subprocess
import time
import datetime
import felix_core as fc
import felix_constants as constant

continue_recording = True


def continue_with_the_recording():
    global continue_recording

    user_continue_recording = raw_input("Do you want to continue"
        	                            " recording? y/n: ")
    if user_continue_recording not in constant.AFIRMATIVE_ANSWERS:
        continue_recording = False
    else:
        countdown = 3  # Seconds countdown
        while countdown:
            print countdown
            time.sleep(1)
            countdown -= 1

if __name__ == "__main__":

    with_prediction = raw_input("Do you want the prediction? y/n: ")

    while continue_recording:

        if with_prediction in constant.AFIRMATIVE_ANSWERS:
            current_prediction = fc.take_and_predict()
            print "\n-------------------------------------------------------"
            print "The prediction: " + current_prediction
            print "-------------------------------------------------------\n"
            continue_with_the_recording()

        else:
            #this_moment = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
            #output_filename = 'test/samples/rec' + this_moment + '.raw'
            output_filename = raw_input("\n-File name (Name-my_order.raw): ")
            output_filename = 'test/samples/' + output_filename

            cmd_recorder = list(constant.CMD_RECORDER)
            cmd_recorder[cmd_recorder.index("@param_file")] = output_filename

            subprocess.call(cmd_recorder)

            print "\n-------------------------------------------------------"
            print "Recorded correctly: " + output_filename
            print "-------------------------------------------------------\n"

            #continue_with_the_recording() To speedup the process

    print "\nSee you soon.\n"
