import datetime
import wave
import sys
import socket

read_until = datetime.datetime.now() + datetime.timedelta(seconds = 30)

wav = wave.open('sound.wav', 'wb')
wav.setnchannels(1)
wav.setframerate(8000)
wav.setsampwidth(4)

is_first = True

try:
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind(('', 3333))

	while datetime.datetime.now() < read_until:
		if is_first:
			read_until = datetime.datetime.now() \
				+ datetime.timedelta(seconds = 30)
			is_first = False

		msg, _ = sock.recvfrom(1024)
		wav.writeframes(msg)
		print("writing data")
finally:
	wav.close()
