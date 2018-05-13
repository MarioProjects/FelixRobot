#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Information:
#->Felix brain: Constantly records a phrase, predict them and send the correct command to Felix

import time
from socketIO_client import SocketIO, BaseNamespace
import felix_core as fc
import felix_constants as constant


class Namespace(BaseNamespace):

    def on_connect(self):
        print('[Connected to Félix]')

    def on_reconnect(self):
        print('[Reconnected to Félix]')

    def on_disconnect(self):
        print('[Disconnected to Félix]')
        os.execl(sys.executable, sys.executable, *sys.argv)


if __name__ == "__main__":

    prev_status = "sientate"
    felix_ip = raw_input("Where IP is Felix? ")
    felix_port = raw_input("In which port? ")

    socketIO = SocketIO(felix_ip, felix_port, Namespace)

    while True:

        print "\nListening..."
        voice_command = fc.take_and_predict()
        orders_performed = 0

        for command in voice_command.split("y"):
            orders_performed += 1
            command_parts = command.strip().split(" ")
            the_command = command_parts[0]

            how_much = "infinite"
            time_unity = "no_unity"

            if(len(command_parts) > 1):
                how_much = constant.STR_TO_INT[command_parts[1]]
                time_unity = command_parts[2]

            if how_much != "infinite" and time_unity == "minuto" or time_unity == "minutos":
                how_much = int(how_much) * 60

            # Send order via Socket
            print "\nSend the order: " + the_command + " (" + str(how_much) + " seconds" ").\n"
            socketIO.emit('voiceCommand', {'text': the_command})

            if how_much != "infinite":
                time.sleep(how_much)
            # If there are no more actions to do -> State = Stop
            if(orders_performed == len(voice_command.split("y"))):
                socketIO.emit('voiceCommand', {'text': "para"})
