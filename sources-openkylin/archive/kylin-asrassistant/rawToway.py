import wave

pcmfile = open("/home/hh/work/test.raw", 'rb')
pcmdata = pcmfile.read()
wavfile = wave.open("/home/hh/work/test.wav", 'wb')
wavfile.setframerate(16000)
wavfile.setsampwidth(2)    #16位采样即为2字节
wavfile.setnchannels(1)
wavfile.writeframes(pcmdata)
wavfile.close()
