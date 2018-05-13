#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    This file is used to find the best parameters
    for test/felix.cnf
    Answer the questions and you will obtain the results
    of the search of the best parameters
    as a dictionaries and dataframes

    __author__ = "Mario Parreño Lara"
    __license__ = "MIT"
    __email__ = "maparla@inf.upv.es"
    __status__ = "Production"
"""

import os
import math
import pickle
import fileinput
import pandas as pd
import csv
import operator
import felix_core as fc
import felix_constants as constant

# With the shape {percentage: [printed, '#'Num]}
# -printed: painted or not for the percentage (percentage)
# -'#'Num: tells us how many '#' to put
progress_bar = {}
for indx, progress in enumerate(range(0, 102, 2)):
    progress_bar[progress] = [False, indx]


def change_config(beam, gsf, wip):
    """ Change the analyzer configuration

    Takes the analyzer configuration file (test/felix.cnf) and modify
    ('beam', 'grammar-scale-factor', 'word-insertion-penalty') from [decoder]

    Args:
        beam: The new value for 'beam'
        gsf: The new value for 'grammar-scale-factor'
        wip: The new value for 'word-insertion-penalty'
    """
    for line in fileinput.FileInput("test/felix.cnf", inplace=1):
        nline = line.strip().split("=")
        if nline[0].startswith("beam"):
            nline[1] = str(beam)
        elif nline[0].startswith("grammar-scale-factor"):
            nline[1] = str(gsf)
        elif nline[0].startswith("word-insertion-penalty"):
            nline[1] = str(wip)
        line = ' = '.join(nline)
        line = ' '.join(line.split())
        print line


def print_progress(num_iterations, current_iteration):
    """ Print a status bar

    Takes the analyzer configuration file (test/felix.cnf) and modify
    ('beam', 'grammar-scale-factor', 'word-insertion-penalty') from [decoder]

    Args:
        num_iterations: Total number of iterations
        current_iteration: The number of current iteration
    """
    global progress_bar
    base = 2  # Defines the round steps
    real_progress = (current_iteration * 100) / num_iterations
    rounded_progress = int(base * math.floor(float(real_progress) / base))

    if not progress_bar[rounded_progress][0]:
        progress_bar[rounded_progress][0] = True

        completed = progress_bar[rounded_progress][1]
        incompleted = progress_bar[100][1] - completed

        status_bar = "|" + \
            ''.join(completed * ['#']) + ''.join(incompleted * ["-"]) + "|"
        print status_bar


def analyze_folder(folder_path, with_verbosity):
    """ Takes a folder and analyzes it

    Takes a folder path and question analyzer configuration,
    then predicts all the files and returns the wrong and
    correct number of predictions.

    Args:
        folder_path: The relative path to the folder wich we want analyze
        with_verbosity: For print more or less information
    """
    hits = 0
    semantic_hits = 0
    errors = 0
    semantic_errors = 0
    files_analyzed = 0
    total_files = len(os.listdir(folder_path))
    for a_file in os.listdir(folder_path):
        files_analyzed += 1

        file_path = folder_path + a_file
        correct_command = (file_path[file_path.index('-') + 1:
                                     file_path.index('.raw')].replace('_', ' '))
        prediction = fc.predict_file(file_path)

        if with_verbosity:
            print "\n-----------------------------------------------"
            print "The path to the file is: " + file_path
            print "The correct command is: " + correct_command
            print "The prediction is: " + prediction
            print "-----------------------------------------------"

        semantic_ok_cmd = correct_command.replace("minutos", "minuto")
        semantic_ok_cmd = semantic_ok_cmd.replace("segundos", "segundo")
        semantic_pred = prediction.replace("minutos", "minuto")
        semantic_pred = semantic_pred.replace("segundos", "segundo")

        if correct_command == prediction:
            hits += 1
        else:
            errors += 1

        if semantic_ok_cmd == semantic_pred:
            semantic_hits += 1
        else:
            semantic_errors += 1

        if not with_verbosity:
            print_progress(total_files, files_analyzed)

    print "\n\n-------> ANALYSIS FINISHED <-------"
    print "Total files analyzed: " + str(files_analyzed)
    print "-- Grammar percentage --"
    print "\tNumber of hits: " + str(hits)
    print "\tNumber of errors: " + str(errors)
    print "\Percentage of grammatical accuracy: "\
          + str((hits * 100) / files_analyzed) + "%"
    print "-- Semantic percentage --"
    print "\tNumber of hits: " + str(semantic_hits)
    print "\tNumber of errors: " + str(semantic_errors)
    print "\Percentage of semantic accuracy: "\
          + str((semantic_hits * 100) / files_analyzed) + "%"
    print "----------------------------------\n"


if __name__ == "__main__":

    folder_path = raw_input("Which directory do you want to analyze? ")
    sweep = raw_input("Do you want to make a sweep of parameters? y/n: ")

    if sweep == "n":
        verbosity = raw_input("Verbosity? y/n: ")
        print "\nOkay, insert the parameters:"
        user_beam = raw_input("\t-Beam: ")
        user_gsf = raw_input("\t-Grammar Scale Factor: ")
        user_wip = raw_input("\t-Word Insertion Penalty: ")
        change_config(user_beam, user_gsf, user_wip)
        analyze_folder(folder_path, constant.YN_TO_BOOL[verbosity])

    else:
        print "\nOkay, insert the parameters:"

        bm_min = int(raw_input("\t-Minimum Beam: "))
        bm_max = int(raw_input("\t-Maximum Beam: "))
        bm_step = int(raw_input("\t-Beam Step: "))

        gsf_min = int(raw_input("\t-Minimum Grammar Scale Factor: "))
        gsf_max = int(raw_input("\t-Maximum Grammar Scale Factor: "))
        gsf_step = int(raw_input("\t-Grammar Scale Factor Step: "))

        wip_min = int(raw_input("\t-Minimum Word Insertion Penalty: "))
        wip_max = int(raw_input("\t-Maximum Word Insertion Penalty: "))
        wip_step = int(raw_input("\t-Word Insertion Penalty Step: "))

        analysis_res = {}

        total_bm_steps = (bm_max - bm_min) / bm_step + 1
        total_gsf_steps = (gsf_max - gsf_min) / gsf_step + 1
        total_wip_steps = (wip_max - wip_min) / wip_step + 1

        total_it = total_bm_steps * total_gsf_steps * total_wip_steps

        curr_it = 0

        print "\nLet's start -> " + str(total_it) + " iterations.\n"

        # At the maximum number I add the step because then I would not do the last step
        for bm in range(bm_min, bm_max + bm_step, bm_step):
            for gsf in range(gsf_min, gsf_max + gsf_step, gsf_step):
                for wip in range(wip_min, wip_max + wip_step, wip_step):
                    change_config(bm, gsf, wip)
                    analysis_res[(bm, gsf, wip)] = fc.predict_folder(
                        folder_path, True)
                    curr_it += 1
                    print_progress(total_it, curr_it)

        print "Completed analysis!"
        print "Saving the results..."

        # https://stackoverflow.com/questions/8685809/python-writing-a-dictionary-to-a-csv-file-with-one-line-for-every-key-value
        with open('dict.csv', 'wb') as csv_file:
            writer_csv = csv.writer(csv_file)
            for key, value in sorted(analysis_res.items(), key=operator.itemgetter(1), reverse=True):
                writer_csv.writerow([key, value])

        with open('sweep_dic.pickle', 'wb') as handle:
            pickle.dump(analysis_res, handle, protocol=pickle.HIGHEST_PROTOCOL)

        print "The highest hit rate is for the combination"
        print str(max(analysis_res, key=analysis_res.get)) + " with " + str(analysis_res[max(analysis_res, key=analysis_res.get)]) + "."

        """
        # http://stackoverflow.com/questions/11218477/how-can-i-use-pickle-to-save-a-dict
        with open('sweep_dic.pickle', 'wb') as handle:
            pickle.dump(analysis_res, handle, protocol=pickle.HIGHEST_PROTOCOL)

        with open('sweep_dic.txt', 'wb') as handle:
            pickle.dump(analysis_res, handle, protocol=pickle.HIGHEST_PROTOCOL)

        dataframe_res = pd.Series(
                            analysis_res).reset_index()
        dataframe_res.columns = [
            'Beam', 'Grammar Scale Factor',
            'Word Insertion Penalty', 'Hit rate % (Grammar/Semantic)'
        ]
        # http://stackoverflow.com/questions/17098654/how-to-store-data-frame-using-pandas-python
        dataframe_res.to_pickle("sweep_dataframe.pickle")
        dataframe_res.to_pickle("sweep_dataframe.txt")

        # http://stackoverflow.com/questions/613183/sort-a-python-dictionary-by-value
        # TODO: Sort dataframe
        """

        print("")
        print("--> COMPLETED! <--")
