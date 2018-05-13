import os
import operator
import csv
import math
import pickle
import fileinput
from random import randint


def iterator():

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

    real_it = 0

    print "\nLet's start -> " + str(total_it) + " iterations.\n"

    # Al máximo le sumo el step ya que el último paso no lo hace
    for bm in range(bm_min, bm_max + bm_step, bm_step):
        for gsf in range(gsf_min, gsf_max + gsf_step, gsf_step):
            for wip in range(wip_min, wip_max + wip_step, wip_step):
                analysis_res[(bm, gsf, wip)] = randint(0, 100)
                real_it += 1

    print "Completed analysis!"
    print "Real total iterations -> " + str(real_it) + "\n"
    print "Saving the results..."

    # https://stackoverflow.com/questions/8685809/python-writing-a-dictionary-to-a-csv-file-with-one-line-for-every-key-value
    with open('dict.csv', 'wb') as csv_file:
        writer = csv.writer(csv_file)
        for key, value in sorted(analysis_res.items(), key=operator.itemgetter(1), reverse=True):
            writer.writerow([key, value])

    with open('sweep_dic.pickle', 'wb') as handle:
        pickle.dump(analysis_res, handle, protocol=pickle.HIGHEST_PROTOCOL)

    print "The highest hit rate is for the combination"
    print str(max(analysis_res, key=analysis_res.get)) + " with " + str(analysis_res[max(analysis_res, key=analysis_res.get)]) + "."
    #dataframe_res = pd.Series(analysis_res).reset_index()

    # dataframe_res.columns = [
    #    'Beam', 'Grammar Scale Factor',
    #    'Word Insertion Penalty', 'Hit rate % (Grammar/Semantic)'
    #]

    # http://stackoverflow.com/questions/17098654/how-to-store-data-frame-using-pandas-python
    # dataframe_res.to_pickle("sweep_dataframe.pickle")
    # dataframe_res.to_pickle("sweep_dataframe.txt")

    print "\n----- SAVED -----\n\n"


if __name__ == "__main__":

    options = {1: iterator}

    while True:

        valid_option = False
        while not valid_option:
            wich_test = int(raw_input(
                "What want you test?\n \t[1] Iterator - Test iterations from trainer and how save the results.\n\n"))
            if wich_test in options:
                valid_option = True
                options[wich_test]()
            else:
                print "\nYou choosed an invalid option.\n"

        want_continue = raw_input("\t-Do you want to do more tests?: (y/n) ")
        if want_continue != 'y' or want_continue != 'Y':
            break
