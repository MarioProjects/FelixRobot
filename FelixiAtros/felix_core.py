#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    File with auxiliary functions for taking
    and predicting sentences

    __author__ = "Mario Parreño Lara"
    __license__ = "MIT"
    __email__ = "maparla@inf.upv.es"
    __status__ = "Production"
"""

import subprocess
import os
import sys
import datetime
import felix_constants as constant

# Mirar la referencia:
# http://stackoverflow.com/questions/24470561/how-to-check-if-a-shell-command-is-over-in-python
analyzer_proc = ""


def extract_prediction(cmd_output):
    """ To anaylze predictions

    Takes the output of the subprocess that analyzes the files
    as string and check if contains the prediction. If it contains
    then  return it

    Args:
        cmd_output: The output of the analyzer subprocess

    Returns:
        The output of the process that analyzes the files
    """
    if "<s>" in cmd_output:
        return cmd_output[cmd_output.index('<s>') + 3:
                          cmd_output.index('</s>')].strip()
    return ""


def init_predictor_proc():
    """ Initializes predictor

    Check if the subprocess that predicts the
    cepstral files exists, if not exists, it initializes

    """
    global analyzer_proc
    cmd_analyzer = [
        "LD_LIBRARY_PATH=iatros-v1.0/build/lib/ stdbuf -oL \
        iatros-v1.0/build/bin/iatros-offline -c test/felix.cnf"]
    if not analyzer_proc:
        analyzer_proc = subprocess.Popen(cmd_analyzer,
                                         stdout=subprocess.PIPE,
                                         stderr=subprocess.STDOUT,
                                         bufsize=1,
                                         shell=True)


def take_and_predict():
    """ Takes and predict a sound

    This function first records the voice and then returns his prediction

    Returns:
        Prediction for the recorded sound
    """
    moment = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    output_filename = 'test/samples/rec%s.raw' % moment

    cmd_recorder = list(constant.CMD_RECORDER)
    cmd_recorder[cmd_recorder.index("@param_file")] = output_filename

    subprocess.call(cmd_recorder)

    prediction = predict_file(output_filename)
    os.remove(output_filename)

    return prediction


def predict_file(the_file):
    """ Predict a File

    This function takes a file and returns his iATROS prediction

    Args:
        the_file: File to analyze

    Returns:
        Prediction command string
    """
    if not analyzer_proc:
        init_predictor_proc()

    cmdCepstral = [
        'LD_LIBRARY_PATH=iatros-v1.0/build/lib/ \
        iatros-v1.0/build/bin/iatros-speech-cepstral \
        -c test/conf.feat -i ' + the_file + ' -o /tmp/felix.CC'
    ]
    subprocess.call(cmdCepstral, shell=True)

    prediction = ""
    for analyzer_output in iter(analyzer_proc.stdout.readline, b''):
        prediction += extract_prediction(analyzer_output)
        if prediction.strip():
            break
    return prediction


def predict_file_new_analyzer(the_file):
    """ Predict a File

    This function takes a file and returns his iATROS prediction
    killing the analyzer process and creating it again

    Args:
        the_file: File to analyze

    Returns:
        Prediction command string
    """
    if analyzer_proc:
        analyzer_proc.kill()
    init_predictor_proc()

    cmdCepstral = [
        'LD_LIBRARY_PATH=iatros-v1.0/build/lib/ \
        iatros-v1.0/build/bin/iatros-speech-cepstral \
        -c test/conf.feat -i ' + the_file + ' -o /tmp/felix.CC'
    ]
    subprocess.call(cmdCepstral, shell=True)

    prediction = ""
    for analyzer_output in iter(analyzer_proc.stdout.readline, b''):
        prediction += extract_prediction(analyzer_output)
        if prediction.strip():
            break
    return prediction


def predict_folder(folder_path, restore_analyzer):
    """ Predict a Folder

    This function takes a folder and returns his hit rate
    for the prediction of all of his files

    Args:
        folder_path: Folder to analyze

    Returns:
        Hit semantic rate from all files prediction
    """
    hits = 0
    semantic_hits = 0
    errors = 0
    semantic_errors = 0
    num_files = 0

    for a_file in os.listdir(folder_path):
        num_files += 1

        file_path = folder_path + a_file
        correct_command = file_path[file_path.index(
            '-') + 1:file_path.index('.raw')].replace('_', ' ')
        if not restore_analyzer:
            prediction = predict_file(file_path)
        else:
            prediction = predict_file_new_analyzer(file_path)

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

    return (semantic_hits * 100) / num_files
