import datetime
import wave
import sys
import socket

read_until = datetime.datetime.now() + datetime.timedelta(seconds = 30)

wav = wave.open('sound.wav', 'wb')
wav.setnchannels(1)
wav.setframerate(16000)
wav.setsampwidth(3)

n = 0
is_first = True

try:
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind(('', 3333))

	while datetime.datetime.now() < read_until:
		if is_first:
			read_until = datetime.datetime.now() \
				+ datetime.timedelta(seconds = 30)
			is_first = False

		msg, _ = sock.recvfrom(2100)
		wav.writeframes(msg)

		n += 1
		print(f"writing data: {n}")
finally:
	wav.close()
