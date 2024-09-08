import datetime
import wave
import sys
import socket

read_until = datetime.datetime.now() + datetime.timedelta(seconds = 30)

wav = wave.open('sound.wav', 'wb')
wav.setnchannels(1)
wav.setframerate(8000)
wav.setsampwidth(4)

try:
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind(('', 3333))

	while datetime.datetime.now() < read_until:
		msg, _ = sock.recvfrom(1024)
		print(msg)
		wav.writeframes(msg)
finally:
	wav.close()
